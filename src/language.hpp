#pragma once
#include <string>
#include <unordered_set>
#include <vector>

struct LanguageProfile {
    std::string name;
    std::vector<std::string> extensions;
    std::unordered_set<std::string> keywords;
    std::string singleLineComment; // e.g. // or # or --
    std::string multiLineCommentStart; // e.g. /*
    std::string multiLineCommentEnd;   // e.g. */
    std::vector<std::string> stringDelimiters; // ", ', """, `, etc
    char charDelimiter = '\''; // for char literals
    bool hasPreprocessor = false; // # for C/C++
    std::unordered_set<std::string> preservedIdentifiers; // e.g. main, if not to rename
};

LanguageProfile getLanguageProfile(const std::string& langName);
LanguageProfile detectLanguage(const std::string& filename);
std::vector<LanguageProfile> getAllLanguages();
