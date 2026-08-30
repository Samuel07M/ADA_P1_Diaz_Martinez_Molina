#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP


#include <iostream>
#include <string>

namespace testutils {
inline int& fallosGlobales() {
    static int fallos = 0;
    return fallos;
}
inline int& checksGlobales() {
    static int checks = 0;
    return checks;
}
}  // namespace testutils

#define CHECK(cond, descripcion)                                             \
    do {                                                                     \
        ++testutils::checksGlobales();                                      \
        if (!(cond)) {                                                      \
            ++testutils::fallosGlobales();                                  \
            std::cerr << "[FALLO] " << descripcion << "  (" << #cond << ", " \
                      << __FILE__ << ":" << __LINE__ << ")\n";               \
        } else {                                                             \
            std::cout << "[OK]    " << descripcion << "\n";                  \
        }                                                                     \
    } while (0)

#define CHECK_EQ(actual, esperado, descripcion)                              \
    do {                                                                     \
        ++testutils::checksGlobales();                                      \
        auto a_ = (actual);                                                  \
        auto e_ = (esperado);                                                \
        if (!(a_ == e_)) {                                                   \
            ++testutils::fallosGlobales();                                  \
            std::cerr << "[FALLO] " << descripcion << "  esperado=" << e_    \
                      << " actual=" << a_ << "  (" << __FILE__ << ":"        \
                      << __LINE__ << ")\n";                                  \
        } else {                                                             \
            std::cout << "[OK]    " << descripcion << " (=" << a_ << ")\n";  \
        }                                                                     \
    } while (0)


inline int resumenFinal(const std::string& nombreSuite) {
    int fallos = testutils::fallosGlobales();
    int total = testutils::checksGlobales();
    std::cout << "\n=== " << nombreSuite << ": " << (total - fallos) << "/"
              << total << " checks OK";
    if (fallos > 0) std::cout << "  (" << fallos << " FALLARON)";
    std::cout << " ===\n";
    return fallos == 0 ? 0 : 1;
}

#endif
