#pragma once
#include <string>
#include <vector>

namespace encodings {

std::vector<std::string> listEncodings();
bool isValidEncoding(const std::string& name);
std::string normalizeEncodingName(const std::string& name);

std::string encodeString(const std::string& raw, const std::string& encoding, const std::string& lang, int caesarShift = 3, const std::string& xorKey = "K", const std::string& vigenereKey = "KEY");

std::string encodeFileContent(const std::string& content, const std::string& fileEncoding);

// Base encodings
std::string b2_encode(const std::string& in);
std::string b8_encode(const std::string& in);
std::string b10_encode(const std::string& in);
std::string b16_encode(const std::string& in);
std::string b32_encode(const std::string& in);
std::string b36_encode(const std::string& in);
std::string b45_encode(const std::string& in);
std::string b58_encode(const std::string& in);
std::string b62_encode(const std::string& in);
std::string b64_encode(const std::string& in);
std::string b64url_encode(const std::string& in);
std::string b85_encode(const std::string& in);
std::string b91_encode(const std::string& in);
std::string b92_encode(const std::string& in);
std::string b100_encode(const std::string& in);
std::string b122_encode(const std::string& in);
std::string b32768_encode(const std::string& in);

std::string url_encode(const std::string& in);
std::string html_encode(const std::string& in);
std::string xml_encode(const std::string& in);
std::string json_encode(const std::string& in);
std::string unicode_escape(const std::string& in);
std::string hex_escape(const std::string& in);
std::string octal_escape(const std::string& in);
std::string binary_encode(const std::string& in);

// ROT / substitution
std::string rot_encode(const std::string& in, int n);
std::string rot13_encode(const std::string& in);
std::string rot47_encode(const std::string& in);
std::string caesar_encode(const std::string& in, int shift);
std::string atbash_encode(const std::string& in);
std::string affine_encode(const std::string& in, int a=5, int b=8);
std::string vigenere_encode(const std::string& in, const std::string& key);
std::string beaufort_encode(const std::string& in, const std::string& key);
std::string autokey_encode(const std::string& in, const std::string& key);
std::string railfence_encode(const std::string& in, int rails=3);
std::string columnar_encode(const std::string& in, const std::string& key);

// Obfuscation helpers
std::string string_escape(const std::string& in);
std::string string_split(const std::string& in);
std::string string_concat(const std::string& in);
std::string char_substitution(const std::string& in);
std::string homoglyph_encode(const std::string& in);
std::string whitespace_encode(const std::string& in);

// Crypto (demo - reversible without external libs)
std::string xor_encode(const std::string& in, const std::string& key);
std::string aes_encode(const std::string& in, const std::string& key);
std::string chacha20_encode(const std::string& in, const std::string& key);
std::string rsa_encode(const std::string& in);
std::string des_encode(const std::string& in, const std::string& key);
std::string des3_encode(const std::string& in, const std::string& key);
std::string blowfish_encode(const std::string& in, const std::string& key);
std::string twofish_encode(const std::string& in, const std::string& key);

// Compression + encoding (stub using simple RLE+base64)
std::string gzip_b64_encode(const std::string& in);
std::string zlib_b64_encode(const std::string& in);
std::string deflate_b64_encode(const std::string& in);
std::string brotli_b64_encode(const std::string& in);
std::string lzma_b64_encode(const std::string& in);
std::string bzip2_b64_encode(const std::string& in);

std::string morse_encode(const std::string& in);
std::string quoted_printable_encode(const std::string& in);
std::string punycode_encode(const std::string& in);
std::string utf16_encode_hex(const std::string& in);
std::string utf32_encode_hex(const std::string& in);

}
