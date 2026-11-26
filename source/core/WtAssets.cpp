#include "WtAssets.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>

#include "../dsp/Fft.h"

namespace sbd {

// ---------------------------------------------------------------- SHA-256 (FIPS 180-4)
namespace {
constexpr std::uint32_t K256[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
    0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
    0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
inline std::uint32_t rotr(std::uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
}  // namespace

std::string sha256Hex(const void* data, std::size_t size) {
  std::uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  std::vector<std::uint8_t> m(static_cast<const std::uint8_t*>(data), static_cast<const std::uint8_t*>(data) + size);
  const std::uint64_t bits = static_cast<std::uint64_t>(size) * 8;
  m.push_back(0x80);
  while (m.size() % 64 != 56) m.push_back(0);
  for (int i = 7; i >= 0; --i) m.push_back(static_cast<std::uint8_t>(bits >> (8 * i)));
  for (std::size_t off = 0; off < m.size(); off += 64) {
    std::uint32_t w[64];
    for (int i = 0; i < 16; ++i)
      w[i] = (std::uint32_t(m[off + 4 * i]) << 24) | (std::uint32_t(m[off + 4 * i + 1]) << 16) | (std::uint32_t(m[off + 4 * i + 2]) << 8) | m[off + 4 * i + 3];
    for (int i = 16; i < 64; ++i) {
      const std::uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
      const std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
      w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
    for (int i = 0; i < 64; ++i) {
      const std::uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25), ch = (e & f) ^ (~e & g);
      const std::uint32_t t1 = hh + S1 + ch + K256[i] + w[i];
      const std::uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22), mj = (a & b) ^ (a & c) ^ (b & c);
      const std::uint32_t t2 = S0 + mj;
      hh = g;
      g = f;
      f = e;
      e = d + t1;
      d = c;
      c = b;
      b = a;
      a = t1 + t2;
    }
    h[0] += a, h[1] += b, h[2] += c, h[3] += d, h[4] += e, h[5] += f, h[6] += g, h[7] += hh;
  }
  static const char* hex = "0123456789abcdef";
  std::string o;
  for (std::uint32_t v : h)
    for (int i = 7; i >= 0; --i) o += hex[(v >> (4 * i)) & 15];
  return o;
}

// ---------------------------------------------------------------- WAV
namespace {
std::uint32_t le32(const std::uint8_t* p) { return std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) | (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24); }
std::uint16_t le16(const std::uint8_t* p) { return static_cast<std::uint16_t>(p[0] | (p[1] << 8)); }
}  // namespace

bool readWavMonoFromMemory(const std::vector<std::uint8_t>& b, std::vector<float>& out, double& sr, std::string& error) {
  if (b.size() < 44 || std::memcmp(b.data(), "RIFF", 4) != 0 || std::memcmp(b.data() + 8, "WAVE", 4) != 0) {
    error = "fichier WAV invalide (RIFF/WAVE)";
    return false;
  }
  int fmt = 0, ch = 0, bitsPer = 0;
  const std::uint8_t* data = nullptr;
  std::size_t dataLen = 0;
  std::size_t p = 12;
  while (p + 8 <= b.size()) {
    const std::uint32_t len = le32(&b[p + 4]);
    if (p + 8 + len > b.size()) break;
    if (std::memcmp(&b[p], "fmt ", 4) == 0 && len >= 16) {
      fmt = le16(&b[p + 8]);
      ch = le16(&b[p + 10]);
      sr = le32(&b[p + 12]);
      bitsPer = le16(&b[p + 22]);
      if (fmt == 0xFFFE && len >= 40) fmt = le16(&b[p + 32]);  // WAVE_FORMAT_EXTENSIBLE : sous-format
    } else if (std::memcmp(&b[p], "data", 4) == 0) {
      data = &b[p + 8];
      dataLen = len;
    }
    p += 8 + len + (len & 1);
  }
  if (!data || ch < 1 || ch > 8 || !((fmt == 1 && (bitsPer == 16 || bitsPer == 24 || bitsPer == 32)) || (fmt == 3 && bitsPer == 32))) {
    error = "format WAV non supporte (PCM 16/24/32 ou float32, 1..8 canaux)";
    return false;
  }
  const int bps = bitsPer / 8;
  const std::size_t frames = dataLen / static_cast<std::size_t>(bps * ch);
  out.assign(frames, 0.0f);
  for (std::size_t f = 0; f < frames; ++f) {
    double acc = 0.0;
    for (int c = 0; c < ch; ++c) {
      const std::uint8_t* s = data + (f * static_cast<std::size_t>(ch) + static_cast<std::size_t>(c)) * static_cast<std::size_t>(bps);
      double v;
      if (fmt == 3) {
        float x;
        std::memcpy(&x, s, 4);
        v = x;
      } else if (bps == 2) {
        v = static_cast<std::int16_t>(le16(s)) / 32768.0;
      } else if (bps == 3) {
        std::int32_t x = (s[0] << 8) | (s[1] << 16) | (s[2] << 24);
        v = (x >> 8) / 8388608.0;
      } else {
        v = static_cast<std::int32_t>(le32(s)) / 2147483648.0;
      }
      acc += v;
    }
    const double m = acc / ch;
    if (!std::isfinite(m)) {
      error = "echantillon non fini";
      return false;
    }
    out[f] = static_cast<float>(m);
  }
  return true;
}

bool readWavMono(const std::string& path, std::vector<float>& out, double& sr, std::string& error) {
  std::ifstream in(path, std::ios::binary | std::ios::ate);
  if (!in) {
    error = "fichier introuvable";
    return false;
  }
  const auto size = static_cast<std::size_t>(in.tellg());
  if (size > (64u << 20)) {
    error = "fichier trop volumineux (> 64 Mio)";
    return false;
  }
  std::vector<std::uint8_t> b(size);
  in.seekg(0);
  in.read(reinterpret_cast<char*>(b.data()), static_cast<std::streamsize>(size));
  return readWavMonoFromMemory(b, out, sr, error);
}

bool writeWavFloat(const std::string& path, const std::vector<float>& s, double sr, std::string& error) {
  std::ofstream o(path, std::ios::binary | std::ios::trunc);
  if (!o) {
    error = "ecriture impossible";
    return false;
  }
  auto w32 = [&o](std::uint32_t v) { o.put(char(v & 255)).put(char((v >> 8) & 255)).put(char((v >> 16) & 255)).put(char((v >> 24) & 255)); };
  auto w16 = [&o](std::uint16_t v) { o.put(char(v & 255)).put(char((v >> 8) & 255)); };
  const std::uint32_t dataLen = static_cast<std::uint32_t>(s.size() * 4);
  o.write("RIFF", 4);
  w32(36 + dataLen);
  o.write("WAVEfmt ", 8);
  w32(16);
  w16(3);
  w16(1);
  w32(static_cast<std::uint32_t>(sr));
  w32(static_cast<std::uint32_t>(sr) * 4);
  w16(4);
  w16(32);
  o.write("data", 4);
  w32(dataLen);
  o.write(reinterpret_cast<const char*>(s.data()), static_cast<std::streamsize>(dataLen));
  if (!o) {
    error = "ecriture interrompue";
    return false;
  }
  return true;
}

// ---------------------------------------------------------------- banques
std::string userBankId(int nx, int ny, const std::vector<float>& raw) {
  std::vector<std::uint8_t> buf(8 + raw.size() * 4);
  const std::uint32_t d[2] = {static_cast<std::uint32_t>(nx), static_cast<std::uint32_t>(ny)};
  std::memcpy(buf.data(), d, 8);
  std::memcpy(buf.data() + 8, raw.data(), raw.size() * 4);
  return "user:" + sha256Hex(buf.data(), buf.size()).substr(0, 16);
}

std::shared_ptr<const WtBank> bankFromCycles(const std::vector<float>& signal, int cycle, const std::string& name, std::string& error) {
  if (cycle < 16 || cycle > 65536) {
    error = "longueur de cycle hors limites (16..65536)";
    return nullptr;
  }
  const int count = static_cast<int>(signal.size() / static_cast<std::size_t>(cycle));
  if (count < 1) {
    error = "signal plus court qu'un cycle";
    return nullptr;
  }
  const int waves = std::min(count, WtBank::kMaxX * WtBank::kMaxY);
  const int nx = std::min(waves, WtBank::kMaxX), ny = (waves + nx - 1) / nx;
  const int N = WtBank::kLength;
  std::vector<float> raw(static_cast<std::size_t>(nx) * ny * N);
  // rééchantillonnage spectral (troncature/complétion du spectre, conservation de la forme sur un cycle)
  int fftN = 1;
  while (fftN < std::max(cycle, N)) fftN <<= 1;
  Fft fin(cycle == fftN ? fftN : 0), fout(N);
  for (int w = 0; w < nx * ny; ++w) {
    const int src = std::min(w, waves - 1);
    const float* c = &signal[static_cast<std::size_t>(src) * static_cast<std::size_t>(cycle)];
    std::vector<std::complex<double>> spec(static_cast<std::size_t>(N), 0.0);
    if (cycle == N) {
      for (int i = 0; i < N; ++i) raw[static_cast<std::size_t>(w) * N + static_cast<std::size_t>(i)] = c[i];
      continue;
    }
    // DFT directe du cycle (hors RT ; cycles courts typiques) puis synthèse à 2048 points
    const int H = std::min(cycle / 2, N / 2 - 1);
    for (int k = 0; k <= H; ++k) {
      std::complex<double> s(0.0, 0.0);
      for (int i = 0; i < cycle; ++i) s += static_cast<double>(c[i]) * std::polar(1.0, -2.0 * 3.14159265358979323846 * k * i / cycle);
      s *= static_cast<double>(N) / cycle;
      spec[static_cast<std::size_t>(k)] = s;
      if (k > 0) spec[static_cast<std::size_t>(N - k)] = std::conj(s);
    }
    fout.transform(spec.data(), true);
    for (int i = 0; i < N; ++i) raw[static_cast<std::size_t>(w) * N + static_cast<std::size_t>(i)] = static_cast<float>(spec[static_cast<std::size_t>(i)].real() / N);
  }
  (void)fin;
  return WtBank::build(userBankId(nx, ny, raw), name, nx, ny, raw, WtBank::Options{}, error);
}

namespace {
std::mutex gUserMutex;
std::map<std::string, std::shared_ptr<const WtBank>>& userMap() {
  static std::map<std::string, std::shared_ptr<const WtBank>> m;
  return m;
}
}  // namespace

void UserBanks::add(std::shared_ptr<const WtBank> bank) {
  std::lock_guard<std::mutex> lock(gUserMutex);
  userMap()[bank->id()] = std::move(bank);
}
std::shared_ptr<const WtBank> UserBanks::get(const std::string& id) {
  std::lock_guard<std::mutex> lock(gUserMutex);
  auto it = userMap().find(id);
  return it == userMap().end() ? nullptr : it->second;
}
std::vector<std::string> UserBanks::ids() {
  std::lock_guard<std::mutex> lock(gUserMutex);
  std::vector<std::string> v;
  for (auto& kv : userMap()) v.push_back(kv.first);
  return v;
}

std::shared_ptr<const WtBank> resolveBank(const std::string& id) {
  if (auto b = FactoryBanks::get(id)) return b;
  return UserBanks::get(id);
}

}  // namespace sbd
