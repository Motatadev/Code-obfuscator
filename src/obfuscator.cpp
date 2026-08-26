#include "obfuscator.hpp"
#include "utils.hpp"
#include "encodings.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

Obfuscator::Obfuscator(LanguageProfile lang_, ObfuscatorOptions opts_)
    : lang(lang_), opts(opts_) {
    if (opts.seed == 0) {
        std::random_device rd;
        opts.seed = rd();
    }
    rng.seed(opts.seed);
    buildPreservedSet();
}

void Obfuscator::buildPreservedSet() {
    preserved = lang.preservedIdentifiers;
    // Always preserve language keywords (already handled) and common builtins
    // Add extra preserves for level light: don't rename short names? No
    // Common entry points
    std::vector<std::string> extra = {"main","Main","__main__","__name__","__init__","__import__","constructor","printf","scanf","cout","cin","include","define","import","from","require","module","exports","console","log","print","len","range","str","int","float","list","dict","base64","urllib","codecs","html","json","discord","aiohttp","asyncio","logging","typing","Optional","app_commands"};
    for (auto &s: extra) preserved.insert(s);

    // For JS/Python/C#, preserve 'self', 'this', 'super'
    preserved.insert("self");
    preserved.insert("this");
    preserved.insert("super");
}

std::string Obfuscator::generateObfuscatedName() {
    // Level 1: _0x + hex
    // Level 2: confusing Il1O
    // Level 3: even more confusing + hex mix
    if (opts.level <= 1) {
        return utils::randomHexString(rng, 6 + (rng()%4));
    } else if (opts.level == 2) {
        return utils::randomConfusingName(rng);
    } else {
        // heavy: mix
        if (rng()%2==0) return utils::randomHexString(rng, 8);
        else return utils::randomConfusingName(rng) + utils::randomHexString(rng, 3).substr(1);
    }
}

void Obfuscator::collectIdentifiers(const std::vector<Token>& tokens) {
    // First, collect import targets and keyword args to preserve (for runnable Python)
    std::unordered_set<std::string> importPreserved;
    std::unordered_set<std::string> kwPreserved;
    for (size_t i=0;i<tokens.size();++i){
        if(tokens[i].type!=TokenType::Identifier) continue;
        int prev=-1;
        for(int j=(int)i-1;j>=0;--j){
            if(tokens[j].type==TokenType::Whitespace || tokens[j].type==TokenType::Newline || tokens[j].type==TokenType::Comment) continue;
            prev=j; break;
        }
        int next=-1;
        for(size_t j=i+1;j<tokens.size();++j){
            if(tokens[j].type==TokenType::Whitespace || tokens[j].type==TokenType::Comment) continue;
            if(tokens[j].type==TokenType::Newline) break;
            next=(int)j; break;
        }
        bool isImportTarget=false;
        if(prev>=0 && tokens[prev].type==TokenType::Keyword && (tokens[prev].value=="import" || tokens[prev].value=="from")) isImportTarget=true;
        else if(prev>=0 && tokens[prev].type==TokenType::Symbol && tokens[prev].value==","){
            for(int k=prev;k>=0;--k){
                if(tokens[k].type==TokenType::Newline) break;
                if(tokens[k].type==TokenType::Keyword && (tokens[k].value=="import" || tokens[k].value=="from")){ isImportTarget=true; break; }
            }
        }
        if(prev>=0 && tokens[prev].type==TokenType::Symbol && tokens[prev].value=="."){
            if(lang.name=="python" || lang.name=="lua") isImportTarget=true;
        }
        if(isImportTarget) importPreserved.insert(tokens[i].value);

        // Keyword args / assigns: identifier before = or : -> preserve globally for Python to keep calls runnable
        if(next>=0 && tokens[next].type==TokenType::Symbol && (tokens[next].value=="=" || tokens[next].value==":")){
            if(lang.name=="python"){
                kwPreserved.insert(tokens[i].value);
            }
        }
    }
    for(auto &s: importPreserved) preserved.insert(s);
    for(auto &s: kwPreserved) preserved.insert(s);

    std::unordered_set<std::string> seen;
    for (size_t i=0;i<tokens.size();++i){
        auto &t = tokens[i];
        if (t.type != TokenType::Identifier) continue;
        const std::string &name = t.value;
        if (preserved.find(name)!=preserved.end()) continue;
        if (importPreserved.find(name)!=importPreserved.end()) continue;
        // Skip if after dot (attribute) - already preserved via importPreserved check, but double check
        int prev=-1;
        for(int j=(int)i-1;j>=0;--j){
            if(tokens[j].type==TokenType::Whitespace || tokens[j].type==TokenType::Newline || tokens[j].type==TokenType::Comment) continue;
            prev=j; break;
        }
        if(prev>=0 && tokens[prev].type==TokenType::Symbol && tokens[prev].value=="."){
            if(lang.name=="python") continue;
        }
        // Skip if previous is import/from (extra safety)
        if(prev>=0 && tokens[prev].type==TokenType::Keyword && (tokens[prev].value=="import" || tokens[prev].value=="from")) continue;
        // Skip keyword arguments: identifier directly before = or : (e.g., level=, format=, nom: Optional)
        int next=-1;
        for(size_t j=i+1;j<tokens.size();++j){
            if(tokens[j].type==TokenType::Whitespace || tokens[j].type==TokenType::Comment) continue;
            // Newline ends the check
            if(tokens[j].type==TokenType::Newline) break;
            next=(int)j; break;
        }
        if(next>=0 && tokens[next].type==TokenType::Symbol && (tokens[next].value=="=" || tokens[next].value==":")){
            // Distinguish assignment vs keyword arg: if inside parentheses, it's kwarg/type hint; else assignment.
            // For safety, preserve all with =/: to keep API calls working (level=, format=, etc.)
            // For Python, preserve to avoid breaking calls like basicConfig(level=...)
            if(lang.name=="python") continue;
        }

        if (opts.preserveMain && name=="main") continue;
        if (opts.level ==1) {
            bool isConst = true;
            for (char c: name) if (!std::isupper((unsigned char)c) && c!='_' && !std::isdigit((unsigned char)c)) { isConst=false; break; }
            if (isConst && name.size()>1) continue;
        }
        if (seen.find(name)!=seen.end()) continue;
        seen.insert(name);
        std::string obf;
        do {
            obf = generateObfuscatedName();
        } while (preserved.find(obf)!=preserved.end());
        bool alreadyUsed = false;
        for (auto &kv: identMap) if (kv.second==obf) { alreadyUsed=true; break; }
        if (alreadyUsed) obf = obf + std::to_string(rng()%1000);
        identMap[name] = obf;
    }
}

