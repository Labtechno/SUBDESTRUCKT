#include "WtLibrary.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "../dsp/Rng.h"
#include "StateCodec.h"
#include "WtAssets.h"

namespace sbd {

namespace {
constexpr int N = WtBank::kLength;
constexpr double kTwoPiL = 6.283185307179586476925286766559;

void appendWave(std::vector<float>& raw, const std::vector<float>& w) { raw.insert(raw.end(), w.begin(), w.begin() + N); }
}  // namespace

const char* procedureName(WtProcedure p) noexcept {
  switch (p) {
    case WtProcedure::HarmonicTilt: return "Harmonic Tilt";
    case WtProcedure::PulseWidth: return "Pulse Width";
    case WtProcedure::RandomSpectra: return "Random Spectra";
    case WtProcedure::Formant: return "Formant Sweep";
    case WtProcedure::Fold: return "Fold Sweep";
    default: return "?";
  }
}

std::vector<float> generateProcedural(WtProcedure p, int nx, int ny, std::uint64_t seed, double amount) {
  nx = std::clamp(nx, 1, WtBank::kMaxX);
  ny = std::clamp(ny, 1, WtBank::kMaxY);
  amount = std::clamp(amount, 0.0, 1.0);
  std::vector<float> raw;
  raw.reserve(static_cast<size_t>(nx) * ny * N);
  for (int y = 0; y < ny; ++y)
    for (int x = 0; x < nx; ++x) {
      const double u = nx > 1 ? static_cast<double>(x) / (nx - 1) : 0.0;  // balayage principal
      const double v = ny > 1 ? static_cast<double>(y) / (ny - 1) : 0.0;  // variation secondaire
      switch (p) {
        case WtProcedure::HarmonicTilt: {
          const int n = 1 + static_cast<int>(std::lround(u * (8 + 248 * amount)));
          std::vector<double> a(static_cast<size_t>(n)), ph(static_cast<size_t>(n), 0.0);
          for (int h = 1; h <= n; ++h) {
            a[static_cast<size_t>(h - 1)] = std::pow(h, -(0.4 + 1.6 * v));
            ph[static_cast<size_t>(h - 1)] = (h % 2 == 0 ? 0.5 : 0.0) * kTwoPiL * v;
          }
          appendWave(raw, WtGenerator::additive(a, ph));
          break;
        }
        case WtProcedure::PulseWidth:
          appendWave(raw, WtGenerator::pulse(0.5 - (0.05 + 0.4 * amount) * u * (1.0 - 0.5 * v)));
          break;
        case WtProcedure::RandomSpectra:
          appendWave(raw, WtGenerator::randomSpectrum(seed * 1315423911ULL + static_cast<std::uint64_t>(y * 64 + x), 64 + static_cast<int>(448 * amount),
                                                      0.5 + 1.0 * v, 0.05 + 0.9 * u));
          break;
        case WtProcedure::Formant:
          appendWave(raw, WtGenerator::formant(300.0 + 900.0 * u, 900.0 + 1800.0 * v * (0.5 + amount), 60.0 + 200.0 * (1.0 - amount)));
          break;
        case WtProcedure::Fold:
          appendWave(raw, WtGenerator::foldedSine(1.0 + (2.0 + 10.0 * amount) * u + 2.0 * v));
          break;
        default: appendWave(raw, WtGenerator::sine());
      }
    }
  return raw;
}

std::vector<float> drawnWave(const std::vector<std::pair<double, double>>& pointsIn) {
  std::vector<float> w(N, 0.0f);
  if (pointsIn.size() < 2) return w;
  auto pts = pointsIn;
  for (auto& p : pts) {
    p.first = p.first - std::floor(p.first);
    p.second = std::clamp(p.second, -1.0, 1.0);
  }
  std::sort(pts.begin(), pts.end());
  std::vector<double> d(N);
  const size_t m = pts.size();
  for (int i = 0; i < N; ++i) {
    const double t = static_cast<double>(i) / N;
    // segment [pk, pk+1] contenant t (bouclage entre le dernier et le premier point)
    size_t k = m - 1;
    for (size_t j = 0; j < m; ++j)
      if (pts[j].first <= t) k = j;
    const auto& a = pts[k];
    const auto& b = pts[(k + 1) % m];
    double ta = a.first, tb = b.first, tt = t;
    if (tb <= ta) tb += 1.0;
    if (tt < ta) tt += 1.0;
    const double f = tb - ta > 1e-12 ? (tt - ta) / (tb - ta) : 0.0;
    d[static_cast<size_t>(i)] = a.second + f * (b.second - a.second);
  }
  double mean = 0.0;
  for (double x : d) mean += x;
  mean /= N;
  double pk = 0.0;
  for (double& x : d) {
    x -= mean;
    pk = std::max(pk, std::fabs(x));
  }
  for (int i = 0; i < N; ++i) w[static_cast<size_t>(i)] = pk > 1e-9 ? static_cast<float>(d[static_cast<size_t>(i)] / pk) : 0.0f;
  return w;
}

bool replaceWave(const std::vector<float>& raw, int nx, int ny, int wave, const std::vector<float>& samples, std::vector<float>& out,
                 std::string& error) {
  if (raw.size() != static_cast<size_t>(nx) * ny * N || wave < 0 || wave >= nx * ny || samples.size() != static_cast<size_t>(N)) {
    error = "edition de forme : dimensions invalides";
    return false;
  }
  for (float s : samples)
    if (!std::isfinite(s)) {
      error = "edition de forme : valeur non finie";
      return false;
    }
  out = raw;
  std::copy(samples.begin(), samples.end(), out.begin() + static_cast<std::ptrdiff_t>(wave) * N);
  return true;
}

// ------------------------------------------------------------------ fichier .sdwt
std::string encodeBankFile(const WtBank& bank) {
  const auto& raw = bank.raw();
  std::ostringstream o;
  o << "SUBDESTRUCKT_WT 1\n";
  o << "name " << base64Encode(reinterpret_cast<const std::uint8_t*>(bank.name().data()), bank.name().size()) << "\n";
  o << "grid " << bank.nx() << " " << bank.ny() << "\n";
  o << "sha256 " << sha256Hex(raw.data(), raw.size() * sizeof(float)) << "\n";
  o << "data " << base64Encode(reinterpret_cast<const std::uint8_t*>(raw.data()), raw.size() * sizeof(float)) << "\n";
  o << "end\n";
  return o.str();
}

std::shared_ptr<const WtBank> decodeBankFile(const std::string& text, std::string& error) {
  std::istringstream in(text);
  std::string line, name, sha;
  int nx = 0, ny = 0;
  std::vector<std::uint8_t> bytes;
  bool header = false, end = false, haveData = false;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    std::istringstream w(line);
    std::string k;
    w >> k;
    if (!header) {
      int ver = 0;
      if (k != "SUBDESTRUCKT_WT" || !(w >> ver) || ver != 1) {
        error = "banque : en-tete ou version non pris en charge";
        return nullptr;
      }
      header = true;
    } else if (k == "name") {
      std::string b;
      w >> b;
      std::vector<std::uint8_t> nb;
      if (!b.empty() && !base64Decode(b, nb)) {
        error = "banque : nom invalide";
        return nullptr;
      }
      name.assign(nb.begin(), nb.end());
      name = name.substr(0, 64);
    } else if (k == "grid") {
      w >> nx >> ny;
    } else if (k == "sha256") {
      w >> sha;
    } else if (k == "data") {
      std::string b;
      w >> b;
      haveData = base64Decode(b, bytes);
    } else if (k == "end") {
      end = true;
      break;
    }
  }
  if (!header || !end || !haveData || nx < 1 || ny < 1 || nx > WtBank::kMaxX || ny > WtBank::kMaxY ||
      bytes.size() != static_cast<size_t>(nx) * ny * N * sizeof(float)) {
    error = "banque : fichier incomplet ou dimensions invalides";
    return nullptr;
  }
  std::vector<float> raw(bytes.size() / sizeof(float));
  std::memcpy(raw.data(), bytes.data(), bytes.size());
  if (sha256Hex(raw.data(), raw.size() * sizeof(float)) != sha) {
    error = "banque : integrite (SHA-256) non verifiee";
    return nullptr;
  }
  for (float s : raw)
    if (!std::isfinite(s) || std::fabs(s) > 16.0f) {
      error = "banque : valeur hors bornes";
      return nullptr;
    }
  return WtBank::build(userBankId(nx, ny, raw), name.empty() ? "User" : name, nx, ny, raw, WtBank::Options{}, error);
}

