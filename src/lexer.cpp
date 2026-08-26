#include "lexer.hpp"
#include <cctype>

std::vector<Token> tokenize(const std::string& code, const LanguageProfile& lang) {
    std::vector<Token> tokens;
    size_t i = 0;
    int line = 1;
    size_t n = code.size();

    auto isIdentStart = [](char c){ return std::isalpha((unsigned char)c) || c=='_'; };
    auto isIdentChar = [](char c){ return std::isalnum((unsigned char)c) || c=='_'; };

    while (i < n) {
        char c = code[i];

        // Newline
        if (c == '\n') {
            tokens.push_back({TokenType::Newline, "\n", line});
            line++; i++; continue;
        }
        if (c == '\r') { // handle \r\n
            if (i+1 < n && code[i+1]=='\n') i++;
            tokens.push_back({TokenType::Newline, "\n", line});
            line++; i++; continue;
        }

        // Whitespace (space, tab)
        if (c==' ' || c=='\t' || c=='\f' || c=='\v') {
            size_t start=i;
            while (i<n && (code[i]==' '||code[i]=='\t'||code[i]=='\f'||code[i]=='\v')) i++;
            tokens.push_back({TokenType::Whitespace, code.substr(start, i-start), line});
            continue;
        }

        // Preprocessor for C++: line starting with # after optional whitespace at beginning of line
        if (lang.hasPreprocessor && c=='#') {
            // check if at start of line (previous token is newline or beginning)
            bool atStart = tokens.empty() || tokens.back().type==TokenType::Newline;
            if (atStart) {
                size_t start=i;
                while (i<n && code[i]!='\n' && code[i]!='\r') i++;
                tokens.push_back({TokenType::Preprocessor, code.substr(start, i-start), line});
                continue;
            }
        }

        // Single line comment
        if (!lang.singleLineComment.empty() && code.compare(i, lang.singleLineComment.size(), lang.singleLineComment)==0) {
            // Python: need to ensure it's not inside string; handled before.
            // For lua, -- also start of --[[ handled as multiline first?
            // Check for multiline start that begins with same prefix: e.g. --[[ vs --
            // If multiline start also matches, prefer multiline.
            bool isMulti = false;
            if (!lang.multiLineCommentStart.empty() && code.compare(i, lang.multiLineCommentStart.size(), lang.multiLineCommentStart)==0) {
                isMulti = true;
            }
            if (!isMulti) {
                size_t start=i;
                while (i<n && code[i]!='\n' && code[i]!='\r') i++;
                tokens.push_back({TokenType::Comment, code.substr(start, i-start), line});
                continue;
            }
        }

        // Multi line comment
        if (!lang.multiLineCommentStart.empty() && code.compare(i, lang.multiLineCommentStart.size(), lang.multiLineCommentStart)==0) {
            size_t start=i;
            i+=lang.multiLineCommentStart.size();
            // special for lua --[[ ... ]]
            while (i<n) {
                if (!lang.multiLineCommentEnd.empty() && code.compare(i, lang.multiLineCommentEnd.size(), lang.multiLineCommentEnd)==0) {
                    i+=lang.multiLineCommentEnd.size();
                    break;
                }
                if (code[i]=='\n') line++;
                i++;
            }
            tokens.push_back({TokenType::Comment, code.substr(start, i-start), line});
            continue;
        }

        // Python string prefixes f, r, b, u, fr, rf, br etc.
        if (lang.name=="python") {
            // Check for prefix + quote
            size_t prefixLen=0;
            if (i<n && (code[i]=='f'||code[i]=='F'||code[i]=='r'||code[i]=='R'||code[i]=='b'||code[i]=='B'||code[i]=='u'||code[i]=='U')) {
                // Try 2-char prefix
                if (i+1<n && (code[i+1]=='f'||code[i+1]=='F'||code[i+1]=='r'||code[i+1]=='R'||code[i+1]=='b'||code[i+1]=='B')) {
                    // check if after 2-char prefix there's a quote
                    if (i+2<n && (code[i+2]=='"'||code[i+2]=='\'')) prefixLen=2;
                    else if (i+2<n && code.compare(i+2,3,"\"\"\"")==0) prefixLen=2;
                    else if (i+2<n && code.compare(i+2,3,"'''")==0) prefixLen=2;
                    else if (i+1<n && (code[i+1]=='"'||code[i+1]=='\'')) prefixLen=1; // 1-char prefix
                } else {
                    if (i+1<n && (code[i+1]=='"'||code[i+1]=='\'')) prefixLen=1;
                    else if (i+1<n && code.compare(i+1,3,"\"\"\"")==0) prefixLen=1;
                    else if (i+1<n && code.compare(i+1,3,"'''")==0) prefixLen=1;
                }
            }
            if (prefixLen>0) {
                // Check triple quote first
                bool isTriple=false;
                std::string delim;
                if (code.compare(i+prefixLen,3,"\"\"\"")==0) { delim="\"\"\""; isTriple=true; }
                else if (code.compare(i+prefixLen,3,"'''")==0) { delim="'''"; isTriple=true; }
                else if (code[i+prefixLen]=='"'||code[i+prefixLen]=='\'') { delim=std::string(1,code[i+prefixLen]); isTriple=false; }

                if (!delim.empty()) {
                    size_t start=i;
                    i+=prefixLen+delim.size();
                    while (i<n) {
                        if (isTriple && code.compare(i, delim.size(), delim)==0) { i+=delim.size(); break; }
                        if (!isTriple && code[i]==delim[0]) {
                            // check escaped?
                            if (i>0 && code[i-1]=='\\') { i++; continue; }
                            i++; break;
                        }
                        if (code[i]=='\n') line++;
                        // For single-line non-triple, break at newline (should not happen for valid Python but handle)
                        if (!isTriple && code[i]=='\n') break;
                        if (code[i]=='\\' && i+1<n) { i+=2; continue; }
                        i++;
                    }
                    tokens.push_back({TokenType::String, code.substr(start, i-start), line});
                    continue;
                }
            }
        }

        // String literals
        // Handle Python triple quotes first
        bool stringFound = false;
        for (auto &delim : lang.stringDelimiters) {
            if (delim.size()>1) { // triple quotes etc
                if (code.compare(i, delim.size(), delim)==0) {
                    // triple quoted
                    size_t start=i;
                    std::string d = delim;
                    i+=d.size();
                    while (i<n) {
                        if (code.compare(i, d.size(), d)==0) { i+=d.size(); break; }
                        if (code[i]=='\\' && i+1<n) { i+=2; continue; }
                        if (code[i]=='\n') line++;
                        i++;
                    }
                    tokens.push_back({TokenType::String, code.substr(start, i-start), line});
                    stringFound=true;
                    break;
                }
            }
        }
        if (stringFound) continue;

        // Single char delimiters " ' `
        if (c=='"' || c=='\'' || c=='`') {
            // check if this delim is allowed for this language
            std::string delim(1,c);
            bool allowed=false;
            for (auto &d: lang.stringDelimiters) if (d==delim) allowed=true;
            // For generic, allow both
            if (allowed || lang.name=="generic" || lang.name=="cpp" || lang.name=="javascript" || lang.name=="java" || lang.name=="csharp") {
                size_t start=i;
                char quote=c;
                i++;
                bool escaped=false;
                while (i<n) {
                    char cur=code[i];
                    if (cur=='\n' && quote!='`') { // strings not spanning lines except backtick/python
                        // Python allows multiline with " but our triple handled; break without closing to avoid infinite
                        // treat as end
                        break;
                    }
                    if (!escaped && cur=='\\') { escaped=true; i++; continue; }
                    if (!escaped && cur==quote) { i++; break; }
                    if (cur=='\n') line++;
                    escaped=false;
                    i++;
                }
                tokens.push_back({TokenType::String, code.substr(start, i-start), line});
                continue;
            }
        }

        // Lua [[ string
        if (c=='[' && i+1<n && code[i+1]=='[' && lang.name=="lua") {
            size_t start=i;
            i+=2;
            while (i<n) {
                if (code[i]==']' && i+1<n && code[i+1]==']') { i+=2; break; }
                if (code[i]=='\n') line++;
                i++;
            }
            tokens.push_back({TokenType::String, code.substr(start, i-start), line});
            continue;
        }

        // Number (integer/float/hex)
        if (std::isdigit((unsigned char)c) || (c=='.' && i+1<n && std::isdigit((unsigned char)code[i+1]))) {
            size_t start=i;
            if (c=='0' && i+1<n && (code[i+1]=='x' || code[i+1]=='X')) {
                i+=2;
                while (i<n && std::isxdigit((unsigned char)code[i])) i++;
            } else {
                while (i<n && (std::isdigit((unsigned char)code[i]) || code[i]=='.' || code[i]=='e' || code[i]=='E' || code[i]=='+' || code[i]=='-' || code[i]=='x' || code[i]=='b')) {
                    // break if symbol
                    if (code[i]=='+' || code[i]=='-') {
                        // only if previous was e/E
                        if (i>start && (code[i-1]=='e' || code[i-1]=='E')) { i++; continue; }
                        else break;
                    }
                    i++;
                    // prevent consuming too much: stop at whitespace or symbol
                    if (i<n && !std::isalnum((unsigned char)code[i]) && code[i]!='.' && code[i]!='_' && code[i]!='x' && code[i]!='b') break;
                }
            }
            tokens.push_back({TokenType::Number, code.substr(start, i-start), line});
            continue;
        }

        // Identifier / Keyword
        if (isIdentStart(c)) {
            size_t start=i;
            while (i<n && isIdentChar(code[i])) i++;
            std::string word = code.substr(start, i-start);
            if (lang.keywords.find(word)!=lang.keywords.end()) {
                tokens.push_back({TokenType::Keyword, word, line});
            } else {
                tokens.push_back({TokenType::Identifier, word, line});
            }
            continue;
        }

        // Symbols (including operators)
        // Try multi-char symbols first: ==, !=, <=, >=, &&, ||, ++, --, ->, ::, <<, >>, etc
        // Just take one char for generic approach, but handle 2-char
        size_t start=i;
        if (i+1<n) {
            std::string two = code.substr(i,2);
            if (two=="=="||two=="!="||two=="<="||two==">="||two=="&&"||two=="||"||two=="++"||two=="--"||two=="->"||two=="::"||two=="<<"||two==">>"||two=="+="||two=="-="||two=="*="||two=="/="||two=="%="||two=="&="||two=="|="||two=="^="||two=="//"||two=="**"||two==":=") {
                tokens.push_back({TokenType::Symbol, two, line});
                i+=2;
                continue;
            }
        }
        tokens.push_back({TokenType::Symbol, std::string(1,c), line});
        i++;
    }

    return tokens;
}