std::string Obfuscator::obfuscateStringContent(const std::string& tokenValue) {
    // tokenValue includes delimiters, e.g. "hello" or 'c' or """..."""
    if (tokenValue.size() < 2) return tokenValue;
    // Detect delimiter
    std::string delim;
    std::string content;
    std::string endDelim;

    // Find which delimiter matches
    for (auto &d : lang.stringDelimiters) {
        if (d.size()<=tokenValue.size() && tokenValue.rfind(d,0)==0 && tokenValue.size()>=d.size()) {
            // check end
            if (tokenValue.size()>=d.size()*2 && tokenValue.substr(tokenValue.size()-d.size())==d) {
                delim = d;
                endDelim = d;
                content = tokenValue.substr(d.size(), tokenValue.size()-d.size()*2);
                break;
            }
        }
    }
    if (delim.empty()) {
        // fallback single char
        delim = tokenValue.substr(0,1);
        endDelim = tokenValue.substr(tokenValue.size()-1,1);
        content = tokenValue.substr(1, tokenValue.size()-2);
    }

    if (content.empty()) return tokenValue;

    // Handle Python f-strings: rename identifiers inside { } but don't encode the string
    if (lang.name=="python") {
        size_t p=0;
        while (p<tokenValue.size() && (tokenValue[p]=='f'||tokenValue[p]=='F'||tokenValue[p]=='r'||tokenValue[p]=='R'||tokenValue[p]=='b'||tokenValue[p]=='B'||tokenValue[p]=='u'||tokenValue[p]=='U')) p++;
        if (p>0 && p<tokenValue.size() && (tokenValue[p]=='"' || tokenValue[p]=='\'')) {
            bool isF=false; for(size_t k=0;k<p;++k) if(tokenValue[k]=='f'||tokenValue[k]=='F') isF=true;
            if(isF){
                // For f-strings, rename variables inside {} using identMap and return
                std::string result = tokenValue;
                // Replace each identifier inside braces
                for(auto &kv: identMap){
                    std::string from = kv.first;
                    std::string to = kv.second;
                    // Find { ... from ... } - simple replace of {from} or {from.} etc.
                    // Use search for {from and from} inside
                    size_t pos=0;
                    while((pos=result.find(from, pos))!=std::string::npos){
                        // Check if inside braces: look for { before and } after
                        size_t open = result.rfind('{', pos);
                        size_t close = result.find('}', pos);
                        if(open!=std::string::npos && close!=std::string::npos && open<pos && pos<close){
                            // Ensure word boundaries
                            bool leftOk = (pos==0 || !std::isalnum((unsigned char)result[pos-1]) && result[pos-1]!='_');
                            bool rightOk = (pos+from.size()>=result.size() || !std::isalnum((unsigned char)result[pos+from.size()]) && result[pos+from.size()]!='_');
                            if(leftOk && rightOk){
                                result.replace(pos, from.size(), to);
                                pos+=to.size();
                                continue;
                            }
                        }
                        pos+=from.size();
                    }
                }
                return result;
            }
        }
    }

    // Custom encoding override: use requested encoding (from --encode)
    if (opts.stringEncoding != "default" && encodings::isValidEncoding(opts.stringEncoding)) {
        // Skip f-strings for custom encoding (already handled above, but double check)
        if (lang.name=="python") {
            size_t p=0;
            while (p<tokenValue.size() && (tokenValue[p]=='f'||tokenValue[p]=='F'||tokenValue[p]=='r'||tokenValue[p]=='R'||tokenValue[p]=='b'||tokenValue[p]=='B'||tokenValue[p]=='u'||tokenValue[p]=='U')) p++;
            if (p>0 && p<tokenValue.size() && (tokenValue[p]=='"' || tokenValue[p]=='\'')) {
                bool isF=false; for(size_t k=0;k<p;++k) if(tokenValue[k]=='f'||tokenValue[k]=='F') isF=true;
                if(isF){
                    // Already handled above, but return with renamed braces
                    std::string result = tokenValue;
                    for(auto &kv: identMap){
                        size_t pos=0;
                        while((pos=result.find(kv.first, pos))!=std::string::npos){
                            size_t open = result.rfind('{', pos);
                            size_t close = result.find('}', pos);
                            if(open!=std::string::npos && close!=std::string::npos && open<pos && pos<close){
                                bool leftOk = (pos==0 || !std::isalnum((unsigned char)result[pos-1]) && result[pos-1]!='_');
                                bool rightOk = (pos+kv.first.size()>=result.size() || !std::isalnum((unsigned char)result[pos+kv.first.size()]) && result[pos+kv.first.size()]!='_');
                                if(leftOk && rightOk){ result.replace(pos, kv.first.size(), kv.second); pos+=kv.second.size(); continue; }
                            }
                            pos+=kv.first.size();
                        }
                    }
                    return result;
                }
            }
        }
        return encodings::encodeString(content, opts.stringEncoding, lang.name, opts.caesarShift, opts.xorKey, opts.vigenereKey);
    }

    // Don't obfuscate very short strings or preprocessor includes?
    if (content.size() < 2 && opts.level < 3) return tokenValue;

    // Choose encoding based on language and level
    std::uniform_int_distribution<int> coin(0,1);

    // For C++/JS/Java/C#: use \x hex escapes
    if (lang.name=="cpp" || lang.name=="javascript" || lang.name=="java" || lang.name=="csharp" || lang.name=="generic") {
        if (opts.level==1) {
            std::string escaped = utils::escapeStringToHex(content);
            return delim + escaped + endDelim;
        } else if (opts.level>=2) {
            // Heavy: split into char codes or hex
            // For demo, use hex escapes; occasionally use concatenation trick for JS
            if (lang.name=="javascript" && opts.level==3 && content.size()>3 && coin(rng)==0) {
                // "hello" -> String.fromCharCode(104,101,...)
                std::stringstream ss;
                ss << "String.fromCharCode(";
                for (size_t i=0;i<content.size();++i){
                    if (i) ss<<",";
                    ss << (int)(unsigned char)content[i];
                }
                ss << ")";
                return ss.str();
            }
            std::string escaped = utils::escapeStringToHex(content);
            return delim + escaped + endDelim;
        }
    }

    if (lang.name=="python") {
        if (opts.level==1) {
            std::string escaped = utils::escapeStringToHex(content);
            return delim + escaped + endDelim;
        } else if (opts.level>=2) {
            // Python: use hex escapes or char construction
            if (opts.level==3 && content.size()>3) {
                // "".join(chr(x) for x in [104,101,...]) or bytes
                std::stringstream ss;
                ss << "bytes([";
                for (size_t i=0;i<content.size();++i){
                    if (i) ss<<",";
                    ss << (int)(unsigned char)content[i];
                }
                ss << "]).decode()";
                return ss.str();
            }
            std::string escaped = utils::escapeStringToHex(content);
            return delim + escaped + endDelim;
        }
    }

    if (lang.name=="lua") {
        std::string escaped = utils::escapeStringToHex(content);
        return delim + escaped + endDelim;
    }

    // fallback
    std::string escaped = utils::escapeStringToHex(content);
    return delim + escaped + endDelim;
}

