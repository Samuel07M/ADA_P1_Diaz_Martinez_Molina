#include "fb_fuerzabruta.hpp"
#include "third_party/picosha2.h"
#include <chrono>
#include <fstream>

namespace fb {

std::string sha256Hex(const std::string& input) {
    std::string hashHex;
    picosha2::hash256_hex_string(input, hashHex);
    return hashHex;
}

static bool nextCandidate(std::string& candidate, const std::string& alphabet) {
    int pos = (int)candidate.size() - 1;
    bool carry = true;

    while (pos >= 0 && carry) {
        int idx = (int)alphabet.find(candidate[pos]);

        if (idx < (int)alphabet.size() - 1) {
            candidate[pos] = alphabet[idx + 1];
            carry = false;
        } else {
            candidate[pos] = alphabet[0];
            pos--;
        }
    }

    return carry; 
}

FBResult bruteForceSearch(const FBConfig& config) {
    auto startTime = std::chrono::high_resolution_clock::now();
    long long evaluated = 0;

    for (int n = config.minLength; n <= config.maxLength; n++) {

        std::string candidate(n, config.alphabet[0]);
        bool exhausted = false;

        while (!exhausted) {
            evaluated++;
            std::string hash = sha256Hex(candidate);

            if (hash == config.targetHashHex) {
                auto endTime = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                return FBResult{true, candidate, evaluated, ms};
            }

            exhausted = nextCandidate(candidate, config.alphabet);
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return FBResult{false, "", evaluated, ms};
}

FBResult dictionaryAttack(const std::string& targetHashHex,
                           const std::string& dictionaryPath) {
    auto startTime = std::chrono::high_resolution_clock::now();
    long long evaluated = 0;

    std::ifstream file(dictionaryPath);
    std::string candidate;

    while (std::getline(file, candidate)) {
        if (candidate.empty()) continue; // 
        evaluated++;

        std::string hash = sha256Hex(candidate);
        if (hash == targetHashHex) {
            auto endTime = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();
            return FBResult{true, candidate, evaluated, ms};
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return FBResult{false, "", evaluated, ms};
}

}
