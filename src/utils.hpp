#pragma once
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

namespace utils {

inline std::string toHex(int v, bool withPrefix = true) {
    std::stringstream ss;
    if (withPrefix) ss << "0x";
    ss << std::hex << v;
    return ss.str();
}

inline std::string randomHexString(std::mt19937 &rng, int len = 6) {
    std::stringstream ss;
    ss << "_0x";
    std::uniform_int_distribution<int> dist(0, 15);
    for (int i = 0; i < len; ++i) ss << std::hex << dist(rng);
    return ss.str();
}

inline std::string randomConfusingName(std::mt19937 &rng) {
    const char* chars = "Il1O0";
    std::uniform_int_distribution<int> lenDist(6, 12);
    std::uniform_int_distribution<int> charDist(0, 4);
    int len = lenDist(rng);
    std::string s;
    s.reserve(len);
    s += "_";
    for (int i = 0; i < len; ++i) s += chars[charDist(rng)];
    // Ensure not starting with digit after _
    return s;
}

inline std::string escapeStringToHex(const std::string& s) {
    std::string out;
    for (unsigned char c : s) {
        std::stringstream ss;
        ss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        out += ss.str();
    }
    return out;
}

inline std::string escapeStringToOctal(const std::string& s) {
    std::string out;
    for (unsigned char c : s) {
        std::stringstream ss;
        ss << "\\" << std::oct << (int)c;
        out += ss.str();
    }
    return out;
}

inline std::string encodeNumber(int n, std::mt19937& rng) {
    std::uniform_int_distribution<int> choice(0, 3);
    int c = choice(rng);
    switch (c) {
        case 0: return toHex(n, true);
        case 1: {
            // octal
            std::stringstream ss;
            ss << "0" << std::oct << n;
            return ss.str();
        }
        case 2: {
            // expression: (n-1+1) or (n ^ k ^ k)
            int r = std::uniform_int_distribution<int>(1, 100)(rng);
            return "(" + std::to_string(n - r) + "+" + std::to_string(r) + ")";
        }
        case 3: {
            int k = std::uniform_int_distribution<int>(1, 255)(rng);
            return "((" + std::to_string(n ^ k) + "^" + std::to_string(k) + "))";
        }
    }
    return std::to_string(n);
}

}
