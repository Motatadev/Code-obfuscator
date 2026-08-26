# Code Obfuscator — Universal Code Obfuscator

> Obfuscate any source code in any programming language with a single fast C++ tool.

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Encodings](https://img.shields.io/badge/encodings-50%2B-orange.svg)]()

A **language-agnostic** obfuscator written in C++17. It supports **C/C++, Python, JavaScript/TypeScript, Java, C#, Lua** and any generic language via automatic detection. Choose **any file** and **any encoding** among 50+ options — from Base64 to AES — with an interactive picker.

GitHub: **https://github.com/Motatadev/Code-obfuscator**

---

## ✨ Features

- 🌐 **Universal** — One binary handles many languages (auto-detection by extension or `--lang`)
- 📁 **File picker** — `--interactive` mode: choose any file/directory + encoding from a menu
- 🔤 **50+ Encodings** — Bases, URL/HTML, Ciphers, Crypto, Compression — see tables below
- 🔀 **Identifier renaming** — `calculateSum` → `_0x7fa3b1` / `_Il1O0lI`
- 🔤 **String encoding** — per-encoding wrapper (`base64.b64decode`, `atob`, `bytes().decode`, etc.)
- 🔢 **Number obfuscation** — `42` → `0x2a` / `(41+1)` / `((7^45))`
- 🧹 **Minification** — Remove whitespace & comments (preserves Python indentation)
- 💀 **Dead-code injection** — Opaque predicates (`if(false){...}`, `if False: ...`)
- 🎲 **Deterministic** — `--seed` for reproducible builds
- ⚡ **Fast & offline** — Pure C++17, no dependencies, no telemetry

---

## 📋 Supported Languages

| Language   | Extensions              | `--lang` value          | Notes |
|------------|-------------------------|-------------------------|-------|
| C / C++    | `.c .cpp .cc .h .hpp`  | `cpp`, `c`, `c++`       | Preserves `#include` |
| Python     | `.py .pyw`             | `python`, `py`          | Keeps indentation |
| JavaScript | `.js .mjs .cjs .ts`    | `js`, `javascript`, `ts`| Handles `` ` `` |
| Java       | `.java`                | `java`                  | - |
| C#         | `.cs`                  | `csharp`, `cs`, `c#`    | - |
| Lua        | `.lua`                 | `lua`                   | Handles `--[[ ]]` |
| Generic    | `.txt` / any           | `generic`               | Fallback lexer |

Auto-detection is used when `--lang auto` (default) and an input file is provided.

---

## 🔤 Encodings — Choose Your Transform

Use `-e, --encode <name>` for string-level encoding and `--file-encoding <name>` for whole-file output encoding.

### 🔢 Base Encodings

| Encoding | Aliases | Example (`"Hello"`) |
|----------|---------|---------------------|
| Base2 - Binary | `base2`, `binary` | `01001000 01100101 ...` |
| Base8 - Octal | `base8`, `octal` | `110 145 154 ...` |
| Base10 | `base10` | `72 101 108 ...` |
| Base16 - Hex | `base16`, `hex`, `b16` | `48656c6c6f` |
| Base32 | `base32`, `b32` | `JBSWY3DP...` |
| Base36 | `base36`, `b36` | `5PZ4I...` |
| Base45 | `base45`, `b45` | `...` RFC9285 |
| Base58 | `base58`, `b58` | Bitcoin alphabet |
| Base62 | `base62`, `b62` | `0-9A-Za-z` |
| Base64 | `base64`, `b64` | `SGVsbG8=` |
| Base64URL | `base64url` | `SGVsbG8` (url-safe) |
| Base85 / ASCII85 | `base85`, `ascii85`, `b85` | Adobe ASCII85 |
| Base91 | `base91`, `b91` | `...` |
| Base92 | `base92`, `b92` | `...` |
| Base100 | `base100` | Emoji-based |
| Base122 | `base122` | Printable 122 |
| Base32768 | `base32768` | CJK 15-bit |
| URL / Percent | `url`, `percent` | `%48%65%6c...` |
| HTML Entity | `html` | `&lt; &amp;` |
| Unicode Escape | `unicode-escape` | `\u0048\u0065...` |
| Hex Escape | `hex-escape` | `\x48\x65...` |
| Octal Escape | `octal` | `\110\145...` |

### 🔄 ROT / Substitution

| Encoding | Description |
|----------|-------------|
| `rot13` | ROT13 |
| `rot1` → `rot25` | Any ROT N (e.g. `--encode rot5`) |
| `rot47` | ROT47 (ASCII 33-126) |
| `rot8000` | ROT8000 (stub) |
| `caesar` | Caesar with `--caesar-shift N` (default 3) |
| `atbash` | Atbash (A↔Z) |
| `affine` | Affine `a=5,b=8` |
| `vigenere` | Vigenère with `--vigenere-key KEY` |
| `beaufort` | Beaufort |
| `autokey` | Autokey |
| `rail-fence` | Rail Fence (3 rails) |
| `columnar` | Columnar Transposition with key |

### 🧩 Text/Code Obfuscation

| Encoding | Effect |
|----------|--------|
| `string-escaping` | Hex escapes |
| `string-splitting` | `"He"+"llo"` |
| `string-concatenation` | Same as splitting |
| `char-substitution` | `a→@, e→3, i→1, o→0` |
| `homoglyph` | `a→а (Cyrillic)` |
| `whitespace` | Insert invisible whitespace |
| Plus built-in: dead-code, variable renaming, control-flow | Via `--dead-code` + `--level` |

### 🔐 Cryptographic (demo, reversible, labeled)

| Encoding | Notes |
|----------|-------|
| `xor` | Hex XOR with `--xor-key K` |
| `aes` | AES-like (XOR+Base64, labeled) |
| `chacha20` | ChaCha20-like |
| `rsa` | RSA-like (Base64 labeled) |
| `des` / `3des` | DES / TripleDES-like |
| `blowfish` | Blowfish-like |
| `twofish` | Twofish-like |

> Real crypto would need key management; these are **demo encodings** that show the transform and keep the output reversible for inspection.

### 🗜️ Compression + Encoding

| Encoding | Description |
|----------|-------------|
| `gzip+base64` | gzip + Base64 (stub) |
| `zlib+base64` | zlib + Base64 |
| `deflate+base64` | DEFLATE + Base64 |
| `brotli+base64` | Brotli + Base64 |
| `lzma+base64` | LZMA + Base64 |
| `bzip2+base64` | BZip2 + Base64 |

### File Encodings

`--file-encoding` controls the **output file bytes**: `ascii`, `utf-8`, `utf-16` (with BOM), `utf-32`, `base64`, `base32`, `base58`, `url`, `binary`, `gzip+base64`, etc. The whole file is transformed.

---

## 🚀 Quick Start

### 1. Build

```powershell
git clone https://github.com/Motatadev/Code-obfuscator.git
cd Code-obfuscator
.\build.bat
.\build\obfuscator.exe --help
```

### 2. Obfuscate with encoding choice

```powershell
# Choose file and encoding interactively
.\build\obfuscator.exe --interactive

# Direct: pick file + encoding
.\build\obfuscator.exe -i examples\example.cpp -o out.cpp --encode base64
.\build\obfuscator.exe -i examples\example.py -o out.py --encode rot13 --seed 42
.\build\obfuscator.exe -i examples\example.js -o out.js --encode aes --xor-key mysecret
.\build\obfuscator.exe -i examples\example.cpp -o out.cpp --encode hex-escape --level 3
.\build\obfuscator.exe -i examples\example.py -o out.py --encode vigenere --vigenere-key SECRET

# File-level encoding: whole file as Base64 or UTF-16
.\build\obfuscator.exe -i input.cpp -o out.b64 --file-encoding base64
.\build\obfuscator.exe -i input.cpp -o out.utf16 --file-encoding utf-16

# Directory (batch)
.\build\obfuscator.exe -i src/ -o build/obf/ --encode base64
```

### 3. List options

```powershell
.\build\obfuscator.exe --list-langs
.\build\obfuscator.exe --list-encodings
```

---

## 📦 Installation

### Option A — Visual Studio (Windows)

Requires **Visual Studio 2022** with C++ Desktop Development.

```powershell
git clone https://github.com/Motatadev/Code-obfuscator.git
cd Code-obfuscator
.\build.bat
.\build\obfuscator.exe --help
```

### Option B — CMake (Windows/Linux/macOS)

```bash
git clone https://github.com/Motatadev/Code-obfuscator.git
cd Code-obfuscator
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/obfuscator --help   # or ./build/Release/obfuscator.exe on Windows
```

### Option C — g++ / clang++

```bash
g++ -std=c++17 -O2 -o obfuscator src/main.cpp src/obfuscator.cpp src/language.cpp src/lexer.cpp src/encodings.cpp
clang++ -std=c++17 -O2 -o obfuscator src/main.cpp src/obfuscator.cpp src/language.cpp src/lexer.cpp src/encodings.cpp
```

### Verify

```bash
obfuscator --list-encodings
obfuscator -i examples/example.cpp -o /tmp/out.cpp --encode base64 --seed 123
obfuscator -i examples/example.py -o /tmp/out.py --encode morse
```

---

## 📖 Usage Tutorial

### Basic CLI

```
obfuscator [options]

  -i, --input <file|dir>   Input file or directory (default: interactive)
  -o, --output <file>      Output file (default: stdout or <input>.obf.<ext>)
  --interactive            Launch interactive picker
  -l, --lang <lang>        Force language (default: auto)
  --level <1-3>            Strength
  --seed <n>               Seed
  -e, --encode <name>      String encoding (see --list-encodings)
      --file-encoding <name> File encoding
      --xor-key <key>      Key for XOR/AES/ChaCha
      --vigenere-key <key> Key for Vigenere etc.
      --caesar-shift <n>   Caesar shift
  --no-rename / --no-strings / --no-numbers / --keep-comments / --no-minify / --dead-code
  --list-langs / --list-encodings / -h, --help
```

### Interactive Picker

```powershell
.\build\obfuscator.exe --interactive
# or just
.\build\obfuscator.exe
```

You will be prompted:

```
Select input file:
  [0] .\examples\example.cpp
  [1] .\examples\example.py
  [2] .\examples\example.js
  Enter number or path: 0
Language [auto/cpp/python/js/...] (default auto): auto
Enter string encoding [default]: base64
File output encoding [utf-8]: utf-8
Level 1-3 [2]: 2
Output file [auto: <input>.obf.<ext>]:
```

### Encoding Examples

```bash
# ROT
obfuscator -i code.py -o out.py --encode rot13
obfuscator -i code.py -o out.py --encode rot5      # any 1-25
obfuscator -i code.js -o out.js --encode caesar --caesar-shift 7

# Ciphers
obfuscator -i code.cpp -o out.cpp --encode vigenere --vigenere-key LEMON
obfuscator -i code.cpp -o out.cpp --encode atbash
obfuscator -i code.cpp -o out.cpp --encode rail-fence

# Bases
obfuscator -i code.py -o out.py --encode base32
obfuscator -i code.py -o out.py --encode base58
obfuscator -i code.py -o out.py --encode base91

# Obfuscation tricks
obfuscator -i code.js -o out.js --encode string-splitting
obfuscator -i code.js -o out.js --encode homoglyph
obfuscator -i code.js -o out.js --encode char-substitution

# Crypto
obfuscator -i secret.py -o out.py --encode aes --xor-key SuperSecret
obfuscator -i secret.py -o out.py --encode xor --xor-key K

# Compression
obfuscator -i big.cpp -o out.b64 --encode gzip+base64
obfuscator -i big.cpp -o out.b64 --file-encoding gzip+base64
```

### Pipe & stdin

```bash
cat input.cpp | obfuscator --lang cpp --encode hex-escape > output.cpp
echo 'print("hi")' | obfuscator --lang python --encode rot13
```

---

## 🔧 How It Works

1. **Language detection** — `language.cpp` maps extensions to profiles (keywords, comments, delimiters). See `src/language.hpp:10`.
2. **Lexing** — `lexer.cpp:tokenize()` produces `Identifier / Keyword / String / Comment / Number` tokens.
3. **Identifier renaming** — `obfuscator.cpp:collectIdentifiers()` builds `name → _0x...` map.
4. **String transform** — `obfuscator.cpp:obfuscateStringContent()` → if `--encode` is set, delegates to `encodings.cpp:encodeString()`, otherwise level-based hex/`bytes()` wrappers.
5. **Number obfuscation** — `obfuscateNumberContent()` → hex/oct/expr.
6. **File encoding** — `main.cpp` optionally calls `encodings::encodeFileContent()` for `--file-encoding`.
7. **Emit** — rebuilds source with optional minification and header.

```
src/
  main.cpp          CLI, interactive picker, file I/O
  obfuscator.hpp/cpp Core transforms
  lexer.hpp/cpp     Tokenizer
  language.hpp/cpp  Profiles
  encodings.hpp/cpp 50+ encoders (bases, ciphers, crypto, compression)
  utils.hpp         Helpers
```

---

## 🗂️ Project Structure

```
Code-obfuscator/
├── src/
│   ├── main.cpp
│   ├── obfuscator.hpp/cpp
│   ├── lexer.hpp/cpp
│   ├── language.hpp/cpp
│   ├── encodings.hpp/cpp   # 50+ encodings
│   └── utils.hpp
├── examples/
│   ├── example.cpp
│   ├── example.py
│   └── example.js
├── build.bat           # Windows one-click build
├── CMakeLists.txt
├── .gitignore
├── LICENSE (MIT)
└── README.md
```

---

## 🧪 Testing

```powershell
.\build\obfuscator.exe -i examples\example.cpp -o examples\example.obf.cpp --encode base64 --seed 42
type examples\example.obf.cpp

.\build\obfuscator.exe -i examples\example.py -o examples\example.obf.py --encode rot13
type examples\example.obf.py
python examples\example.obf.py

.\build\obfuscator.exe -i examples\example.js -o examples\example.obf.js --encode aes --xor-key key
type examples\example.obf.js

# List encodings
.\build\obfuscator.exe --list-encodings
.\build\obfuscator.exe --list-langs
```

---

## 🤝 Contributing

```bash
git clone https://github.com/Motatadev/Code-obfuscator.git
git checkout -b feature/new-encoding
# add encoder in src/encodings.cpp, register in listEncodings(), test, PR
```

Ideas: add **Rust/Go/PHP** profiles, real **AES-NI**, or a GUI.

---

## 📄 License

MIT — see [LICENSE](LICENSE).

---

## 👤 Author

**Motatadev** — https://github.com/Motatadev

> Made with C++17. If you like it, star ⭐ the repo!
