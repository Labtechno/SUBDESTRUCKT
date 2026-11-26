// SUBDESTRUCKT — L1 — actifs wavetable utilisateur (référentiel §5, §9 ; architecture §3.12) : import WAV, export WAV,
// identifiant de contenu SHA-256 (données canoniques float32), registre des banques utilisateur (immuables, COW).
// Tout s'exécute hors RT. Import borné et traité comme non fiable.
#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../dsp/WtBank.h"

namespace sbd {

std::string sha256Hex(const void* data, std::size_t size);

// Lecture WAV PCM 16/24/32 bits entiers ou float32, mixage mono ; refus des fichiers invalides ou > 64 Mio.
bool readWavMono(const std::string& path, std::vector<float>& out, double& sampleRate, std::string& error);
bool readWavMonoFromMemory(const std::vector<std::uint8_t>& bytes, std::vector<float>& out, double& sampleRate, std::string& error);
// Écrit des échantillons float32 mono (export exact du contenu brut).
bool writeWavFloat(const std::string& path, const std::vector<float>& samples, double sampleRate, std::string& error);

// Découpe un signal en cycles de `cycleLength` échantillons, rééchantillonne chaque cycle à 2048 (par le spectre),
// dispose jusqu'à 64 x 16 formes (X d'abord), complète la dernière table par la dernière forme.
std::shared_ptr<const WtBank> bankFromCycles(const std::vector<float>& signal, int cycleLength, const std::string& name,
                                             std::string& error);
// Identifiant de contenu « user:<16 hex> » (préfixe du SHA-256 des données brutes et dimensions).
std::string userBankId(int nx, int ny, const std::vector<float>& raw);

// Registre des banques utilisateur (processus) : immuables, partagées ; jamais supprimées tant qu'elles sont référencées.
struct UserBanks {
  static void add(std::shared_ptr<const WtBank> bank);
  static std::shared_ptr<const WtBank> get(const std::string& id);
  static std::vector<std::string> ids();
};
// Résolution d'un identifiant de banque (usine ou utilisateur).
std::shared_ptr<const WtBank> resolveBank(const std::string& id);

}  // namespace sbd
