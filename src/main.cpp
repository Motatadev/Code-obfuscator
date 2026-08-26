#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include "obfuscator.hpp"
#include "language.hpp"
#include "encodings.hpp"

namespace fs = std::filesystem;

void printUsage(const char* prog) {
    std::cout << "Universal Code Obfuscator - v1.0\n";
    std::cout << "Obfuscate any source code in any language with 30+ encodings.\n";
    std::cout << "\nUsage:\n";
    std::cout << "  " << prog << " [options]\n";
    std::cout << "\nFile selection:\n";
    std::cout << "  -i, --input <file|dir>   Input file or directory (default: interactive)\n";
    std::cout << "  -o, --output <file>      Output file (default: stdout or <input>.obf.<ext>)\n";
    std::cout << "  --interactive            Launch interactive file & encoding picker\n";
    std::cout << "\nLanguage:\n";
    std::cout << "  -l, --lang <lang>        Language: cpp, python, js, java, csharp, lua, generic, auto (default: auto)\n";
    std::cout << "\nObfuscation:\n";
    std::cout << "  --level <1-3>            Obfuscation strength 1=light 2=medium 3=heavy (default: 2)\n";
    std::cout << "  --seed <n>               Seed for reproducibility (default: random)\n";
    std::cout << "  --no-rename              Disable identifier renaming\n";
    std::cout << "  --no-strings             Disable string encoding\n";
    std::cout << "  --no-numbers             Disable number encoding\n";
    std::cout << "  --keep-comments          Keep comments\n";
    std::cout << "  --no-minify              Keep original whitespace\n";
    std::cout << "  --dead-code              Inject dead code\n";
    std::cout << "\nEncodings (string level):\n";
    std::cout << "  -e, --encode <name>      String encoding: see --list-encodings\n";
    std::cout << "      --file-encoding <name> File output encoding (utf-8, utf-16, base64, etc., default: utf-8)\n";
    std::cout << "      --xor-key <key>      Key for XOR/AES/ChaCha (default: K)\n";
    std::cout << "      --vigenere-key <key> Key for Vigenere/Beaufort/Autokey/Columnar (default: KEY)\n";
    std::cout << "      --caesar-shift <n>   Shift for Caesar (default: 3)\n";
    std::cout << "\nInfo:\n";
    std::cout << "  --list-langs             List supported languages\n";
    std::cout << "  --list-encodings         List all supported encodings\n";
    std::cout << "  -h, --help               Show this help\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << prog << " -i input.cpp -o output.cpp --level 3 --encode base64\n";
    std::cout << "  " << prog << " -i script.py --encode rot13 --seed 42\n";
    std::cout << "  " << prog << " -i app.js --encode aes --xor-key mysecret --file-encoding utf-8\n";
    std::cout << "  " << prog << " --interactive\n";
    std::cout << "  cat code.cpp | " << prog << " --lang cpp --encode hex-escape > obf.cpp\n";
    std::cout << "\nGitHub: https://github.com/Motatadev/Code-obfuscator\n";
}

void printEncodings() {
    auto encs = encodings::listEncodings();
    std::cout << "Supported encodings (" << encs.size() << "):\n";
    std::cout << "\n[Base encodings]\n  base2, base8, base10, base16/hex, base32, base36, base45, base58, base62, base64, base64url, base85/ascii85, base91, base92, base100, base122, base32768\n";
    std::cout << "  binary, octal, hex-escape, unicode-escape\n";
    std::cout << "\n[URL/HTML]\n  url/percent, html, xml, json, quoted-printable, punycode\n";
    std::cout << "\n[ROT/Substitution]\n  rot13, rot1-rot25, rot47, rot8000, caesar, atbash, affine, vigenere, beaufort, autokey, rail-fence, columnar\n";
    std::cout << "\n[Obfuscation]\n  string-escaping, string-splitting, string-concatenation, char-substitution, homoglyph, whitespace\n";
    std::cout << "\n[Crypto]\n  xor, aes, chacha20, rsa, des, 3des, blowfish, twofish\n";
    std::cout << "\n[Compression+Encoding]\n  gzip+base64, zlib+base64, deflate+base64, brotli+base64, lzma+base64, bzip2+base64\n";
    std::cout << "\n[File encodings]\n  ascii, utf-8, utf-16, utf-32, unicode\n";
    std::cout << "\nUsage: --encode <name>  (e.g. --encode base64, --encode rot13, --encode aes)\n";
    std::cout << "       --file-encoding <name>  (e.g. --file-encoding utf-16, --file-encoding base64)\n";
}

