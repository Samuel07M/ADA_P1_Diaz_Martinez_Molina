#pragma once
#include <string>

namespace fb {

struct FBConfig {
    std::string alphabet;     
    int minLength;            
    int maxLength;             
    std::string targetHashHex; 
};

struct FBResult {
    bool found;
    std::string password;          
    long long candidatesEvaluated;  
    double elapsedMs;               
};

std::string sha256Hex(const std::string& input);

FBResult bruteForceSearch(const FBConfig& config);

FBResult dictionaryAttack(const std::string& targetHashHex, const std::string& dictionaryPath);

}
