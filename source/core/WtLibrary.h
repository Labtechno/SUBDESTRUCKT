// SUBDESTRUCKT — L1 — édition, génération procédurale et bibliothèque des banques wavetable utilisateur (ARCH-G4.1 §3).
// Tout s'exécute hors RT (thread de contrôle) : une banque éditée ou générée est une NOUVELLE banque immuable
// (identifiant de contenu user:<sha>), préparée (mip-maps, spectres) avant publication au moteur. La grille reste
// bornée à 64 x 16 formes de 2048 échantillons ; le band-limiting, la phase, le morph et la modulation suivent les
// contrats de WtBank (aucun chemin particulier pour les banques utilisateur).
#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../dsp/WtBank.h"

namespace sbd {

// Générateurs procéduraux : balayage X d'un paramètre principal, variation Y d'un paramètre secondaire.
enum class WtProcedure : int { HarmonicTilt = 0, PulseWidth, RandomSpectra, Formant, Fold, Count };
const char* procedureName(WtProcedure p) noexcept;
// Retourne nx*ny*2048 échantillons (X d'abord). Déterministe pour (procédure, nx, ny, seed, amount).
std::vector<float> generateProcedural(WtProcedure p, int nx, int ny, std::uint64_t seed, double amount);

// Forme dessinée : points de contrôle (phase 0..1, valeur -1..1) interpolés linéairement (bouclage), DC retiré,
// crête normalisée à 1 (forme nulle conservée nulle). Au moins 2 points.
std::vector<float> drawnWave(const std::vector<std::pair<double, double>>& points);
// Remplace la forme `wave` d'une banque (copie : la banque d'origine reste immuable).
bool replaceWave(const std::vector<float>& raw, int nx, int ny, int wave, const std::vector<float>& samples,
                 std::vector<float>& out, std::string& error);

// Fichier de banque portable .sdwt : texte, en-tête versionné, nom, dimensions, données float32 base64, SHA-256 du
// contenu (identifiant recalculé et vérifié au chargement ; fichier altéré refusé).
std::string encodeBankFile(const WtBank& bank);
std::shared_ptr<const WtBank> decodeBankFile(const std::string& text, std::string& error);
bool saveBankFile(const std::string& path, const WtBank& bank, std::string& error);
std::shared_ptr<const WtBank> loadBankFile(const std::string& path, std::string& error);

// Bibliothèque de banques utilisateur (répertoire de fichiers .sdwt).
struct BankFileInfo {
  std::string path, name, id;
  int nx = 1, ny = 1;
};
std::vector<BankFileInfo> listBankFiles(const std::string& directory);

}  // namespace sbd