std::string Obfuscator::obfuscateNumberContent(const std::string& tokenValue) {
    try {
        if (tokenValue.rfind("0x",0)==0 || tokenValue.rfind("0X",0)==0) {
            return tokenValue;
        }
        bool isFloat = tokenValue.find('.')!=std::string::npos || tokenValue.find('e')!=std::string::npos || tokenValue.find('E')!=std::string::npos;
        if (isFloat && opts.level<3) return tokenValue;

        long long val = std::stoll(tokenValue);
        if (val < 0) return tokenValue;
        if (val > 1000000) return tokenValue;

        // Language-specific encoding: Python doesn't allow 0-prefixed octal (use 0o prefix)
        if (lang.name == "python") {
            // For Python, use hex or arithmetic expression only
            std::uniform_int_distribution<int> choice(0, 2);
            int c = choice(rng);
            switch (c) {
                case 0: return utils::toHex((int)val, true);
                case 1: {
                    int r = std::uniform_int_distribution<int>(1, 100)(rng);
                    return "(" + std::to_string((int)val - r) + "+" + std::to_string(r) + ")";
                }
                case 2: {
                    int k = std::uniform_int_distribution<int>(1, 255)(rng);
                    return "((" + std::to_string((int)val ^ k) + "^" + std::to_string(k) + "))";
                }
            }
        }
        return utils::encodeNumber((int)val, rng);
    } catch (...) {
        return tokenValue;
    }
}

