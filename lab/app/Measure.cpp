// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Component Lab — mesures (implémentation).
// Les largeurs sont mesurées par juce::GlyphArrangement::getStringWidth avec la police d'interface de 0.4.1
// (sans-serif système par défaut), au corps exact de chaque zoom : ce sont des mesures, pas des estimations.
#include "app/Measure.h"

#include <cmath>
#include <set>

#include "r2/R2Style.h"

namespace sbd::lab {

namespace t = sbd::ui::tok;

std::vector<juce::StringArray> readCsv(const juce::File& f) {
  std::vector<juce::StringArray> rows;
  juce::String text = f.loadFileAsString();
  if (text.startsWithChar(static_cast<juce::juce_wchar>(0xFEFF))) text = text.substring(1);
  juce::StringArray row;
  juce::String cell;
  bool q = false;
  auto p = text.getCharPointer();
  while (!p.isEmpty()) {
    const auto c = p.getAndAdvance();
    if (q) {
      if (c == '"') {
        if (*p == '"') {
          cell << '"';
          ++p;
        } else q = false;
      } else cell << juce::String::charToString(c);
    } else if (c == '"') q = true;
    else if (c == ',') {
      row.add(cell);
      cell.clear();
    } else if (c == '\n' || c == '\r') {
      if (c == '\r' && *p == '\n') ++p;
      row.add(cell);
      cell.clear();
      rows.push_back(row);
      row.clear();
    } else cell << juce::String::charToString(c);
  }
  if (cell.isNotEmpty() || !row.isEmpty()) {
    row.add(cell);
    rows.push_back(row);
  }
  return rows;
}

bool writeCsv(const juce::File& f, const std::vector<juce::StringArray>& rows) {
  juce::String out = juce::String::charToString(static_cast<juce::juce_wchar>(0xFEFF));
  for (const auto& r : rows) {
    for (int k = 0; k < r.size(); ++k) {
      auto c = r[k];
      if (c.containsAnyOf(",\"\n\r")) c = "\"" + c.replace("\"", "\"\"") + "\"";
      out << (k ? "," : "") << c;
    }
    out << "\r\n";
  }
  f.getParentDirectory().createDirectory();
  return f.replaceWithText(out, false, false, "\r\n");
}

namespace {

juce::String num(double v, int d = 2) { return juce::String(v, d); }

bool isCritical(const juce::String& ref, const juce::String& occurrence, const juce::String& location) {
  // repères critiques de façade (R2-DESIGN-SYSTEM-SPEC.md §3, OD-06) : PANIC, crête, état d'autonomie, axes du Core,
  // WT X/Y/Z, CROSS LVL, macros — en PERFORMANCE, dans la barre et dans le Core.
  static const std::set<juce::String> params{"wt.x", "wt.y", "wt.z", "cross.master", "cross.fwd", "cross.rev", "macro1", "macro2", "macro3", "macro4"};
  static const std::set<juce::String> offreg{"bar.peak", "bar.panic", "bar.authorize", "bar.drone", "perf.core.x-axis", "perf.core.y-axis",
                                             "perf.auth.armed", "perf.auth.exec", "state.armed", "state.active", "state.render", "state.suspended",
                                             "state.revoked", "state.configured", "state.authorized"};
  if (offreg.count(ref) > 0) return true;
  return params.count(ref) > 0 && occurrence.startsWith("Variante") && location.startsWith("PERFORMANCE");
}

float bestTwoLines(const juce::Font& f, const juce::String& s) {
  float best = 1.0e9f;
  for (int k = s.indexOfChar(' '); k >= 0; k = s.indexOfChar(k + 1, ' '))
    best = std::min(best, std::max(r2::draw::textWidth(f, s.substring(0, k).trimEnd()), r2::draw::textWidth(f, s.substring(k + 1).trimStart())));
  return best;
}

}  // namespace

int runMeasure(const juce::File& csvIn, const juce::File& csvOut, juce::StringArray& log) {
  const auto rows = readCsv(csvIn);
  if (rows.size() < 2) {
    log.add("PT-02: cannot read " + csvIn.getFullPathName());
    return 1;
  }
  const auto& h = rows[0];
  const auto col = [&](const char* name) { return h.indexOf(juce::String::fromUTF8(name)); };
  const int cRef = col("Référence"), cOcc = col("Occurrence"), cFam = col("Famille graphique"), cLoc = col("Emplacement"),  // utf8-ok
            cLab = col("Libellé court proposé"), cBody = col("Corps px log."), cBud = col("Budget px log."), cPri = col("Priorité"),  // utf8-ok
            cEst = col("Largeur proposée estimée px log.");  // utf8-ok
  if (cRef < 0 || cLab < 0 || cBody < 0 || cBud < 0) {
    log.add("PT-02: unexpected header");
    return 1;
  }
  const auto platform = juce::SystemStats::getOperatingSystemName();
  const auto fontName = r2::labelFont(12.0f).getTypefacePtr()->getName();
  std::vector<juce::StringArray> out;
  out.push_back(juce::StringArray{juce::String::fromUTF8("Référence"), "Occurrence", "Famille graphique", "Emplacement",
                                  juce::String::fromUTF8("Libellé court proposé"), juce::String::fromUTF8("Priorité"),
                                  juce::String::fromUTF8("Repère critique"), "Corps CSV px log.", "Corps R2 retenu px log.", "Graisse",
                                  "Budget px log.", "Lignes admises", juce::String::fromUTF8("Largeur mesurée 100 % px"),
                                  juce::String::fromUTF8("Largeur mesurée 75 % px écran"), juce::String::fromUTF8("Largeur mesurée 150 % px écran"),
                                  juce::String::fromUTF8("Corps écran 75 % px"), juce::String::fromUTF8("Plancher 9 px à 75 %"),
                                  juce::String::fromUTF8("Lignes nécessaires"), "Ligne la plus longue px", "Marge 100 % px",
                                  "Verdict 100 %", "Verdict 75 %", "Verdict 150 %", juce::String::fromUTF8("Largeur estimée R2-A px"),
                                  juce::String::fromUTF8("Écart mesure - estimation px"), "Plateforme", "Police"});
  int counts[5] = {0, 0, 0, 0, 0};  // OK, marge, 2 lignes, dépassement, sans budget
  int crit = 0, critFloor = 0;
  for (size_t i = 1; i < rows.size(); ++i) {
    const auto& r = rows[i];
    if (r.size() <= std::max({cRef, cLab, cBody, cBud})) continue;
    const auto ref = r[cRef], occ = r[cOcc], fam = r[cFam], loc = r[cLoc], lab = r[cLab];
    const bool critical = isCritical(ref, occ, loc);
    const float body0 = r[cBody].isEmpty() ? 11.0f : r[cBody].getFloatValue();
    const float body = critical ? std::max(body0, r2::tok2::fontCritical) : body0;
    const bool bold = occ.startsWith("Titre") || occ.startsWith("Hors registre") || occ.startsWith(juce::String::fromUTF8("État")) ||
                      fam == "NEXUS XL";
    const bool two = fam.contains("NEXUS") || fam.contains("NEURAL") || fam == "PLASMA V";
    const bool hasBudget = r[cBud].isNotEmpty();
    const float budget = r[cBud].getFloatValue();
    float w[3];
    const float zs[3] = {0.75f, 1.0f, 1.5f};
    for (int z = 0; z < 3; ++z) w[z] = r2::draw::textWidth(r2::labelFont(body * zs[z], bold), lab);
    const auto f100 = r2::labelFont(body, bold);
    int lines = 1;
    float longest = w[1];
    juce::String v[3];
    for (int z = 0; z < 3; ++z) {
      if (!hasBudget) {
        v[z] = "BUDGET_NON_MESURE";
        continue;
      }
      const float bz = budget * zs[z];
      if (w[z] <= bz) v[z] = (bz - w[z] < 8.0f * zs[z]) ? "OK_MARGE<8" : "OK";
      else if (two && lab.containsChar(' ') && bestTwoLines(r2::labelFont(body * zs[z], bold), lab) <= bz) v[z] = "2_LIGNES";
      else v[z] = juce::String::fromUTF8("DÉPASSEMENT");
    }
    if (hasBudget && w[1] > budget && two && lab.containsChar(' ')) {
      longest = bestTwoLines(f100, lab);
      if (longest <= budget) lines = 2;
    }
    const int vi = !hasBudget ? 4 : (v[1] == "OK" ? 0 : (v[1] == "OK_MARGE<8" ? 1 : (v[1] == "2_LIGNES" ? 2 : 3)));
    ++counts[vi];
    const float screen75 = body * 0.75f;
    if (critical) {
      ++crit;
      if (screen75 >= 8.999f) ++critFloor;
    }
    const float est = cEst >= 0 ? r[cEst].getFloatValue() : 0.0f;
    out.push_back(juce::StringArray{ref, occ, fam, loc, lab, cPri >= 0 ? r[cPri] : juce::String(), critical ? "oui" : "non", num(body0, 1), num(body, 1),
                                    bold ? "gras" : "normal", hasBudget ? num(budget) : juce::String(), juce::String(two ? 2 : 1), num(w[1]), num(w[0]),
                                    num(w[2]), num(screen75), critical ? (screen75 >= 8.999f ? "atteint" : "NON ATTEINT") : "n/a", juce::String(lines),
                                    num(longest), hasBudget ? num(budget - w[1]) : juce::String(), v[1], v[0], v[2],
                                    cEst >= 0 ? r[cEst] : juce::String(), cEst >= 0 && r[cEst].isNotEmpty() ? num(w[1] - est) : juce::String(),
                                    platform, fontName});
  }
  const bool ok = writeCsv(csvOut, out);
  log.add("PT-02 " + platform + " font " + fontName + ": " + juce::String(static_cast<int>(out.size()) - 1) + " occurrences; 100%: OK " +
          juce::String(counts[0]) + ", OK_MARGE<8 " + juce::String(counts[1]) + ", 2_LIGNES " + juce::String(counts[2]) + juce::String::fromUTF8(", DÉPASSEMENT ") +
          juce::String(counts[3]) + ", BUDGET_NON_MESURE " + juce::String(counts[4]) + "; critiques " + juce::String(crit) + ", plancher 9 px a 75 % " +
          juce::String(critFloor) + "/" + juce::String(crit));
  return ok ? 0 : 1;
}

int runGlyphs(const juce::File& csvOut, juce::StringArray& log) {
  struct G {
    juce::juce_wchar cp;
    const char *name, *ascii;
  };
  const G glyphs[4] = {{0x2192, "RIGHTWARDS ARROW", ">"}, {0x00B7, "MIDDLE DOT", "."}, {0x2026, "HORIZONTAL ELLIPSIS", "..."}, {0x2212, "MINUS SIGN", "-"}};
  std::vector<juce::StringArray> out;
  juce::StringArray head;
  for (const char* h : {"Plateforme", "Police (rôle)", "Police résolue", "Graisse", "Point de code", "Nom", "Glyphe natif", "Police de substitution",  // utf8-ok
                        "Avance à 12 px", "Repli ASCII"})  // utf8-ok
    head.add(juce::String::fromUTF8(h));
  out.push_back(head);
  int native = 0, total = 0;
  for (int role = 0; role < 2; ++role)
    for (int b = 0; b < 2; ++b) {
      const auto f = role == 0 ? r2::labelFont(12.0f, b == 1) : r2::valueFont(12.0f, b == 1);
      const auto tf = f.getTypefacePtr();
      for (const auto& g : glyphs) {
        const juce::String s = juce::String::charToString(g.cp);
        const bool has = tf != nullptr && tf->getNominalGlyphForCodepoint(g.cp).has_value();
        const auto fb = has ? juce::String() : f.findSuitableFontForText(s).getTypefacePtr()->getName();
        ++total;
        native += has ? 1 : 0;
        out.push_back(juce::StringArray{juce::SystemStats::getOperatingSystemName(), role == 0 ? "interface (sans)" : "valeurs (mono)",
                                        tf != nullptr ? tf->getName() : juce::String("?"), b ? "gras" : "normal",
                                        "U+" + juce::String::toHexString(static_cast<int>(g.cp)).toUpperCase().paddedLeft('0', 4), g.name, has ? "oui" : "NON",
                                        fb, num(r2::draw::textWidth(f, s)), g.ascii});
      }
    }
  const bool ok = writeCsv(csvOut, out);
  log.add("PT-05 " + juce::SystemStats::getOperatingSystemName() + ": glyphes natifs " + juce::String(native) + "/" + juce::String(total));
  return ok ? 0 : 1;
}

namespace {
double lin(double c) { return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4); }
double lum(juce::Colour c) { return 0.2126 * lin(c.getFloatRed()) + 0.7152 * lin(c.getFloatGreen()) + 0.0722 * lin(c.getFloatBlue()); }
double ratio(juce::Colour a, juce::Colour b) {
  const double la = lum(a), lb = lum(b);
  return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}
}  // namespace