bool saveBankFile(const std::string& path, const WtBank& bank, std::string& error) {
  std::ofstream f(std::filesystem::u8path(path), std::ios::binary | std::ios::trunc);
  if (!f) {
    error = "banque : ecriture impossible";
    return false;
  }
  f << encodeBankFile(bank);
  return static_cast<bool>(f);
}

std::shared_ptr<const WtBank> loadBankFile(const std::string& path, std::string& error) {
  std::error_code ec;
  const auto size = std::filesystem::file_size(std::filesystem::u8path(path), ec);
  if (ec || size > 64u * 1024u * 1024u) {
    error = "banque : fichier absent ou trop grand";
    return nullptr;
  }
  std::ifstream f(std::filesystem::u8path(path), std::ios::binary);
  std::string text((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  return decodeBankFile(text, error);
}

std::vector<BankFileInfo> listBankFiles(const std::string& directory) {
  std::vector<BankFileInfo> out;
  std::error_code ec;
  for (const auto& e : std::filesystem::directory_iterator(std::filesystem::u8path(directory), ec)) {
    if (!e.is_regular_file() || e.path().extension() != ".sdwt") continue;
    std::string err;
    if (auto b = loadBankFile(e.path().u8string(), err)) out.push_back({e.path().u8string(), b->name(), b->id(), b->nx(), b->ny()});
  }
  std::sort(out.begin(), out.end(), [](const BankFileInfo& a, const BankFileInfo& b) { return a.name < b.name; });
  return out;
}

}  // namespace sbd
