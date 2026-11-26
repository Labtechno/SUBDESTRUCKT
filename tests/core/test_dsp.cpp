// G-05 (retard T2 / Thiran1), G-06 (latence du décimateur) et contrôles des primitives.
#include <cmath>
#include <complex>
#include <cstdio>
#include <vector>

#include "Analysis.h"
#include "TestKit.h"
#include "dsp/CausalDelay.h"
#include "dsp/Decimator.h"
#include "dsp/Envelope.h"
#include "dsp/Wavetable.h"
#include "dsp/WtBank.h"
#include "engine/Engine.h"

using namespace sbd;

namespace {

// Réponse impulsionnelle mesurée du lecteur réel (CausalDelayLine), N échantillons.
std::vector<double> measuredImpulse(const DelaySpec& d, int N) {
  CausalDelayLine line;
  line.reset();
  std::vector<double> h(static_cast<size_t>(N));
  for (int n = 0; n < N; ++n) {
    h[static_cast<size_t>(n)] = line.read(d);
    line.write(n == 0 ? 1.0 : 0.0);
  }
  return h;
}

std::complex<double> dtft(const std::vector<double>& h, double omega) {
  std::complex<double> s(0.0, 0.0);
  for (size_t n = 0; n < h.size(); ++n) s += h[n] * std::exp(std::complex<double>(0.0, -omega * static_cast<double>(n)));
  return s;
}

double unwrappedPhaseDelay(const std::vector<double>& h, double omega, double D) {
  // phase attendue ~ -omega*D : on choisit la branche la plus proche
  const double ph = std::arg(dtft(h, omega));
  const double expected = -omega * D;
  const double k = std::round((expected - ph) / (2.0 * an::kPi));
  return -(ph + 2.0 * an::kPi * k) / omega;
}

void testDelays() {
  struct Case { double fs; };
  bool allOk = true;
  for (double fs : {44100.0, 48000.0, 88200.0, 96000.0, 22050.0, 32000.0, 192000.0}) {
    const int K = Engine::oversamplingFor(fs);
    const double fsE = fs * K;
    const DelaySpec d = makeT2Delay(fsE);
    bool ok = d.valid && d.D >= 1.0 && K > 0;
    const auto h = measuredImpulse(d, 8192);
    if (d.integer) {
      for (size_t n = 0; n < h.size(); ++n) ok = ok && (h[n] == (static_cast<int>(n) == d.di + 0 ? 1.0 : 0.0));
      tk::info("G-05 fs=%.0f K=%d fsE=%.0f D=%.6f entier exact : %s", fs, K, fsE, d.D, ok ? "oui" : "NON");
    } else {
      // causalité : aucune réponse avant d0 (lecture de x[n-d0] et x[n-d0-1] seulement)
      for (int n = 0; n < d.d0; ++n) ok = ok && h[static_cast<size_t>(n)] == 0.0;
      double magErr = 0.0;
      for (double f : {20.0, 100.0, 1000.0, 5000.0, 10000.0, 15000.0, 20000.0}) {
        const double om = 2.0 * an::kPi * f / fsE;
        magErr = std::max(magErr, std::fabs(std::abs(dtft(h, om)) - 1.0));
      }
      const double tauDc = unwrappedPhaseDelay(h, 2.0 * an::kPi * 0.01 / fsE, d.D);
      const double tau1k = unwrappedPhaseDelay(h, 2.0 * an::kPi * 1000.0 / fsE, d.D);
      ok = ok && std::fabs(d.a) < 1.0 && magErr <= 1e-12 && std::fabs(tauDc - d.D) <= 1e-6 &&
           std::fabs(tau1k - d.D) <= 0.005 * d.D;
      char line[512];
      int off = std::snprintf(line, sizeof line, "G-05 fs=%.0f K=%d fsE=%.0f D=%.6f Thiran1 d0=%d a=%.6f |H|-1 max=%.1e tau(DC)=%.9f tau(1k)=%.6f ; tau(f) :",
                              fs, K, fsE, d.D, d.d0, d.a, magErr, tauDc, tau1k);
      for (double f : {5000.0, 10000.0, 15000.0, 20000.0}) {
        if (f >= 0.5 * fs) continue;
        off += std::snprintf(line + off, sizeof line - static_cast<size_t>(off), " %.0fHz=%.5f", f,
                             unwrappedPhaseDelay(h, 2.0 * an::kPi * f / fsE, d.D));
      }
      tk::info("%s", line);
    }
    // concordance avec la réponse théorique
    const std::complex<double> th = delayResponse(d, 2.0 * an::kPi * 1000.0 / fsE);
    ok = ok && std::abs(th - dtft(h, 2.0 * an::kPi * 1000.0 / fsE)) < 1e-9;
    TK_CHECK(ok);
    allOk = allOk && ok;
  }
  // politique de K de la tranche
  const bool policy = Engine::oversamplingFor(44100.0) == 2 && Engine::oversamplingFor(48000.0) == 1 &&
                      Engine::oversamplingFor(88200.0) == 1 && Engine::oversamplingFor(96000.0) == 1 &&
                      Engine::oversamplingFor(8000.0) == 0;
  TK_CHECK(policy);
  tk::result("G-05", allOk && policy, "retard T2 : D entier exact ou Thiran1 (|H|=1, tau(DC)=D, tau(1k) a 0,5 %%), K=2 a 44,1 kHz, K=1 a 48/88,2/96 kHz");
}

void testDecimator() {
  bool ok = true;
  for (int K : {2, 4}) {
    Decimator dec(K);
    const int nh = 2048;
    std::vector<double> in(static_cast<size_t>(nh * K), 0.0), out(static_cast<size_t>(nh), 0.0);
    const int m = 300;
    in[static_cast<size_t>(m * K)] = 1.0;
    dec.process(in.data(), out.data(), nh);
    size_t arg = 0;
    for (size_t i = 0; i < out.size(); ++i)
      if (std::fabs(out[i]) > std::fabs(out[arg])) arg = i;
    const int measured = static_cast<int>(arg) - m;
    const bool kOk = measured == dec.latencyHostSamples();
    tk::info("G-06 K=%d latence declaree=%d, pic de la reponse impulsionnelle a +%d", K, dec.latencyHostSamples(), measured);
    ok = ok && kOk;
  }
  Engine e(FactoryBanks::get("factory:saw"));
  bool lat = true;
  for (double fs : {44100.0, 48000.0, 88200.0, 96000.0}) {
    e.prepare(fs, 512);
    const int expected = (fs == 44100.0) ? 32 : 0;
    lat = lat && e.setup().latencySamples == expected;
    tk::info("G-06 fs=%.0f K=%d latence=%d (attendu %d)", fs, e.setup().oversampling, e.setup().latencySamples, expected);
  }
  TK_CHECK(ok && lat);
  tk::result("G-06", ok && lat, "latence du decimateur = pic impulsionnel exact (K=2,4) ; 32 ech. a 44,1 kHz, 0 a 48/88,2/96 kHz");
}

void testPrimitives() {
  // rampe linéaire : cible exacte en fin de rampe
  LinearRamp r;
  r.setLength(7);
  r.reset(0.3);
  r.setTarget(0.0);
  double v = 1.0;
  for (int i = 0; i < 7; ++i) v = r.next();
  TK_CHECK(v == 0.0 && !r.ramping());
  // ADSR : attaque, sustain, relâchement jusqu'à 0 exact
  Adsr a;
  a.setSampleRate(1000.0);
  a.setParameters(0.01, 0.01, 0.5, 0.01);
  a.noteOn();
  for (int i = 0; i < 100; ++i) a.next();
  TK_CHECK(a.level() == 0.5);
  a.noteOff();
  for (int i = 0; i < 100; ++i) a.next();
  TK_CHECK(a.level() == 0.0 && a.stage() == Adsr::Stage::Idle);
  // table : harmoniques du niveau 0 en 1/k (contrôle direct de la table)
  // cycle unique périodique : DFT sans fenêtre, l'harmonique k tombe exactement sur le bin k
  const auto t = FactoryBanks::get("factory:saw");
  std::vector<std::complex<double>> c(WtBank::kLength);
  for (int i = 0; i < WtBank::kLength; ++i) c[static_cast<size_t>(i)] = t->sample(0, 0, i);
  an::fft(c);
  bool saw = true;
  for (int k = 2; k <= 10; ++k)
    saw = saw && std::fabs(20.0 * std::log10(std::abs(c[static_cast<size_t>(k)]) / std::abs(c[1]) * k)) < 0.01;
  TK_CHECK(saw);
}

}  // namespace

void runDspTests() {
  std::printf("[dsp]\n");
  testDelays();
  testDecimator();
  testPrimitives();
}