std::string readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open file: " + path);
    std::stringstream ss; ss << in.rdbuf(); return ss.str();
}
void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("Failed to write file: " + path);
    out << content;
}
std::string readStdin(){ std::stringstream ss; ss<<std::cin.rdbuf(); return ss.str(); }

std::vector<std::string> listFilesRecursive(const std::string& dir) {
    std::vector<std::string> files;
    try {
        for (auto &p: fs::recursive_directory_iterator(dir)) {
            if (p.is_regular_file()) files.push_back(p.path().string());
        }
    } catch(...) {}
    return files;
}

void interactivePicker(std::string &inputPath, std::string &outputPath, ObfuscatorOptions &opts, std::string &langStr) {
    std::cout << "\n=== Interactive Mode ===\n";
    std::cout << "Current directory: " << fs::current_path().string() << "\n\n";

    // List files
    std::vector<std::string> files;
    for (auto &p: fs::directory_iterator(".")) {
        if (p.is_regular_file()) files.push_back(p.path().string());
    }
    std::sort(files.begin(), files.end());
    if (files.empty()) {
        std::cout << "No files in current directory.\n";
        return;
    }
    std::cout << "Select input file:\n";
    for (size_t i=0;i<files.size() && i<20; ++i) std::cout << "  ["<<i<<"] "<<files[i]<<"\n";
    std::cout << "  [c] Custom path\n  Enter number or path: ";
    std::string choice; std::getline(std::cin, choice);
    if (choice=="c"||choice=="C") {
        std::cout << "Enter file path: "; std::getline(std::cin, inputPath);
    } else {
        try { int idx=std::stoi(choice); if(idx>=0 && idx<(int)files.size()) inputPath=files[idx]; else inputPath=choice; } catch(...) { inputPath=choice; }
    }
    if (inputPath.empty()) inputPath=files[0];
    std::cout << "Selected: " << inputPath << "\n\n";

    // Language
    std::cout << "Language [auto/cpp/python/js/java/csharp/lua/generic] (default auto): ";
    std::getline(std::cin, langStr);
    if(langStr.empty()) langStr="auto";

    // Encoding
    std::cout << "\nAvailable string encodings (enter --list-encodings for full list):\n";
    std::cout << "  base2, base16/hex, base32, base64, base64url, base85, hex-escape, unicode-escape,\n";
    std::cout << "  url, html, rot13, rot47, caesar, atbash, vigenere, xor, aes, chacha20, gzip+base64, etc.\n";
    std::cout << "Enter string encoding [default]: ";
    std::string enc; std::getline(std::cin, enc);
    if(!enc.empty()) opts.stringEncoding = enc; else opts.stringEncoding="default";

    if(opts.stringEncoding=="caesar"||opts.stringEncoding=="xor"||opts.stringEncoding=="aes"||opts.stringEncoding=="vigenere"){
        std::cout << "Enter key/shift for "<<opts.stringEncoding<<" (xor-key/vigenere-key/caesar-shift): ";
        std::string key; std::getline(std::cin, key);
        if(!key.empty()){
            if(opts.stringEncoding=="caesar") opts.caesarShift = std::stoi(key);
            else if(opts.stringEncoding=="xor"||opts.stringEncoding=="aes"||opts.stringEncoding=="chacha20") opts.xorKey=key;
            else opts.vigenereKey=key;
        }
    }

    std::cout << "File output encoding [utf-8]: ";
    std::string fenc; std::getline(std::cin, fenc);
    if(!fenc.empty()) opts.fileEncoding=fenc;

    std::cout << "Level 1-3 [2]: ";
    std::string lvl; std::getline(std::cin, lvl);
    if(!lvl.empty()) opts.level=std::stoi(lvl);

    std::cout << "Output file [auto: <input>.obf.<ext>]: ";
    std::getline(std::cin, outputPath);
    if(outputPath.empty()){
        // auto
        fs::path p(inputPath);
        outputPath = p.stem().string() + ".obf" + p.extension().string();
        if(outputPath==".obf") outputPath = inputPath + ".obf";
    }
    std::cout << "\n[Interactive] Will obfuscate "<<inputPath<<" -> "<<outputPath<<" | lang="<<langStr<<" | encode="<<opts.stringEncoding<<" | fileEnc="<<opts.fileEncoding<<" | level="<<opts.level<<"\n";
}

