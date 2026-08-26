#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <unordered_set>
#include "language.hpp"
#include "lexer.hpp"

struct ObfuscatorOptions {
    int level = 2; // 1 light, 2 medium, 3 heavy
    bool renameIdentifiers = true;
    bool encodeStrings = true;
    bool encodeNumbers = true;
    bool removeComments = true;
    bool minifyWhitespace = true;
    bool injectDeadCode = false;
    bool preserveMain = true;
    unsigned int seed = 0; // 0 = random
    bool keepPreprocessor = true;
    std::string stringEncoding = "default"; // e.g. base64, hex-escape, rot13, aes, etc.
    std::string fileEncoding = "utf-8"; // output file encoding
    std::string xorKey = "K";
    std::string vigenereKey = "KEY";
    int caesarShift = 3;
};

class Obfuscator {
public:
    Obfuscator(LanguageProfile lang, ObfuscatorOptions opts);
    std::string obfuscate(const std::string& code);

    // expose mapping for debugging
    std::unordered_map<std::string, std::string> getIdentifierMap() const { return identMap; }

private:
    LanguageProfile lang;
    ObfuscatorOptions opts;
    std::mt19937 rng;
    std::unordered_map<std::string, std::string> identMap;
    std::unordered_set<std::string> preserved;

    std::string generateObfuscatedName();
    std::string obfuscateStringContent(const std::string& tokenValue);
    std::string obfuscateNumberContent(const std::string& tokenValue);
    void buildPreservedSet();
    void collectIdentifiers(const std::vector<Token>& tokens);
};