std::string tokensToString(const std::vector<Token>& tokens, bool minify, bool keepComments) {
    std::string out;
    for (auto &t: tokens) {
        if (t.type==TokenType::Comment && !keepComments) continue;
        if (minify) {
            if (t.type==TokenType::Whitespace || t.type==TokenType::Newline) {
                // keep at most one space if needed between alphanums to avoid merging tokens
                // We'll insert a single space if previous and next are identifier/keyword/number
                // Simpler: skip all whitespace, but ensure separation when needed during iteration
                continue;
            }
        }
        out+=t.value;
    }
    // If minify, need to re-insert minimal spaces where tokens would merge
    if (minify) {
        // We already skipped whitespaces: need to ensure e.g. "int a" doesn't become "inta"
        // So do second pass with smart spacing
        // Instead, rebuild with logic:
        std::string rebuilt;
        TokenType prevType = TokenType::Unknown;
        std::string prevVal;
        for (auto &t: tokens) {
            if (t.type==TokenType::Comment && !keepComments) continue;
            if (t.type==TokenType::Whitespace || t.type==TokenType::Newline) continue;
            if (!rebuilt.empty()) {
                bool needSpace = false;
                // if both prev and current are alphanum (identifier, keyword, number) they need space
                bool prevIsWord = (prevType==TokenType::Identifier || prevType==TokenType::Keyword || prevType==TokenType::Number);
                bool curIsWord = (t.type==TokenType::Identifier || t.type==TokenType::Keyword || t.type==TokenType::Number);
                if (prevIsWord && curIsWord) needSpace = true;
                // also cases like "} else" handled by above
                // Also avoid merging: e.g. if prev ends with alnum and cur starts with alnum/_ then need space
                if (needSpace) rebuilt += " ";
                else {
                    // Additional check: if prev is identifier/keyword and cur starts with digit? already covered
                    // check for cases where two symbols could merge incorrectly? e.g. "+ +" vs "++" -> if we had ++ as single token it's fine
                    // nothing
                }
            }
            rebuilt += t.value;
            prevType = t.type;
            prevVal = t.value;
        }
        return rebuilt;
    }
    return out;
}
