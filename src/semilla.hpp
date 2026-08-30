#ifndef SEMILLA_HPP
#define SEMILLA_HPP

#include <cstdint>
#include <string>
#include <vector>

// Modulo compartido por FB y BT (y por tests/) que implementa el
// procedimiento reproducible de generacion de semilla e instancias
// descrito en la Seccion 9 del enunciado. Se centraliza aqui para que
// ambos modulos y la suite de pruebas usen exactamente la misma logica,
// en vez de reimplementarla por separado.
namespace semilla {

// Convierte un apellido a minusculas, sin espacios y sin tildes/dieresis
// (a-z unicamente). Ej: "Diaz" -> "diaz", "Muñoz" -> "munoz".
std::string normalizarApellido(const std::string& apellido);

// Ordena alfabeticamente los apellidos (ya normalizados o no -- esta
// funcion los normaliza internamente), los concatena sin espacios y
// calcula semilla = (suma de los codigos ASCII de la cadena resultante)
// mod 100000, tal como exige la Seccion 9.1.
uint64_t calcularSemilla(std::vector<std::string> apellidos);

// Un paso del generador congruencial lineal exigido por la Seccion 9.1:
// x_{i+1} = (1103515245 * x_i + 12345) mod 2^31.
uint64_t siguienteLCG(uint64_t x);

// Genera las contrasenas objetivo del equipo. alfabetos[i] y
// longitudes[i] describen la contrasena i-esima (mismo orden y tamano).
// El flujo x_0, x_1, x_2, ... es UNICO Y CONTINUO a traves de todas las
// contrasenas (no se reinicia al pasar de una contrasena a la
// siguiente): x_0 = semilla, y el caracter j-esimo de la contrasena i
// se toma como alfabetos[i][x_k mod |alfabetos[i]|] avanzando x_k en
// cada caracter emitido, en el orden en que aparecen las contrasenas.
std::vector<std::string> generarContrasenasEquipo(
    uint64_t semillaInicial, const std::vector<std::string>& alfabetos,
    const std::vector<int>& longitudes);

// Parametros de la politica de contrasenas del Modulo BT derivados de la
// semilla del equipo (Seccion 9.2):
//   minLower  = 2 + (semilla mod 3)
//   minUpper  = 1 + (semilla mod 2)
//   minDigit  = 1 + (semilla mod 3)
//   minSymbol = 1
struct ParametrosPoliticaBT {
    int minLower;
    int minUpper;
    int minDigit;
    int minSymbol;
};

ParametrosPoliticaBT derivarPoliticaBT(uint64_t semillaEquipo);

}  // namespace semilla

#endif
