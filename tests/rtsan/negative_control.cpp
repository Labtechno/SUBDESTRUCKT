// SUBDESTRUCKT — contrôle négatif RealtimeSanitizer : une allocation volontaire dans une fonction [[clang::nonblocking]]
// DOIT être détectée (arrêt du programme avec un rapport RTSan). Si ce programme se termine normalement, l'outil n'est
// pas actif et le résultat « 0 violation » du moteur ne prouve rien.
#include <cstdio>
#include <cstdlib>

#include "engine/Engine.h"  // fournit SBD_NONBLOCKING

namespace {
volatile int sink = 0;
int allocateInRealtime(int n) noexcept SBD_NONBLOCKING {
  int* p = static_cast<int*>(std::malloc(sizeof(int) * static_cast<size_t>(n)));  // violation attendue
  p[0] = n;
  const int v = p[0];
  std::free(p);
  return v;
}
}  // namespace

int main() {
  sink = allocateInRealtime(16);
  std::printf("controle negatif : aucune violation detectee (RTSan inactif)\n");
  return 0;
}
