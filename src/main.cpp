#include <iostream>

void runBT(long long maxNodes);
void runFB();

int main() {
  int opcion = 1;

  while (opcion > 0 && opcion <= 2) {
    std::cout << "\n=== MENU DE EJECUCION ===\n";
    std::cout << "0. Salir\n";
    std::cout << "1. Fuerza Bruta\n";
    std::cout << "2. Backtracking\n";
    std::cout << "Opcion: ";
    std::cin >> opcion;

    if (opcion == 1) {
      runFB();
    }

    if (opcion == 2) {
      long long maxNodes = 50000000LL;
      runBT(maxNodes);
    }
  }

  return 0;
}