std::string Obfuscator::obfuscate(const std::string& code) {
    auto tokens = tokenize(code, lang);

    if (opts.renameIdentifiers) {
        collectIdentifiers(tokens);
    }

    std::vector<Token> out;
    out.reserve(tokens.size());

    for (size_t idx=0; idx<tokens.size(); ++idx) {
        Token t = tokens[idx];

        // Preserve preprocessor lines as-is
        if (t.type==TokenType::Preprocessor && opts.keepPreprocessor) {
            out.push_back(t);
            continue;
        }

        // Comments handling
        if (t.type==TokenType::Comment) {
            if (opts.removeComments) continue;
            out.push_back(t);
            continue;
        }

        // Whitespace / newline handling
        bool isPython = (lang.name == "python");
        bool useMinify = opts.minifyWhitespace && !isPython;
        if (useMinify && (t.type==TokenType::Whitespace || t.type==TokenType::Newline)) {
            if (!out.empty() && out.back().type==TokenType::Preprocessor) {
                out.push_back({TokenType::Newline, "\n", t.line});
            }
            continue;
        }
        // For Python, preserve whitespace/newlines as-is (do not skip)
        if (isPython && (t.type==TokenType::Whitespace || t.type==TokenType::Newline)) {
            out.push_back(t);
            continue;
        }
        // For non-Python with minify disabled, preserve whitespace
        if (!opts.minifyWhitespace && (t.type==TokenType::Whitespace || t.type==TokenType::Newline)) {
            out.push_back(t);
            continue;
        }

        if (t.type==TokenType::Identifier && opts.renameIdentifiers) {
            auto it = identMap.find(t.value);
            if (it != identMap.end()) {
                t.value = it->second;
            }
            out.push_back(t);
            continue;
        }

        if (t.type==TokenType::String && opts.encodeStrings) {
            // Avoid encoding include paths etc? heuristic: if previous token is #include or import, skip
            bool skip = false;
            if (!out.empty() && out.back().type==TokenType::Preprocessor) skip = true;
            // check if previous identifier is "import" or "include"
            if (!skip && !out.empty() && out.back().type==TokenType::Keyword && (out.back().value=="import" || out.back().value=="include")) skip = true;
            if (!skip) {
                // Also skip if string is path like "stdio.h"
                if (t.value.find(".h\"")!=std::string::npos || t.value.find(".hpp\"")!=std::string::npos) skip = true;
            }
            if (!skip) {
                t.value = obfuscateStringContent(t.value);
            }
            out.push_back(t);
            continue;
        }

        if (t.type==TokenType::Number && opts.encodeNumbers) {
            t.value = obfuscateNumberContent(t.value);
            out.push_back(t);
            continue;
        }

        out.push_back(t);
    }

    // Inject dead code if requested and level >=2
    if (opts.injectDeadCode && opts.level >=2) {
        std::vector<Token> withDead;
        for (auto &tok : out) {
            withDead.push_back(tok);
            // After each ';' or '}' inject occasionally
            if (tok.type==TokenType::Symbol && (tok.value==";" || tok.value=="}")) {
                if (rng()%7==0) {
                    // Inject opaque predicate
                    std::string dead;
                    if (lang.name=="python") {
                        dead = "\nif False: "+utils::randomHexString(rng,5)+" = 0\n";
                        withDead.push_back({TokenType::Unknown, dead, 0});
                    } else if (lang.name=="javascript") {
                        withDead.push_back({TokenType::Unknown, "if(false){var "+utils::randomHexString(rng,5)+"=0;}", 0});
                    } else if (lang.name=="cpp" || lang.name=="java" || lang.name=="csharp" || lang.name=="generic") {
                        withDead.push_back({TokenType::Unknown, "if(0){int "+utils::randomHexString(rng,5)+"=0;}", 0});
                    } else if (lang.name=="lua") {
                        withDead.push_back({TokenType::Unknown, "if false then local "+utils::randomHexString(rng,5)+"=0 end", 0});
                    }
                }
            }
        }
        out = withDead;
    }

    // Convert back to string
    std::string result;
    bool isPythonFinal = (lang.name == "python");
    bool shouldMinify = opts.minifyWhitespace && !isPythonFinal;
    if (shouldMinify) {
        // custom to handle Unknown
        std::string rebuilt;
        TokenType prevType = TokenType::Unknown;
        for (auto &t: out) {
            if (t.type==TokenType::Comment && opts.removeComments) continue;
            if (t.type==TokenType::Whitespace || t.type==TokenType::Newline) {
                // skip, but preserve newline after preprocessor
                if (t.type==TokenType::Newline && !rebuilt.empty() && rebuilt.back()!='\n') {
                    // Need to keep newline for preprocessor separation
                    bool needNL = false;
                    // check if last token was preprocessor
                    if (prevType==TokenType::Preprocessor) needNL=true;
                    if (needNL) rebuilt+="\n";
                }
                continue;
            }
            // Need space logic
            if (!rebuilt.empty() && t.type!=TokenType::Unknown) {
                bool prevIsWord = (prevType==TokenType::Identifier || prevType==TokenType::Keyword || prevType==TokenType::Number);
                bool curIsWord = (t.type==TokenType::Identifier || t.type==TokenType::Keyword || t.type==TokenType::Number);
                if (prevIsWord && curIsWord) rebuilt+=" ";
                else {
                    // Also need space after keyword before identifier? covered
                    // check for number followed by identifier? also covered
                }
            } else if (!rebuilt.empty() && t.type==TokenType::Unknown) {
                // dead code snippet may need separator
                rebuilt+=";";
            }
            rebuilt+=t.value;
            prevType=t.type;
        }
        result = rebuilt;
        // Ensure preprocessor lines are newline separated
        // Clean up: ensure no missing ; before dead code already handled
    } else {
        for (auto &t: out) result+=t.value;
    }

    // Optional: add header comment
    if (!opts.removeComments) {
        // keep
    } else {
        // Add obfuscator fingerprint comment optionally removed? For level 3, no header
        if (opts.level < 3) {
            std::string header;
            if (lang.singleLineComment=="//") header = "// Obfuscated with Universal Obfuscator seed="+std::to_string(opts.seed)+"\n";
            else if (lang.singleLineComment=="#") header = "# Obfuscated seed="+std::to_string(opts.seed)+"\n";
            else if (lang.singleLineComment=="--") header = "-- Obfuscated seed="+std::to_string(opts.seed)+"\n";
            else header = "/* Obfuscated seed="+std::to_string(opts.seed)+" */\n";
            // Only add if not minify heavy?
            if (opts.level==1) result = header + result;
        }
    }

    return result;
}
