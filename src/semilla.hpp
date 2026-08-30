#ifndef SEMILLA_HPP
#define SEMILLA_HPP

#include <cstdint>
#include <string>
#include <vector>


namespace semilla {


std::string normalizarApellido(const std::string& apellido);

uint64_t calcularSemilla(std::vector<std::string> apellidos);

uint64_t siguienteLCG(uint64_t x);

std::vector<std::string> generarContrasenasEquipo(
    uint64_t semillaInicial, const std::vector<std::string>& alfabetos,
    const std::vector<int>& longitudes);

struct ParametrosPoliticaBT {
    int minLower;
    int minUpper;
    int minDigit;
    int minSymbol;
};

ParametrosPoliticaBT derivarPoliticaBT(uint64_t semillaEquipo);

}  // namespace semilla

#endif
