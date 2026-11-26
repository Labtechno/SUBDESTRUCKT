#include "WtBank.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <map>
#include <mutex>

#include "DspConstants.h"
#include "Fft.h"
#include "Wavetable.h"

namespace sbd {

std::shared_ptr<const WtBank> WtBank::build(const std::string& id, const std::string& name, int nx, int ny,
                                            const std::vector<float>& raw, const Options& opt, std::string& error) {
  if (nx < 1 || nx > kMaxX || ny < 1 || ny > kMaxY) {
    error = "dimensions de banque hors limites (1..64 x 1..16)";
    return nullptr;
  }
  if (raw.size() != static_cast<size_t>(nx) * ny * kLength) {
    error = "taille des donnees de banque incoherente";
    return nullptr;
  }
  for (float v : raw)
    if (!std::isfinite(v) || std::fabs(v) > 1e6f) {
      error = "echantillon non fini ou hors bornes";
      return nullptr;
    }
  std::shared_ptr<WtBank> b(new WtBank());
  b->id_ = id;
  b->name_ = name;
  b->nx_ = nx;
  b->ny_ = ny;
  b->raw_ = raw;
  const int W = nx * ny;
  b->mipsF_.assign(static_cast<size_t>(W) * kLevels * kStride, 0.0f);
  Fft fft(kLength);
  std::vector<std::complex<double>> spec(kLength), tmp(kLength);
  std::vector<double> level0Peak(static_cast<size_t>(W), 0.0);
  std::vector<double> lev(static_cast<size_t>(kLevels) * kLength);
  for (int w = 0; w < W; ++w) {
    for (int i = 0; i < kLength; ++i) spec[static_cast<size_t>(i)] = raw[static_cast<size_t>(w) * kLength + i];
    fft.transform(spec.data(), false);
    if (opt.removeDc) spec[0] = 0.0;
    for (int L = 0; L < kLevels; ++L) {
      const int H = 1 << (kLevels - 1 - L);
      std::fill(tmp.begin(), tmp.end(), std::complex<double>(0.0, 0.0));
      tmp[0] = spec[0];
      for (int h = 1; h <= H && h <= kLength / 2; ++h) {
        tmp[static_cast<size_t>(h)] = spec[static_cast<size_t>(h)];
        if (h < kLength / 2) tmp[static_cast<size_t>(kLength - h)] = spec[static_cast<size_t>(kLength - h)];
      }
      fft.transform(tmp.data(), true);
      for (int i = 0; i < kLength; ++i) lev[static_cast<size_t>(L) * kLength + i] = tmp[static_cast<size_t>(i)].real() / kLength;
    }
    double pk = 0.0;
    for (int i = 0; i < kLength; ++i) pk = std::max(pk, std::fabs(lev[static_cast<size_t>(i)]));
    level0Peak[static_cast<size_t>(w)] = pk;
    for (int L = 0; L < kLevels; ++L) {
      float* t = &b->mipsF_[(static_cast<size_t>(w) * kLevels + L) * kStride];
      for (int i = 0; i < kLength; ++i) t[i] = static_cast<float>(lev[static_cast<size_t>(L) * kLength + i]);
    }
  }
  double bankPeak = 0.0;
  for (double p : level0Peak) bankPeak = std::max(bankPeak, p);
  for (int w = 0; w < W; ++w) {
    double g = 1.0;
    if (opt.normalize == WtNormalize::PerWave && level0Peak[static_cast<size_t>(w)] > 1e-12) g = 1.0 / level0Peak[static_cast<size_t>(w)];
    if (opt.normalize == WtNormalize::PerBank && bankPeak > 1e-12) g = 1.0 / bankPeak;
    for (int L = 0; L < kLevels; ++L) {
      float* t = &b->mipsF_[(static_cast<size_t>(w) * kLevels + L) * kStride];
      if (g != 1.0)
        for (int i = 0; i < kLength; ++i) t[i] = static_cast<float>(t[i] * g);
      t[kLength] = t[0];  // échantillon de garde
    }
  }
  return b;
}

std::shared_ptr<const WtBank> WtBank::buildB5Saw(const std::string& id, const std::string& name) {
  std::shared_ptr<WtBank> b(new WtBank());
  b->id_ = id;
  b->name_ = name;
  b->precise_ = true;
  const SawWavetable t;  // algorithme de la tranche 0.1.0-b5 (sommation directe, double)
  b->mipsD_.assign(static_cast<size_t>(kLevels) * kStride, 0.0);
  for (int L = 0; L < kLevels; ++L)
    for (int i = 0; i <= kLength; ++i) b->mipsD_[static_cast<size_t>(L) * kStride + static_cast<size_t>(i)] = t.level(L)[i];
  b->raw_.resize(kLength);
  for (int i = 0; i < kLength; ++i) b->raw_[static_cast<size_t>(i)] = static_cast<float>(t.level(0)[i]);
  return b;
}

namespace {
template <typename T>
inline double lerpLevel(const T* t, double phase) noexcept {
  const double x = phase * WtBank::kLength;
  int i = static_cast<int>(x);
  if (i >= WtBank::kLength) i = WtBank::kLength - 1;
  if (i < 0) i = 0;
  const double f = x - i;
  return t[i] + f * (static_cast<double>(t[i + 1]) - t[i]);
}
template <typename T>
inline double cubicLevel(const T* t, double phase) noexcept {
  const double x = phase * WtBank::kLength;
  int i = static_cast<int>(x);
  if (i >= WtBank::kLength) i = WtBank::kLength - 1;
  if (i < 0) i = 0;
  const double f = x - i;
  const int N = WtBank::kLength;
  const double y0 = t[(i - 1 + N) % N], y1 = t[i], y2 = t[i + 1], y3 = t[(i + 2) % N];
  const double c1 = 0.5 * (y2 - y0), c2 = y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3, c3 = 0.5 * (y3 - y0) + 1.5 * (y1 - y2);
  return ((c3 * f + c2) * f + c1) * f + y1;
}
}  // namespace

namespace {
template <typename T>
double readLevels(const T* base, double phase, double absIncrement, bool raw, bool cubic) noexcept {
  constexpr int kL = WtBank::kLevels;
  auto lev = [base](int L) { return base + static_cast<size_t>(L) * WtBank::kStride; };
  if (raw) return cubic ? cubicLevel(lev(0), phase) : lerpLevel(lev(0), phase);
  // niveau continu : harmonique max admissible = 0.5/inc ; le niveau L admet 2^(10-L) => L >= 10 - log2(0.5/inc)
  const double lf = (absIncrement > 1e-12) ? 10.0 - std::log2(0.5 / absIncrement) : -1.0;
  int i0;
  double w;
  if (lf <= -1.0) {
    i0 = 0;
    w = 0.0;
  } else {
    i0 = static_cast<int>(std::ceil(lf));
    w = lf - static_cast<double>(i0 - 1);  // (0,1]
    if (i0 < 0) {
      i0 = 0;
      w = 0.0;
    }
  }
  if (i0 >= kL - 1) return cubic ? cubicLevel(lev(kL - 1), phase) : lerpLevel(lev(kL - 1), phase);
  const double a = cubic ? cubicLevel(lev(i0), phase) : lerpLevel(lev(i0), phase);
  const double b = cubic ? cubicLevel(lev(i0 + 1), phase) : lerpLevel(lev(i0 + 1), phase);
  return a + w * (b - a);
}
}  // namespace

double WtBank::read(int wave, double phase, double absIncrement, bool raw, bool cubic) const noexcept {
  const size_t o = static_cast<size_t>(wave) * kLevels * kStride;
  return precise_ ? readLevels(mipsD_.data() + o, phase, absIncrement, raw, cubic)
                  : readLevels(mipsF_.data() + o, phase, absIncrement, raw, cubic);
}

// ---------------------------------------------------------------- générateur
std::vector<float> WtGenerator::additive(const std::vector<double>& amp, const std::vector<double>& phase) {
  const int N = WtBank::kLength;
  Fft fft(N);
  std::vector<std::complex<double>> s(static_cast<size_t>(N));
  for (size_t h = 1; h <= amp.size() && h < static_cast<size_t>(N / 2); ++h) {
    const double a = amp[h - 1], p = h - 1 < phase.size() ? phase[h - 1] : 0.0;
    // a*sin(2pi h n/N + p) : coefficient complexe sur les bins h et N-h
    const std::complex<double> c = std::polar(a * N / 2.0, p - kPi / 2.0);
    s[h] = c;
    s[static_cast<size_t>(N) - h] = std::conj(c);
  }
  fft.transform(s.data(), true);
  std::vector<float> out(static_cast<size_t>(N));
  for (int i = 0; i < N; ++i) out[static_cast<size_t>(i)] = static_cast<float>(s[static_cast<size_t>(i)].real() / N);
  return out;
}

std::vector<float> WtGenerator::saw() {
  // identique en spectre à la table de la tranche b5 : amplitude 1/h, phase pi sur les harmoniques paires
  std::vector<double> a(1024), p(1024);
  for (int h = 1; h <= 1024; ++h) {
    a[static_cast<size_t>(h - 1)] = 1.0 / h;
    p[static_cast<size_t>(h - 1)] = (h % 2) ? 0.0 : kPi;
  }
  return additive(a, p);
}
std::vector<float> WtGenerator::sine() { return additive({1.0}, {0.0}); }
std::vector<float> WtGenerator::triangle() {
  std::vector<double> a(1024, 0.0), p(1024, 0.0);
  for (int h = 1; h <= 1024; h += 2) {
    a[static_cast<size_t>(h - 1)] = 1.0 / (static_cast<double>(h) * h);
    p[static_cast<size_t>(h - 1)] = ((h / 2) % 2) ? kPi : 0.0;
  }
  return additive(a, p);
}
std::vector<float> WtGenerator::square() {
  std::vector<double> a(1024, 0.0), p(1024, 0.0);
  for (int h = 1; h <= 1024; h += 2) a[static_cast<size_t>(h - 1)] = 1.0 / h;
  return additive(a, p);
}
std::vector<float> WtGenerator::pulse(double width) {
  std::vector<double> a(1024, 0.0), p(1024, 0.0);
  for (int h = 1; h <= 1024; ++h) {
    // impulsion rectangulaire de largeur w : coefficients en sin(pi h w)/h
    a[static_cast<size_t>(h - 1)] = std::fabs(std::sin(kPi * h * width)) / h;
    p[static_cast<size_t>(h - 1)] = (std::sin(kPi * h * width) >= 0 ? kPi / 2.0 : -kPi / 2.0) - kPi * h * width;
  }
  return additive(a, p);
}
std::vector<float> WtGenerator::foldedSine(double gain) {
  std::vector<float> o(WtBank::kLength);
  for (int i = 0; i < WtBank::kLength; ++i) {
    double u = gain * std::sin(kTwoPi * i / WtBank::kLength);
    // repli triangulaire dans [-1,1]
    u = std::fmod(u + 1.0, 4.0);
    if (u < 0) u += 4.0;
    o[static_cast<size_t>(i)] = static_cast<float>(u <= 2.0 ? u - 1.0 : 3.0 - u);
  }
  return o;
}
std::vector<float> WtGenerator::randomSpectrum(std::uint64_t seed, int harmonics, double tilt, double density) {
  std::uint64_t s = seed * 6364136223846793005ULL + 1442695040888963407ULL;
  auto next = [&s]() {
    s = s * 6364136223846793005ULL + 1442695040888963407ULL;
    return static_cast<double>((s >> 11) & ((1ULL << 53) - 1)) / static_cast<double>(1ULL << 53);
  };
  std::vector<double> a(static_cast<size_t>(harmonics), 0.0), p(static_cast<size_t>(harmonics), 0.0);
  for (int h = 1; h <= harmonics; ++h) {
    const double keep = next(), amp = next(), ph = next();
    if (h == 1 || keep < density) a[static_cast<size_t>(h - 1)] = (0.25 + 0.75 * amp) * std::pow(h, -tilt);
    p[static_cast<size_t>(h - 1)] = kTwoPi * ph;
  }
  return additive(a, p);
}
std::vector<float> WtGenerator::formant(double f1, double f2, double bw) {
  std::vector<double> a(256, 0.0), p(256, 0.0);
  for (int h = 1; h <= 256; ++h) {
    const double g1 = std::exp(-0.5 * std::pow((h - f1) / bw, 2.0)), g2 = 0.7 * std::exp(-0.5 * std::pow((h - f2) / bw, 2.0));
    a[static_cast<size_t>(h - 1)] = (g1 + g2 + 0.02) / std::sqrt(static_cast<double>(h));
    p[static_cast<size_t>(h - 1)] = (h % 3) * kPi / 3.0;
  }
  return additive(a, p);
}

// ---------------------------------------------------------------- banques usine
namespace {
struct FactoryDef {
  const char* id;
  const char* name;
};
const FactoryDef kFactory[] = {
    {"factory:saw", "Saw (b5)"},           {"factory:basic", "Basic Shapes"}, {"factory:pwm", "Pulse Width"},
    {"factory:harmonic", "Harmonic Series"}, {"factory:metal", "Metal Spectra"}, {"factory:fold", "Folded Sine"},
    {"factory:formant", "Formants"},
};

std::shared_ptr<const WtBank> makeFactory(const std::string& id) {
  std::vector<float> raw;
  int nx = 1, ny = 1;
  auto push = [&raw](const std::vector<float>& w) { raw.insert(raw.end(), w.begin(), w.end()); };
  if (id == "factory:saw") {
    return WtBank::buildB5Saw(id, FactoryBanks::displayName(id));
  } else if (id == "factory:basic") {
    nx = 4;
    push(WtGenerator::sine());
    push(WtGenerator::triangle());
    push(WtGenerator::saw());
    push(WtGenerator::square());
  } else if (id == "factory:pwm") {
    nx = 16;
    for (int i = 0; i < 16; ++i) push(WtGenerator::pulse(0.5 - 0.45 * i / 15.0));
  } else if (id == "factory:harmonic") {
    nx = 16;
    ny = 4;
    for (int t = 0; t < 4; ++t)
      for (int i = 0; i < 16; ++i) {
        const int n = 1 + i * 4;
        std::vector<double> a(static_cast<size_t>(n)), p(static_cast<size_t>(n), 0.0);
        for (int h = 1; h <= n; ++h) a[static_cast<size_t>(h - 1)] = std::pow(h, -(0.5 + 0.5 * t));
        push(WtGenerator::additive(a, p));
      }
  } else if (id == "factory:metal") {
    nx = 16;
    ny = 4;
    for (int t = 0; t < 4; ++t)
      for (int i = 0; i < 16; ++i) push(WtGenerator::randomSpectrum(0x51DE57ULL + 97ULL * t + i, 512, 0.6 + 0.2 * t, 0.05 + 0.06 * i));
  } else if (id == "factory:fold") {
    nx = 16;
    for (int i = 0; i < 16; ++i) push(WtGenerator::foldedSine(1.0 + 0.6 * i));
  } else if (id == "factory:formant") {
    nx = 16;
    ny = 4;
    for (int t = 0; t < 4; ++t)
      for (int i = 0; i < 16; ++i) push(WtGenerator::formant(2.0 + i * 1.5, 6.0 + i * 3.0 + t * 6.0, 1.5 + t));
  } else {
    return nullptr;
  }
  std::string err;
  return WtBank::build(id, FactoryBanks::displayName(id), nx, ny, raw, WtBank::Options{}, err);
}
}  // namespace

const std::vector<std::string>& FactoryBanks::ids() {
  static const std::vector<std::string> v = [] {
    std::vector<std::string> r;
    for (const auto& f : kFactory) r.emplace_back(f.id);
    return r;
  }();
  return v;
}

std::string FactoryBanks::displayName(const std::string& id) {
  for (const auto& f : kFactory)
    if (id == f.id) return f.name;
  return id;
}

std::shared_ptr<const WtBank> FactoryBanks::get(const std::string& id) {
  static std::mutex m;  // hors RT uniquement
  static std::map<std::string, std::shared_ptr<const WtBank>> cache;
  std::lock_guard<std::mutex> lock(m);
  auto it = cache.find(id);
  if (it != cache.end()) return it->second;
  auto b = makeFactory(id);
  if (b) cache[id] = b;
  return b;
}

}  // namespace sbd