int runContrast(const juce::File& csvOut, juce::StringArray& log) {
  using namespace sbd::r2;
  struct P {
    const char *fg, *bg, *use;
    juce::Colour f, b;
    double min;
  };
  const P pairs[] = {
      {"text", "relief1 (plaque)", "sérigraphie, valeurs", t::text, tok2::relief1, 4.5},  // utf8-ok
      {"text", "carbon (fenêtre de valeur)", "valeur numérique", t::text, t::carbon, 4.5},  // utf8-ok
      {"text", "glass (afficheur)", "titre d'afficheur", t::text, t::glass, 4.5},
      {"text", "relief3 (touche d'action)", "libellé d'action", t::text, tok2::relief3, 4.5},  // utf8-ok
      {"text", "relief2 (touche plate)", "libellé de bascule", t::text, tok2::relief2, 4.5},  // utf8-ok
      {"textDim", "relief1", "texte secondaire", t::textDim, tok2::relief1, 4.5},
      {"textDim", "glass", "source d'afficheur", t::textDim, t::glass, 4.5},
      {"textDim", "relief2", "état OFF", t::textDim, tok2::relief2, 4.5},  // utf8-ok
      {"textMute", "relief1", "libellé inactif", t::textMute, tok2::relief1, 3.0},  // utf8-ok
      {"textMute", "glass", "graduation", t::textMute, t::glass, 3.0},
      {"wt", "glass", "trace WT", t::wt, t::glass, 3.0},
      {"fm", "glass", "trace FM", t::fm, t::glass, 3.0},
      {"cross", "glass", "trace CROSS", t::cross, t::glass, 3.0},
      {"mod", "glass", "trace MOD / EFFECTIVE", t::mod, t::glass, 3.0},
      {"wt", "relief1", "arc BASE WT sur plaque", t::wt, tok2::relief1, 3.0},
      {"fm", "relief1", "arc BASE FM sur plaque", t::fm, tok2::relief1, 3.0},
      {"cross", "relief1", "arc BASE CROSS sur plaque", t::cross, tok2::relief1, 3.0},
      {"mod", "relief1", "anneau EFFECTIVE sur plaque", t::mod, tok2::relief1, 3.0},
      {"warn", "relief2", "état ARMED", t::warn, tok2::relief2, 4.5},  // utf8-ok
      {"warn", "glass", "étiquettes INDICATIVE / DEMO DATA", t::warn, t::glass, 4.5},  // utf8-ok
      {"white", "alert", "texte PANIC", juce::Colours::white, t::alert, 3.0},
      {"alert", "glass", "crête > 0 dBFS", t::alert, t::glass, 3.0},  // utf8-ok
      {"focus", "relief1", "anneau de focus", t::focus, tok2::relief1, 3.0},
      {"track", "relief1", "piste d'arc (décor)", t::track, tok2::relief1, 1.0},  // utf8-ok
      {"relief1", "relief0", "niveau plaque / châssis", tok2::relief1, tok2::relief0, 1.15},  // utf8-ok
      {"relief2", "relief1", "niveau corps / plaque", tok2::relief2, tok2::relief1, 1.15},
      {"relief3", "relief2", "niveau action / corps", tok2::relief3, tok2::relief2, 1.15}};
  std::vector<juce::StringArray> out;
  out.push_back(juce::StringArray{"Avant-plan", "Fond", "Usage", "Avant-plan hex", "Fond hex", "Rapport WCAG", "Seuil", "Verdict"});
  int pass = 0, n = 0;
  for (const auto& p : pairs) {
    const double r = ratio(p.f, p.b);
    const bool ok = r + 1.0e-9 >= p.min;
    pass += ok ? 1 : 0;
    ++n;
    out.push_back(juce::StringArray{p.fg, juce::String::fromUTF8(p.bg), juce::String::fromUTF8(p.use), "#" + p.f.toDisplayString(false),
                                    "#" + p.b.toDisplayString(false), num(r, 2), num(p.min, 2), ok ? "PASS" : "FAIL"});
  }
  const bool ok = writeCsv(csvOut, out);
  log.add("Contrastes : " + juce::String(pass) + "/" + juce::String(n) + " PASS");
  return ok ? 0 : 1;
}

}  // namespace sbd::lab
