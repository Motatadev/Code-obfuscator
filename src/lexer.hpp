#pragma once
#include <string>
#include <vector>
#include "language.hpp"

enum class TokenType {
    Identifier,
    Keyword,
    Number,
    String,
    Comment,
    Preprocessor,
    Whitespace,
    Newline,
    Symbol,
    Unknown
};

struct Token {
    TokenType type;
    std::string value;
    int line = 0;
};

std::vector<Token> tokenize(const std::string& code, const LanguageProfile& lang);
std::string tokensToString(const std::vector<Token>& tokens, bool minify = false, bool keepComments = false);
