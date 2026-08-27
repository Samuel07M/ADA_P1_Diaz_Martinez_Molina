#include "bt_backtracking.hpp"
#include <chrono>
#include <algorithm>
#include <functional>

CharType classify(char c) {
    if (c >= 'a' && c <= 'z') return CharType::LOWER;
    if (c >= 'A' && c <= 'Z') return CharType::UPPER;
    if (c >= '0' && c <= '9') return CharType::DIGIT;
    return CharType::SYMBOL;
}

std::vector<char> buildAlphabet() {
    std::vector<char> a;
    for (char c = 'a'; c <= 'z'; ++c) a.push_back(c);
    for (char c = 'A'; c <= 'Z'; ++c) a.push_back(c);
    for (char c = '0'; c <= '9'; ++c) a.push_back(c);
    for (char c : {'!', '@', '#', '$', '%'}) a.push_back(c);
    return a;
}

bool esFactible(const Counts& c, int longitudActual, int n, const Policy& p) {
    int restantes = n - longitudActual;
    int deficit = 0;
    deficit += std::max(0, p.minLower  - c.lower);
    deficit += std::max(0, p.minUpper  - c.upper);
    deficit += std::max(0, p.minDigit  - c.digit);
    deficit += std::max(0, p.minSymbol - c.symbol);
    return deficit <= restantes;
}

static bool sinRepetidosConsecutivos(const std::string& s) {
    for (size_t i = 1; i < s.size(); ++i)
        if (s[i] == s[i - 1]) return false;
    return true;
}

bool esSolucionValida(const Counts& c, const Policy& p) {
    return c.lower >= p.minLower && c.upper >= p.minUpper &&
           c.digit >= p.minDigit && c.symbol >= p.minSymbol;
}

static bool esSolucionValidaCompleta(const std::string& prefix, const Counts& c, const Policy& p) {
    if (p.noConsecutiveRepeats && !sinRepetidosConsecutivos(prefix)) return false;
    return esSolucionValida(c, p);
}

static void aplicarCaracter(Counts& counts, char c) {
    switch (classify(c)) {
        case CharType::LOWER:  counts.lower++;  break;
        case CharType::UPPER:  counts.upper++;  break;
        case CharType::DIGIT:  counts.digit++;  break;
        case CharType::SYMBOL: counts.symbol++; break;
    }
}

namespace {struct LimiteNodosAlcanzado {};}

static void backtrackRec(const std::vector<char>& alphabet, int n, const Policy& policy,
                          std::string& prefix, const Counts& counts, char lastChar, bool hasLast,
                          SearchStats& stats, long long maxNodes,
                          std::vector<std::string>* outSolutions, long long maxSolutionsToStore) {
    stats.nodesVisited++;
    if (stats.nodesVisited > maxNodes) {
        stats.nodeLimitReached = true;
        throw LimiteNodosAlcanzado{};
    }
    int len = (int)prefix.size();
    if (len == n) {
        if (esSolucionValida(counts, policy)) {
            stats.solutionsFound++;
            if (outSolutions && (long long)outSolutions->size() < maxSolutionsToStore)
                outSolutions->push_back(prefix);
        }
        return;
    }
    for (char c : alphabet) {
        if (policy.noConsecutiveRepeats && hasLast && c == lastChar) continue; // poda 1

        Counts newCounts = counts;
        aplicarCaracter(newCounts, c);

        if (!esFactible(newCounts, len + 1, n, policy)) continue; // poda 2

        prefix.push_back(c);
        backtrackRec(alphabet, n, policy, prefix, newCounts, c, true, stats, maxNodes,
                     outSolutions, maxSolutionsToStore);
        prefix.pop_back();
    }
}

SearchStats backtrackConPoda(const std::vector<char>& alphabet, int n, const Policy& policy,
                              std::vector<std::string>* outSolutions, long long maxSolutionsToStore,
                              long long maxNodes) {
    SearchStats stats;
    std::string prefix;
    prefix.reserve(n);
    Counts counts;

    auto t0 = std::chrono::high_resolution_clock::now();
    try {
        backtrackRec(alphabet, n, policy, prefix, counts, '\0', false, stats, maxNodes,
                     outSolutions, maxSolutionsToStore);
    } catch (const LimiteNodosAlcanzado&) {}
    auto t1 = std::chrono::high_resolution_clock::now();
    stats.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return stats;
}

static void exhaustiveRec(const std::vector<char>& alphabet, int n, const Policy& policy,
                            std::string& prefix, const Counts& counts, SearchStats& stats) {
    stats.nodesGenerated++;
    int len = (int)prefix.size();
    if (len == n) {
        if (esSolucionValidaCompleta(prefix, counts, policy)) stats.solutionsFound++;
        return;
    }
    for (char c : alphabet) {
        Counts nc = counts;
        aplicarCaracter(nc, c);
        prefix.push_back(c);
        exhaustiveRec(alphabet, n, policy, prefix, nc, stats);
        prefix.pop_back();
    }
}

long long tamanoTeoricoArbol(int alphabetSize, int n, bool* overflowed) {
    __int128 total = 0;
    __int128 term = 1;
    const __int128 LIMIT = (__int128)4000000000000000000LL;
    for (int k = 0; k <= n; ++k) {
        total += term;
        if (total > LIMIT) {
            if (overflowed) *overflowed = true;
            return -1;
        }
        term *= alphabetSize;
    }
    if (overflowed) *overflowed = false;
    return (long long) total;
}

SearchStats fuerzaBrutaSinPoda(const std::vector<char>& alphabet, int n, const Policy& policy, long long maxNodesAllowed) {
    SearchStats stats;
    bool overflowed = false;
    long long theoretical = tamanoTeoricoArbol((int)alphabet.size(), n, &overflowed);

    if (overflowed || theoretical < 0 || theoretical > maxNodesAllowed) {
        stats.exhaustiveSkipped = true;
        stats.nodesGenerated = theoretical;
        return stats;
    }

    std::string prefix;
    prefix.reserve(n);
    Counts counts;
    auto t0 = std::chrono::high_resolution_clock::now();
    exhaustiveRec(alphabet, n, policy, prefix, counts, stats);
    auto t1 = std::chrono::high_resolution_clock::now();
    stats.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return stats;
}