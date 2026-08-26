#include "language.hpp"
#include <algorithm>
#include <cctype>

static LanguageProfile makeCppProfile() {
    LanguageProfile p;
    p.name = "cpp";
    p.extensions = {".cpp",".cc",".cxx",".h",".hpp",".c"};
    p.singleLineComment = "//";
    p.multiLineCommentStart = "/*";
    p.multiLineCommentEnd = "*/";
    p.stringDelimiters = {"\"", "'"};
    p.charDelimiter = '\'';
    p.hasPreprocessor = true;
    p.keywords = {
        "alignas","alignof","and","and_eq","asm","auto","bitand","bitor","bool","break","case","catch","char","char8_t","char16_t","char32_t","class","compl","concept","const","consteval","constexpr","constinit","const_cast","continue","co_await","co_return","co_yield","decltype","default","delete","do","double","dynamic_cast","else","enum","explicit","export","extern","false","float","for","friend","goto","if","inline","int","long","mutable","namespace","new","noexcept","not","not_eq","nullptr","operator","or","or_eq","private","protected","public","register","reinterpret_cast","requires","return","short","signed","sizeof","static","static_assert","static_cast","struct","switch","template","this","thread_local","throw","true","try","typedef","typeid","typename","union","unsigned","using","virtual","void","volatile","wchar_t","while","xor","xor_eq",
        "include","define","ifdef","ifndef","endif","pragma","import"
    };
    p.preservedIdentifiers = {"main","std","cout","cin","endl","string","vector","map","include","define"};
    return p;
}

static LanguageProfile makePythonProfile() {
    LanguageProfile p;
    p.name = "python";
    p.extensions = {".py",".pyw"};
    p.singleLineComment = "#";
    p.multiLineCommentStart = "\"\"\"";
    p.multiLineCommentEnd = "\"\"\"";
    p.stringDelimiters = {"\"", "'", "\"\"\"", "'''", "\"", "'"};
    // For simplicity, use " and '
    p.stringDelimiters = {"\"", "'", "\"\"\"", "'''"}; 
    p.keywords = {
        "False","None","True","and","as","assert","async","await","break","class","continue","def","del","elif","else","except","finally","for","from","global","if","import","in","is","lambda","nonlocal","not","or","pass","raise","return","try","while","with","yield","print","self","__init__","__name__","__main__"
    };
    return p;
}

static LanguageProfile makeJsProfile() {
    LanguageProfile p;
    p.name = "javascript";
    p.extensions = {".js",".mjs",".cjs",".ts"};
    p.singleLineComment = "//";
    p.multiLineCommentStart = "/*";
    p.multiLineCommentEnd = "*/";
    p.stringDelimiters = {"\"", "'", "`"};
    p.keywords = {
        "break","case","catch","class","const","continue","debugger","default","delete","do","else","export","extends","false","finally","for","function","if","import","in","instanceof","new","null","return","super","switch","this","throw","true","try","typeof","var","void","while","with","yield","let","static","enum","await","async","from","as","get","set","of","require","console","log","module","exports"
    };
    return p;
}

static LanguageProfile makeJavaProfile() {
    LanguageProfile p;
    p.name = "java";
    p.extensions = {".java"};
    p.singleLineComment = "//";
    p.multiLineCommentStart = "/*";
    p.multiLineCommentEnd = "*/";
    p.stringDelimiters = {"\"", "'"};
    p.keywords = {
        "abstract","assert","boolean","break","byte","case","catch","char","class","const","continue","default","do","double","else","enum","extends","final","finally","float","for","goto","if","implements","import","instanceof","int","interface","long","native","new","package","private","protected","public","return","short","static","strictfp","super","switch","synchronized","this","throw","throws","transient","try","void","volatile","while","true","false","null","var"
    };
    return p;
}

static LanguageProfile makeCSharpProfile() {
    LanguageProfile p;
    p.name = "csharp";
    p.extensions = {".cs"};
    p.singleLineComment = "//";
    p.multiLineCommentStart = "/*";
    p.multiLineCommentEnd = "*/";
    p.stringDelimiters = {"\"", "'", "@\""};
    p.keywords = {
        "abstract","as","base","bool","break","byte","case","catch","char","checked","class","const","continue","decimal","default","delegate","do","double","else","enum","event","explicit","extern","false","finally","fixed","float","for","foreach","goto","if","implicit","in","int","interface","internal","is","lock","long","namespace","new","null","object","operator","out","override","params","private","protected","public","readonly","ref","return","sbyte","sealed","short","sizeof","stackalloc","static","string","struct","switch","this","throw","true","try","typeof","uint","ulong","unchecked","unsafe","ushort","using","virtual","void","volatile","while","var","async","await"
    };
    return p;
}

static LanguageProfile makeLuaProfile() {
    LanguageProfile p;
    p.name = "lua";
    p.extensions = {".lua"};
    p.singleLineComment = "--";
    p.multiLineCommentStart = "--[[";
    p.multiLineCommentEnd = "]]";
    p.stringDelimiters = {"\"", "'", "[["};
    p.keywords = {"and","break","do","else","elseif","end","false","for","function","if","in","local","nil","not","or","repeat","return","then","true","until","while","print","require","self"};
    return p;
}

static LanguageProfile makeGenericProfile() {
    LanguageProfile p;
    p.name = "generic";
    p.extensions = {".txt",""};
    p.singleLineComment = "//";
    p.multiLineCommentStart = "/*";
    p.multiLineCommentEnd = "*/";
    p.stringDelimiters = {"\"", "'"};
    p.keywords = {};
    return p;
}

LanguageProfile getLanguageProfile(const std::string& langName) {
    std::string lower = langName;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower=="cpp"||lower=="c"||lower=="c++"||lower=="cc") return makeCppProfile();
    if (lower=="python"||lower=="py") return makePythonProfile();
    if (lower=="js"||lower=="javascript"||lower=="ts"||lower=="typescript") return makeJsProfile();
    if (lower=="java") return makeJavaProfile();
    if (lower=="csharp"||lower=="cs"||lower=="c#") return makeCSharpProfile();
    if (lower=="lua") return makeLuaProfile();
    if (lower=="generic"||lower=="auto") return makeGenericProfile();
    return makeGenericProfile();
}

LanguageProfile detectLanguage(const std::string& filename) {
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    auto all = getAllLanguages();
    for (auto &p : all) {
        for (auto &ext : p.extensions) {
            if (!ext.empty() && lower.size() >= ext.size() && lower.substr(lower.size()-ext.size())==ext) {
                return p;
            }
        }
    }
    return makeGenericProfile();
}

std::vector<LanguageProfile> getAllLanguages() {
    return {makeCppProfile(), makePythonProfile(), makeJsProfile(), makeJavaProfile(), makeCSharpProfile(), makeLuaProfile(), makeGenericProfile()};
}