int main(int argc, char* argv[]) {
    std::string inputPath;
    std::string outputPath;
    std::string langStr = "auto";
    ObfuscatorOptions opts;
    bool interactive=false;

    for (int i=1;i<argc;++i){
        std::string arg=argv[i];
        if(arg=="-h"||arg=="--help"){ printUsage(argv[0]); return 0; }
        else if(arg=="--list-langs"){
            auto langs=getAllLanguages(); std::cout<<"Supported languages:\n"; for(auto &l:langs){ std::cout<<"  "<<l.name<<" ("; for(size_t j=0;j<l.extensions.size();++j){ if(j) std::cout<<", "; std::cout<<l.extensions[j]; } std::cout<<")\n"; } return 0;
        }
        else if(arg=="--list-encodings"){ printEncodings(); return 0; }
        else if(arg=="--interactive"){ interactive=true; }
        else if(arg=="-i"||arg=="--input"){ if(i+1>=argc){ std::cerr<<"--input requires a file\n"; return 1;} inputPath=argv[++i]; }
        else if(arg=="-o"||arg=="--output"){ if(i+1>=argc){ std::cerr<<"--output requires a file\n"; return 1;} outputPath=argv[++i]; }
        else if(arg=="-l"||arg=="--lang"){ if(i+1>=argc){ std::cerr<<"--lang requires a value\n"; return 1;} langStr=argv[++i]; }
        else if(arg=="--level"){ if(i+1>=argc){ std::cerr<<"--level requires 1-3\n"; return 1;} opts.level=std::stoi(argv[++i]); if(opts.level<1||opts.level>3){ std::cerr<<"level must be 1-3\n"; return 1; } }
        else if(arg=="--seed"){ if(i+1>=argc){ std::cerr<<"--seed requires a value\n"; return 1;} opts.seed=(unsigned int)std::stoul(argv[++i]); }
        else if(arg=="-e"||arg=="--encode"||arg=="--string-encoding"){ if(i+1>=argc){ std::cerr<<"--encode requires a value\n"; return 1;} opts.stringEncoding=argv[++i]; if(!encodings::isValidEncoding(opts.stringEncoding)){ std::cerr<<"[Warning] Unknown encoding: "<<opts.stringEncoding<<" (use --list-encodings)\n"; } }
        else if(arg=="--file-encoding"){ if(i+1>=argc){ std::cerr<<"--file-encoding requires a value\n"; return 1;} opts.fileEncoding=argv[++i]; }
        else if(arg=="--xor-key"){ if(i+1>=argc){ std::cerr<<"--xor-key requires a value\n"; return 1;} opts.xorKey=argv[++i]; }
        else if(arg=="--vigenere-key"){ if(i+1>=argc){ std::cerr<<"--vigenere-key requires a value\n"; return 1;} opts.vigenereKey=argv[++i]; }
        else if(arg=="--caesar-shift"){ if(i+1>=argc){ std::cerr<<"--caesar-shift requires a value\n"; return 1;} opts.caesarShift=std::stoi(argv[++i]); }
        else if(arg=="--no-rename") opts.renameIdentifiers=false;
        else if(arg=="--no-strings") opts.encodeStrings=false;
        else if(arg=="--no-numbers") opts.encodeNumbers=false;
        else if(arg=="--keep-comments") opts.removeComments=false;
        else if(arg=="--no-minify") opts.minifyWhitespace=false;
        else if(arg=="--dead-code") opts.injectDeadCode=true;
        else if(arg.rfind("-",0)==0){ std::cerr<<"Unknown option: "<<arg<<"\n"; printUsage(argv[0]); return 1; }
        else { if(inputPath.empty()) inputPath=arg; else if(outputPath.empty()) outputPath=arg; else { std::cerr<<"Unexpected argument: "<<arg<<"\n"; return 1; } }
    }

    // Trigger interactive if no input and no pipe
    if(interactive || (argc==1)){
        // if stdin is tty and no args, go interactive
        interactivePicker(inputPath, outputPath, opts, langStr);
        if(inputPath.empty()){ std::cerr<<"[Error] No input file selected.\n"; return 1; }
    }

    try{
        std::string code;
        bool isDir=false;
        if(!inputPath.empty() && fs::exists(inputPath) && fs::is_directory(inputPath)){
            isDir=true;
        }
        if(isDir){
            std::cout<<"[Info] Input is directory: "<<inputPath<<"\n";
            auto files=listFilesRecursive(inputPath);
            std::cout<<"[Info] Found "<<files.size()<<" files. Obfuscating each...\n";
            for(auto &f: files){
                try{
                    std::string c=readFile(f);
                    LanguageProfile lang = (langStr=="auto")? detectLanguage(f) : getLanguageProfile(langStr);
                    Obfuscator obf(lang, opts);
                    std::string res=obf.obfuscate(c);
                    if(opts.fileEncoding!="utf-8" && opts.fileEncoding!="utf8") res=encodings::encodeFileContent(res, opts.fileEncoding);
                    fs::path outPath = fs::path(f).string() + ".obf" + fs::path(f).extension().string();
                    if(!outputPath.empty() && fs::is_directory(outputPath)){
                        outPath = fs::path(outputPath) / (fs::path(f).filename().string() + ".obf");
                    }
                    fs::create_directories(fs::path(outPath).parent_path());
                    writeFile(outPath.string(), res);
                    std::cout<<"  [OK] "<<f<<" -> "<<outPath.string()<<" ("<<c.size()<<"->"<<res.size()<<")\n";
                } catch(std::exception &e){ std::cerr<<"  [Skip] "<<f<<": "<<e.what()<<"\n"; }
            }
            return 0;
        }

        if(inputPath.empty()){
            if(std::cin.peek()==std::char_traits<char>::eof()){
                std::cout<<"[Obfuscator] No input. Use -h for help or --interactive.\n"; printUsage(argv[0]); return 1;
            } else code=readStdin();
        } else {
            code=readFile(inputPath);
        }
        if(code.empty()){ std::cerr<<"[Error] Empty or not found file.\n"; return 1; }

        LanguageProfile lang;
        if(langStr=="auto"){
            if(!inputPath.empty()) lang=detectLanguage(inputPath); else lang=getLanguageProfile("cpp");
            std::cout<<"[Info] Detected language: "<<lang.name; if(!inputPath.empty()) std::cout<<" ("<<inputPath<<")"; std::cout<<"\n";
        } else { lang=getLanguageProfile(langStr); std::cout<<"[Info] Language forced: "<<lang.name<<"\n"; }

        std::cout<<"[Info] Level: "<<opts.level<<" | Seed: "<<(opts.seed? std::to_string(opts.seed):"random")<<"\n";
        std::cout<<"[Info] String encoding: "<<opts.stringEncoding<<" | File encoding: "<<opts.fileEncoding<<"\n";
        if(opts.stringEncoding=="xor"||opts.stringEncoding=="aes"||opts.stringEncoding=="chacha20") std::cout<<"[Info] XOR/AES key: "<<opts.xorKey<<"\n";
        if(opts.stringEncoding=="vigenere"||opts.stringEncoding=="beaufort"||opts.stringEncoding=="autokey"||opts.stringEncoding=="columnar") std::cout<<"[Info] Vigenere key: "<<opts.vigenereKey<<"\n";
        if(opts.stringEncoding=="caesar") std::cout<<"[Info] Caesar shift: "<<opts.caesarShift<<"\n";
        std::cout<<"[Info] Options: rename="<<opts.renameIdentifiers<<" strings="<<opts.encodeStrings<<" numbers="<<opts.encodeNumbers<<" minify="<<opts.minifyWhitespace<<" deadcode="<<opts.injectDeadCode<<"\n";

        Obfuscator obf(lang, opts);
        std::string result=obf.obfuscate(code);
        // Apply file-level encoding
        if(opts.fileEncoding!="utf-8" && opts.fileEncoding!="utf8" && opts.fileEncoding!="ascii" && opts.fileEncoding!="unicode"){
            result=encodings::encodeFileContent(result, opts.fileEncoding);
            std::cout<<"[Info] File encoding applied: "<<opts.fileEncoding<<"\n";
        }

        auto map=obf.getIdentifierMap();
        std::cout<<"[Info] "<<map.size()<<" identifiers renamed.\n";
        if(map.size()>0 && map.size()<=20){
            std::cout<<"[Mapping]\n"; for(auto &kv:map) std::cout<<"  "<<kv.first<<" -> "<<kv.second<<"\n";
        }

        if(outputPath.empty()){
            std::cout<<"\n--- OBFUSCATED CODE ---\n"<<result<<"\n";
        } else {
            writeFile(outputPath, result);
            std::cout<<"[Success] Obfuscated file written: "<<outputPath<<" ("<<result.size()<<" bytes, original "<<code.size()<<" bytes)\n";
        }
        double ratio=100.0*result.size()/(code.size()?code.size():1);
        std::cout<<"[Stats] Size: "<<code.size()<<" -> "<<result.size()<<" ("<<(int)ratio<<"%)\n";
    } catch(std::exception &e){ std::cerr<<"[Error] "<<e.what()<<"\n"; return 1; }
    return 0;
}
