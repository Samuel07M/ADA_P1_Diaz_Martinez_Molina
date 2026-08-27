#ifndef BT_BACKTRACKING_HPP
#define BT_BACKTRACKING_HPP

#include <string>
#include <vector>
#include <cstdint>

enum class CharType { LOWER, UPPER, DIGIT, SYMBOL };

CharType classify(char c);

std::vector<char> buildAlphabet();

struct Policy {
    int minLower = 0;
    int minUpper = 0;
    int minDigit = 0;
    int minSymbol = 0;
    bool noConsecutiveRepeats = true;
};

struct Counts {
    int lower = 0, upper = 0, digit = 0, symbol = 0;
};

bool esFactible(const Counts& c, int longitudActual, int n, const Policy& p);

bool esSolucionValida(const Counts& c, const Policy& p);

struct SearchStats {
    long long nodesVisited = 0;
    long long nodesGenerated = 0;
    long long solutionsFound = 0;
    double timeMs = 0.0;
    bool exhaustiveSkipped = false;
    bool nodeLimitReached = false;
};

SearchStats backtrackConPoda(const std::vector<char>& alphabet, int n,
                              const Policy& policy,
                              std::vector<std::string>* outSolutions = nullptr,
                              long long maxSolutionsToStore = 0,
                              long long maxNodes = 200000000LL);

SearchStats fuerzaBrutaSinPoda(const std::vector<char>& alphabet, int n,
                                 const Policy& policy,
                                 long long maxNodesAllowed = 200000000LL);

long long tamanoTeoricoArbol(int alphabetSize, int n, bool* overflowed);

#endif