// SUBDESTRUCKT — contrôle négatif RealtimeSanitizer : une allocation volontaire dans une fonction [[clang::nonblocking]]
// DOIT être détectée (arrêt du programme avec un rapport RTSan). Si ce programme se termine normalement, l'outil n'est
// pas actif et le résultat « 0 violation » du moteur ne prouve rien.
// L'adresse allouée est publiée dans une variable volatile globale : sinon l'optimiseur peut supprimer le couple
// malloc/free local (élision d'allocation) et il n'y aurait rien à détecter. Compiler en -O0.
#include <cstdio>
#include <cstdlib>

#include "engine/Engine.h"  // fournit SBD_NONBLOCKING

namespace {
int* volatile g_escape = nullptr;
int allocateInRealtime(int n) noexcept SBD_NONBLOCKING {
  int* p = static_cast<int*>(std::malloc(sizeof(int) * static_cast<size_t>(n)));  // violation attendue
  g_escape = p;
  p[0] = n;
  const int v = p[0];
  std::free(p);
  return v;
}
}  // namespace

int main() {
  const int v = allocateInRealtime(16);
  std::printf("controle negatif : aucune violation detectee (RTSan inactif), v=%d\n", v);
  return 0;
}
