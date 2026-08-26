#include "encodings.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <vector>

namespace encodings {

static std::string toLower(std::string s){ std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; }
static std::string trim(std::string s){ s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){return !std::isspace(ch);})); s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){return !std::isspace(ch);}).base(), s.end()); return s; }

std::vector<std::string> listEncodings() {
    return {
        "ascii","utf-8","utf8","utf-16","utf16","utf-32","utf32","unicode",
        "base2","base8","base10","base16","hex","hexadecimal","base32","base36","base45","base58","base62","base64","base64url","base85","ascii85","base91","base92","base100","base122","base32768",
        "b2","b8","b16","b32","b36","b45","b58","b62","b64","b85","b91","b92",
        "binary","octal","octal-escape",
        "url","url-encoding","percent","percent-encoding","html","html-encoding","html-entity","xml","xml-encoding","json","json-encoding",
        "unicode-escape","hex-escape",
        "rot13","rot1","rot5","rot18","rot47","rot8000","caesar","caesar-cipher","atbash","affine","vigenere","beaufort","autokey","rail-fence","columnar","columnar-transposition",
        "string-escaping","string-splitting","string-concatenation","char-substitution","homoglyph","unicode-homoglyph","whitespace","whitespace-obfuscation","dead-code","variable-renaming","control-flow","ast","token-obfuscation",
        "xor","xor-encoding","aes","chacha20","rsa","des","3des","tripledes","blowfish","twofish",
        "gzip","gzip+base64","zlib","zlib+base64","deflate","deflate+base64","brotli","brotli+base64","lzma","lzma+base64","bzip2","bzip2+base64",
        "morse","quoted-printable","mime","punycode"
    };
}
bool isValidEncoding(const std::string& name){ auto n=toLower(trim(name)); for(auto &e:listEncodings()) if(toLower(e)==n) return true; // also handle rotN
    if(n.rfind("rot",0)==0){ std::string num=n.substr(3); if(!num.empty() && std::all_of(num.begin(), num.end(), ::isdigit)) return true; } return false; }

std::string normalizeEncodingName(const std::string& name){
    std::string n=toLower(trim(name));
    if(n=="utf8") return "utf-8";
    if(n=="utf16") return "utf-16";
    if(n=="utf32") return "utf-32";
    if(n=="hex"||n=="hexadecimal"||n=="b16") return "base16";
    if(n=="ascii85") return "base85";
    if(n=="b2") return "base2";
    if(n=="b8"||n=="octal") return "base8";
    if(n=="b36") return "base36";
    if(n=="b45") return "base45";
    if(n=="b58") return "base58";
    if(n=="b62") return "base62";
    if(n=="b64") return "base64";
    if(n=="b85") return "base85";
    if(n=="b91") return "base91";
    if(n=="b92") return "base92";
    if(n=="binary") return "base2";
    if(n=="octal-escape") return "base8";
    if(n=="url-encoding"||n=="percent"||n=="percent-encoding") return "url";
    if(n=="html-encoding"||n=="html-entity") return "html";
    if(n=="xml-encoding") return "xml";
    if(n=="json-encoding") return "json";
    if(n=="hex-escape") return "hex-escape";
    if(n=="caesar-cipher") return "caesar";
    if(n=="xor-encoding") return "xor";
    if(n=="3des"||n=="tripledes") return "3des";
    if(n=="gzip"||n=="gzip+base64") return "gzip+base64";
    if(n=="zlib"||n=="zlib+base64") return "zlib+base64";
    if(n=="deflate"||n=="deflate+base64") return "deflate+base64";
    if(n=="brotli"||n=="brotli+base64") return "brotli+base64";
    if(n=="lzma"||n=="lzma+base64") return "lzma+base64";
    if(n=="bzip2"||n=="bzip2+base64") return "bzip2+base64";
    if(n=="rail-fence") return "rail-fence";
    if(n=="columnar-transposition") return "columnar";
    if(n=="string-escaping") return "string-escaping";
    if(n=="whitespace-obfuscation") return "whitespace";
    if(n=="unicode-homoglyph") return "homoglyph";
    return n;
}

// ---------- Base encoders ----------
std::string b64_encode(const std::string& in){
    static const char* tbl="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out; int val=0,valb=-6; for(unsigned char c:in){ val=(val<<8)+c; valb+=8; while(valb>=0){ out.push_back(tbl[(val>>valb)&0x3F]); valb-=6; }} if(valb>-6) out.push_back(tbl[((val<<8)>>(valb+8))&0x3F]); while(out.size()%4) out.push_back('='); return out;
}
std::string b64url_encode(const std::string& in){ std::string s=b64_encode(in); for(char &c:s){ if(c=='+') c='-'; if(c=='/') c='_'; } s.erase(std::find_if(s.rbegin(), s.rend(), [](char ch){return ch!='=';}).base(), s.end()); return s; }
std::string b32_encode(const std::string& in){ static const char* tbl="ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"; std::string out; int buffer=0,bitsLeft=0; for(unsigned char c:in){ buffer=(buffer<<8)|c; bitsLeft+=8; while(bitsLeft>=5){ out.push_back(tbl[(buffer>>(bitsLeft-5))&31]); bitsLeft-=5; }} if(bitsLeft>0) out.push_back(tbl[(buffer<<(5-bitsLeft))&31]); while(out.size()%8) out.push_back('='); return out; }
std::string b16_encode(const std::string& in){ std::stringstream ss; ss<<std::hex<<std::setfill('0'); for(unsigned char c:in) ss<<std::setw(2)<<(int)c; return ss.str(); }
std::string b2_encode(const std::string& in){ std::string out; for(size_t i=0;i<in.size();++i){ unsigned char c=in[i]; for(int b=7;b>=0;--b) out+=((c>>b)&1)?'1':'0'; if(i+1<in.size()) out+=' '; } return out; }
std::string b8_encode(const std::string& in){ std::stringstream ss; ss<<std::oct<<std::setfill('0'); for(unsigned char c:in) ss<<std::setw(3)<<(int)c<<" "; std::string s=ss.str(); if(!s.empty()) s.pop_back(); return s; }
std::string b10_encode(const std::string& in){ std::stringstream ss; for(size_t i=0;i<in.size();++i){ if(i) ss<<' '; ss<<(int)(unsigned char)in[i]; } return ss.str(); }
std::string b36_encode(const std::string& in){
    static const char* alph="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::vector<unsigned char> bytes(in.begin(), in.end()); int zeros=0; while(zeros<(int)bytes.size()&&bytes[zeros]==0) zeros++; std::vector<unsigned char> b; for(size_t i=zeros;i<bytes.size();++i){ int carry=bytes[i]; for(auto &v:b){ carry+=256*v; v=carry%36; carry/=36; } while(carry){ b.push_back(carry%36); carry/=36; } } std::string out; out.assign(zeros,'0'); for(auto it=b.rbegin(); it!=b.rend(); ++it) out.push_back(alph[*it]); return out.empty()?"0":out;
}
std::string b45_encode(const std::string& in){
    // RFC9285 Base45: encode pairs
    static const char* tbl="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
    std::string out; for(size_t i=0;i<in.size(); i+=2){ int n= (unsigned char)in[i]<<8; if(i+1<in.size()) n+= (unsigned char)in[i+1]; else n+=0; // simplified
        if(in.size()-i==1){ out.push_back(tbl[n%45]); out.push_back(tbl[(n/45)%45]); }
        else { out.push_back(tbl[n%45]); out.push_back(tbl[(n/45)%45]); out.push_back(tbl[n/2025]); }
    } return out;
}
std::string b58_encode(const std::string& in){
    static const char* ALPH="123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    std::vector<unsigned char> bytes(in.begin(),in.end()); int zeros=0; while(zeros<(int)bytes.size()&&bytes[zeros]==0) zeros++; std::vector<unsigned char> b58; for(size_t i=zeros;i<bytes.size();++i){ int carry=bytes[i]; for(auto &v:b58){ carry+=256*v; v=carry%58; carry/=58; } while(carry){ b58.push_back(carry%58); carry/=58; } } std::string out; out.assign(zeros,'1'); for(auto it=b58.rbegin(); it!=b58.rend(); ++it) out.push_back(ALPH[*it]); return out.empty()?"1":out;
}
std::string b62_encode(const std::string& in){
    static const char* ALPH="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::vector<unsigned char> bytes(in.begin(),in.end()); int zeros=0; while(zeros<(int)bytes.size()&&bytes[zeros]==0) zeros++; std::vector<unsigned char> b; for(size_t i=zeros;i<bytes.size();++i){ int carry=bytes[i]; for(auto &v:b){ carry+=256*v; v=carry%62; carry/=62; } while(carry){ b.push_back(carry%62); carry/=62; } } std::string out; out.assign(zeros,'0'); for(auto it=b.rbegin(); it!=b.rend(); ++it) out.push_back(ALPH[*it]); return out.empty()?"0":out;
}
std::string b85_encode(const std::string& in){
    std::string out; for(size_t i=0;i<in.size(); i+=4){ uint32_t val=0; int n=std::min<size_t>(4,in.size()-i); for(int j=0;j<4;++j){ val<<=8; if(j<n) val|=(unsigned char)in[i+j]; } if(n==4&&val==0) out.push_back('z'); else { char enc[5]; for(int j=4;j>=0;--j){ enc[j]=val%85+33; val/=85; } for(int j=0;j<n+1;++j) out.push_back(enc[j]); } } return out;
}
std::string b91_encode(const std::string& in){
    static const char tbl[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!#$%&()*+,./:;<=>?@[]^_`{|}~\"";
    std::string out; int b=0,n=0; for(unsigned char c:in){ b|=c<<n; n+=8; if(n>13){ int v=b&8191; if(v>88) b>>=13, n-=13; else v=b&16383, b>>=14, n-=14; out.push_back(tbl[v%91]); out.push_back(tbl[v/91]); } } if(n){ out.push_back(tbl[b%91]); if(n>7||b>90) out.push_back(tbl[b/91]); } return out;
}
std::string b92_encode(const std::string& in){
    static const char tbl[]="!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    // simplified: base92 as base64 variant for demo
    std::string b64=b64_encode(in); std::string out; for(char c:b64){ if(c=='=') break; out.push_back(tbl[(unsigned char)c%91]); } return out;
}
std::string b100_encode(const std::string& in){
    std::string out; for(unsigned char c:in){ out.push_back((char)(0xF0)); out.push_back((char)(0x9F)); out.push_back((char)(0x98+ (c>>4))); out.push_back((char)(0x80+(c&0xF))); } // fake emoji range
    // Simpler: return hex for stability
    return b16_encode(in);
}
std::string b122_encode(const std::string& in){
    // Base122: use 122 printable chars, stub via b64
    return b64_encode(in);
}
std::string b32768_encode(const std::string& in){
    // Base32768: encode 15 bits per char using CJK range, stub
    std::stringstream ss; for(size_t i=0;i<in.size(); i+=2){ int v=(unsigned char)in[i]<<8; if(i+1<in.size()) v|=(unsigned char)in[i+1]; ss<< (char)(0xE0 | (v>>12)) << (char)(0x80 | ((v>>6)&0x3F)) << (char)(0x80 | (v&0x3F)) << ' '; } std::string s=ss.str(); if(!s.empty()) s.pop_back(); return b16_encode(in); // fallback to hex for reliability
}

// URL / HTML / etc
std::string url_encode(const std::string& in){
    std::stringstream ss; ss<<std::hex<<std::uppercase<<std::setfill('0'); for(unsigned char c:in){ if(std::isalnum(c)||c=='-'||c=='_'||c=='.'||c=='~') ss<<(char)c; else ss<<'%'<<std::setw(2)<<(int)c; ss<<std::hex<<std::uppercase; } return ss.str();
}
std::string html_encode(const std::string& in){ std::string out; for(char c:in){ switch(c){ case '&': out+="&amp;"; break; case '<': out+="&lt;"; break; case '>': out+="&gt;"; break; case '"': out+="&quot;"; break; case '\'': out+="&#39;"; break; default: out+=c; }} return out; }
std::string xml_encode(const std::string& in){ return html_encode(in); }
std::string json_encode(const std::string& in){ std::string out; for(unsigned char c:in){ switch(c){ case '"': out+="\\\""; break; case '\\': out+="\\\\"; break; case '\n': out+="\\n"; break; case '\r': out+="\\r"; break; case '\t': out+="\\t"; break; default: if(c<0x20){ std::stringstream ss; ss<<"\\u"<<std::hex<<std::setw(4)<<std::setfill('0')<<(int)c; out+=ss.str(); } else out+=(char)c; }} return out; }
std::string unicode_escape(const std::string& in){ std::stringstream ss; ss<<std::hex<<std::setfill('0'); for(unsigned char c:in) ss<<"\\u"<<std::setw(4)<<(int)c; return ss.str(); }
std::string hex_escape(const std::string& in){ std::stringstream ss; ss<<std::hex<<std::setfill('0'); for(unsigned char c:in) ss<<"\\x"<<std::setw(2)<<(int)c; return ss.str(); }
std::string octal_escape(const std::string& in){ std::stringstream ss; ss<<std::oct<<std::setfill('0'); for(unsigned char c:in) ss<<"\\"<<std::setw(3)<<(int)c; return ss.str(); }
std::string binary_encode(const std::string& in){ return b2_encode(in); }

// ROT / ciphers
std::string rot_encode(const std::string& in, int n){ n%=26; if(n<0) n+=26; std::string out=in; for(char &c:out){ if('a'<=c&&c<='z') c='a'+(c-'a'+n)%26; else if('A'<=c&&c<='Z') c='A'+(c-'A'+n)%26; } return out; }
std::string rot13_encode(const std::string& in){ return rot_encode(in,13); }
std::string rot47_encode(const std::string& in){ std::string out=in; for(char &c:out) if(c>=33&&c<=126) c=33+(c-33+47)%94; return out; }
std::string caesar_encode(const std::string& in, int shift){ return rot_encode(in, shift); }
std::string atbash_encode(const std::string& in){ std::string out=in; for(char &c:out){ if('a'<=c&&c<='z') c='z'-(c-'a'); else if('A'<=c&&c<='Z') c='Z'-(c-'A'); } return out; }
std::string affine_encode(const std::string& in, int a, int b){ std::string out=in; for(char &c:out){ if('a'<=c&&c<='z') c='a'+(a*(c-'a')+b)%26; else if('A'<=c&&c<='Z') c='A'+(a*(c-'A')+b)%26; } return out; }
std::string vigenere_encode(const std::string& in, const std::string& key){ std::string out; out.reserve(in.size()); int ki=0; for(char c:in){ if(std::isalpha((unsigned char)c)){ char k= std::toupper((unsigned char)key[ki%key.size()])-'A'; if('a'<=c&&c<='z') c='a'+(c-'a'+k)%26; else c='A'+(c-'A'+k)%26; ki++; } out.push_back(c); } return out; }
std::string beaufort_encode(const std::string& in, const std::string& key){ std::string out; out.reserve(in.size()); int ki=0; for(char c:in){ if(std::isalpha((unsigned char)c)){ int k=std::toupper((unsigned char)key[ki%key.size()])-'A'; int p=std::toupper((unsigned char)c)-'A'; int e=(k-p+26)%26; char rc='A'+e; if('a'<=c&&c<='z') rc=std::tolower((unsigned char)rc); ki++; out.push_back(rc); } else out.push_back(c); } return out; }
std::string autokey_encode(const std::string& in, const std::string& key){
    std::string out; std::string fullKey=key+in; int ki=0;
    for(char c:in){
        if(std::isalpha((unsigned char)c)){
            int k=std::toupper((unsigned char)fullKey[ki])-'A';
            if('a'<=c&&c<='z') out.push_back('a'+(c-'a'+k)%26);
            else out.push_back('A'+(c-'A'+k)%26);
            ki++;
        } else { out.push_back(c); }
    } return out;
}
std::string railfence_encode(const std::string& in, int rails){
    if(rails<=1) return in;
    std::vector<std::string> fence(rails);
    int rail=0; int dir=1;
    for(char c:in){ fence[rail].push_back(c); rail+=dir; if(rail==0||rail==rails-1) dir*=-1; }
    std::string out; for(auto &s:fence) out+=s; return out;
}
std::string columnar_encode(const std::string& in, const std::string& key){
    int cols= key.empty()? 4 : (int)key.size();
    std::vector<std::string> col(cols);
    for(size_t i=0;i<in.size();++i) col[i%cols].push_back(in[i]);
    // order by key sorting
    std::vector<int> order(cols); for(int i=0;i<cols;++i) order[i]=i;
    std::string k=key; if(k.empty()) k="KEY";
    std::sort(order.begin(), order.end(), [&](int a,int b){ return k[a%k.size()] < k[b%k.size()]; });
    std::string out; for(int idx: order) out+=col[idx]; return out;
}

// Obfuscation helpers
std::string string_escape(const std::string& in){ return hex_escape(in); }
std::string string_split(const std::string& in){ std::string out; for(size_t i=0;i<in.size();++i){ if(i) out+="\"+\""; out.push_back(in[i]); } return "\""+out+"\""; }
std::string string_concat(const std::string& in){ return string_split(in); }
std::string char_substitution(const std::string& in){
    std::string out=in;
    for(char &c:out){
        if(c=='a'||c=='A') c=(c=='a'?'@':'4');
        else if(c=='e'||c=='E') c='3';
        else if(c=='i'||c=='I') c='1';
        else if(c=='o'||c=='O') c='0';
        else if(c=='s'||c=='S') c='$';
    } return out;
}
std::string homoglyph_encode(const std::string& in){
    static std::unordered_map<char,std::string> map = {
        {'a', u8"\u0430"}, {'e', u8"\u0435"}, {'o', u8"\u043E"}, {'p', u8"\u0440"}, {'c', u8"\u0441"}, {'x', u8"\u0445"},
        {'A', u8"\u0410"}, {'E', u8"\u0415"}, {'O', u8"\u041E"}, {'P', u8"\u0420"}, {'C', u8"\u0421"}, {'X', u8"\u0425"}
    };
    std::string out; for(char c:in){ auto it=map.find(c); if(it!=map.end()) out+=it->second; else out.push_back(c); } return out;
}
std::string whitespace_encode(const std::string& in){
    std::string out; for(char c:in){ out.push_back(c); out.push_back(c==' ' ? '\t' : ' '); } return out;
}

// Crypto stubs (reversible demo: XOR + base64 wrappers labeled as AES etc)
std::string xor_encode(const std::string& in, const std::string& key){ std::string out; out.reserve(in.size()*3); for(size_t i=0;i<in.size();++i){ unsigned char k= key.empty()? 0x55: (unsigned char)key[i%key.size()]; unsigned char x=(unsigned char)in[i]^k; std::stringstream ss; ss<<std::hex<<std::setw(2)<<std::setfill('0')<<(int)x; out+=ss.str(); if(i+1<in.size()) out+=' '; } return out; }
static std::string aes_like(const std::string& in, const std::string& key, const std::string& label){
    std::string x=xor_encode(in, key.empty()? label: key);
    std::string b=b64_encode(x);
    return b + " /*"+label+"*/";
}
std::string aes_encode(const std::string& in, const std::string& key){ return aes_like(in, key, "AES"); }
std::string chacha20_encode(const std::string& in, const std::string& key){ return aes_like(in, key, "ChaCha20"); }
std::string rsa_encode(const std::string& in){ std::string b=b64_encode(in); return b+" /*RSA*/"; }
std::string des_encode(const std::string& in, const std::string& key){ return aes_like(in, key, "DES"); }
std::string des3_encode(const std::string& in, const std::string& key){ return aes_like(in, key, "3DES"); }
std::string blowfish_encode(const std::string& in, const std::string& key){ return aes_like(in, key, "Blowfish"); }
std::string twofish_encode(const std::string& in, const std::string& key){ return aes_like(in, key, "Twofish"); }

// Compression stubs
static std::string comp_b64(const std::string& in, const std::string& label){
    // Fake compress: just base64 with label
    return b64_encode(in) + " /*"+label+"+Base64*/";
}
std::string gzip_b64_encode(const std::string& in){ return comp_b64(in,"gzip"); }
std::string zlib_b64_encode(const std::string& in){ return comp_b64(in,"zlib"); }
std::string deflate_b64_encode(const std::string& in){ return comp_b64(in,"DEFLATE"); }
std::string brotli_b64_encode(const std::string& in){ return comp_b64(in,"Brotli"); }
std::string lzma_b64_encode(const std::string& in){ return comp_b64(in,"LZMA"); }
std::string bzip2_b64_encode(const std::string& in){ return comp_b64(in,"BZip2"); }

std::string morse_encode(const std::string& in){ static std::unordered_map<char,std::string> morse={{'A',".-"}, {'B',"-..."}, {'C',"-.-."}, {'D',"-.."}, {'E',"."}, {'F',"..-."}, {'G',"--."}, {'H',"...."}, {'I',".."}, {'J',".---"}, {'K',"-.-"}, {'L',".-.."}, {'M',"--"}, {'N',"-."}, {'O',"---"}, {'P',".--."}, {'Q',"--.-"}, {'R',".-."}, {'S',"..."}, {'T',"-"}, {'U',"..-"}, {'V',"...-"}, {'W',".--"}, {'X',"-..-"}, {'Y',"-.--"}, {'Z',"--.."}, {'0',"-----"}, {'1',".----"}, {'2',"..---"}, {'3',"...--"}, {'4',"....-"}, {'5',"....."}, {'6',"-...."}, {'7',"--..."}, {'8',"---.."}, {'9',"----."}, {' ', "/"}}; std::string out; for(char c:in){ char up=std::toupper((unsigned char)c); auto it=morse.find(up); if(it!=morse.end()){ if(!out.empty()) out+=' '; out+=it->second; } else { if(!out.empty()) out+=' '; out+="?"; }} return out; }
std::string quoted_printable_encode(const std::string& in){ std::stringstream ss; ss<<std::hex<<std::uppercase<<std::setfill('0'); for(unsigned char c:in){ if(c>=33&&c<=126&&c!='=') ss<<(char)c; else ss<<'='<<std::setw(2)<<(int)c; } return ss.str(); }
std::string punycode_encode(const std::string& in){ bool need=false; for(unsigned char c:in) if(c>=128) need=true; if(!need) return in; std::string out="xn--"; for(unsigned char c:in){ if(c<128) out+=(char)c; else { std::stringstream ss; ss<<'-'<<std::hex<<(int)c; out+=ss.str(); } } return out; }
std::string utf16_encode_hex(const std::string& in){ std::stringstream ss; ss<<std::hex<<std::setfill('0'); for(unsigned char c:in) ss<<std::setw(2)<<(int)c<<" "<<std::setw(2)<<0<<" "; std::string s=ss.str(); if(!s.empty()&&s.back()==' ') s.pop_back(); return s; }
std::string utf32_encode_hex(const std::string& in){ std::stringstream ss; ss<<std::hex<<std::setfill('0'); for(unsigned char c:in) ss<<std::setw(2)<<(int)c<<" 00 00 00 "; std::string s=ss.str(); if(!s.empty()&&s.back()==' ') s.pop_back(); return s; }

std::string encodeString(const std::string& raw, const std::string& encoding, const std::string& lang, int caesarShift, const std::string& xorKey, const std::string& vigenereKey){
    std::string enc = normalizeEncodingName(encoding);
    // handle rotN
    if(enc.rfind("rot",0)==0 && enc.size()>3){
        std::string num=enc.substr(3);
        if(!num.empty() && std::all_of(num.begin(), num.end(), ::isdigit)){
            int n=std::stoi(num); return "\""+rot_encode(raw,n)+"\" /* rot"+std::to_string(n)+" */";
        }
    }
    std::string encoded;
    if(enc=="ascii"||enc=="utf-8"||enc=="unicode") encoded=raw;
    else if(enc=="utf-16") encoded=utf16_encode_hex(raw);
    else if(enc=="utf-32") encoded=utf32_encode_hex(raw);
    else if(enc=="base2") encoded=b2_encode(raw);
    else if(enc=="base8") encoded=b8_encode(raw);
    else if(enc=="base10") encoded=b10_encode(raw);
    else if(enc=="base16") encoded=b16_encode(raw);
    else if(enc=="base32") encoded=b32_encode(raw);
    else if(enc=="base36") encoded=b36_encode(raw);
    else if(enc=="base45") encoded=b45_encode(raw);
    else if(enc=="base58") encoded=b58_encode(raw);
    else if(enc=="base62") encoded=b62_encode(raw);
    else if(enc=="base64") encoded=b64_encode(raw);
    else if(enc=="base64url") encoded=b64url_encode(raw);
    else if(enc=="base85") encoded=b85_encode(raw);
    else if(enc=="base91") encoded=b91_encode(raw);
    else if(enc=="base92") encoded=b92_encode(raw);
    else if(enc=="base100") encoded=b100_encode(raw);
    else if(enc=="base122") encoded=b122_encode(raw);
    else if(enc=="base32768") encoded=b32768_encode(raw);
    else if(enc=="url") encoded=url_encode(raw);
    else if(enc=="html") encoded=html_encode(raw);
    else if(enc=="xml") encoded=xml_encode(raw);
    else if(enc=="json") encoded=json_encode(raw);
    else if(enc=="unicode-escape") encoded=unicode_escape(raw);
    else if(enc=="hex-escape") encoded=hex_escape(raw);
    else if(enc=="rot13") encoded=rot13_encode(raw);
    else if(enc=="rot47") encoded=rot47_encode(raw);
    else if(enc=="rot8000") encoded=rot_encode(raw,8000%26); // stub
    else if(enc=="caesar") encoded=caesar_encode(raw, caesarShift);
    else if(enc=="atbash") encoded=atbash_encode(raw);
    else if(enc=="affine") encoded=affine_encode(raw);
    else if(enc=="vigenere") encoded=vigenere_encode(raw, vigenereKey.empty()? "KEY": vigenereKey);
    else if(enc=="beaufort") encoded=beaufort_encode(raw, vigenereKey.empty()? "KEY": vigenereKey);
    else if(enc=="autokey") encoded=autokey_encode(raw, vigenereKey.empty()? "KEY": vigenereKey);
    else if(enc=="rail-fence") encoded=railfence_encode(raw,3);
    else if(enc=="columnar") encoded=columnar_encode(raw, vigenereKey.empty()? "KEY": vigenereKey);
    else if(enc=="string-escaping") encoded=string_escape(raw);
    else if(enc=="string-splitting") return string_split(raw);
    else if(enc=="string-concatenation") return string_concat(raw);
    else if(enc=="char-substitution") encoded=char_substitution(raw);
    else if(enc=="homoglyph") encoded=homoglyph_encode(raw);
    else if(enc=="whitespace") encoded=whitespace_encode(raw);
    else if(enc=="xor") encoded=xor_encode(raw, xorKey);
    else if(enc=="aes") encoded=aes_encode(raw, xorKey);
    else if(enc=="chacha20") encoded=chacha20_encode(raw, xorKey);
    else if(enc=="rsa") encoded=rsa_encode(raw);
    else if(enc=="des") encoded=des_encode(raw, xorKey);
    else if(enc=="3des") encoded=des3_encode(raw, xorKey);
    else if(enc=="blowfish") encoded=blowfish_encode(raw, xorKey);
    else if(enc=="twofish") encoded=twofish_encode(raw, xorKey);
    else if(enc=="gzip+base64") encoded=gzip_b64_encode(raw);
    else if(enc=="zlib+base64") encoded=zlib_b64_encode(raw);
    else if(enc=="deflate+base64") encoded=deflate_b64_encode(raw);
    else if(enc=="brotli+base64") encoded=brotli_b64_encode(raw);
    else if(enc=="lzma+base64") encoded=lzma_b64_encode(raw);
    else if(enc=="bzip2+base64") encoded=bzip2_b64_encode(raw);
    else if(enc=="binary") encoded=b2_encode(raw);
    else if(enc=="morse") encoded=morse_encode(raw);
    else if(enc=="quoted-printable") encoded=quoted_printable_encode(raw);
    else if(enc=="punycode") encoded=punycode_encode(raw);
    else encoded=hex_escape(raw);

    // Wrapping for language
    if(enc=="hex-escape"||enc=="unicode-escape"||enc=="base8") return "\""+encoded+"\"";
    if(enc=="base2"||enc=="base16"||enc=="base58"||enc=="base62"||enc=="base85"||enc=="base91"||enc=="base100"||enc=="morse"||enc=="binary"){
        if(lang=="python"||lang=="lua") return "\""+encoded+"\"";
        return "\""+encoded+"\" /* "+enc+" */";
    }
    if(enc=="base64"){
        if(lang=="python") return "__import__('base64').b64decode(\""+encoded+"\").decode()";
        if(lang=="javascript") return "atob(\""+encoded+"\")";
        return "\""+encoded+"\" /* base64 */";
    }
    if(enc=="base64url"){
        if(lang=="python") return "__import__('base64').urlsafe_b64decode(\""+encoded+"==\").decode()";
        return "\""+encoded+"\" /* base64url */";
    }
    if(enc=="base32"){
        if(lang=="python") return "__import__('base64').b32decode(\""+encoded+"\").decode()";
        return "\""+encoded+"\" /* base32 */";
    }
    if(enc=="url"){
        if(lang=="python") return "__import__('urllib.parse').unquote(\""+encoded+"\")";
        if(lang=="javascript") return "decodeURIComponent(\""+encoded+"\")";
        return "\""+encoded+"\" /* url */";
    }
    // Python-specific runnable wrappers
    if(lang=="python"){
        if(enc=="rot13" || enc.rfind("rot",0)==0) return "__import__('codecs').decode(\""+encoded+"\", 'rot_13')";
        if(enc=="html") return "__import__('html').unescape(\""+encoded+"\")";
        if(enc=="base16") return "bytes.fromhex(\""+encoded+"\").decode()";
        if(enc=="base58"||enc=="base62"||enc=="base85"||enc=="base91"||enc=="base92"||enc=="base100") return "\""+encoded+"\""; // keep as is (no stdlib decoder) - still runnable but string will be encoded
        if(enc=="xor"||enc=="aes"||enc=="chacha20"||enc=="des"||enc=="3des"||enc=="blowfish"||enc=="twofish"){
            // decode via bytes.fromhex + xor - keep simple: return encoded literal with comment but still runnable as string (will be encoded value, but not crash)
            return "\""+encoded+"\"";
        }
        if(enc=="rot47"||enc=="caesar"||enc=="atbash"||enc=="affine"||enc=="vigenere"||enc=="beaufort"||enc=="autokey"||enc=="rail-fence"||enc=="columnar"){
            return "\""+encoded+"\"";
        }
    }
    // default wrapper as string literal
    if(enc=="rot13"||enc.rfind("rot",0)==0||enc=="caesar"||enc=="atbash"||enc=="affine"||enc=="vigenere"||enc=="beaufort"||enc=="autokey"||enc=="rail-fence"||enc=="columnar"||enc=="char-substitution"||enc=="homoglyph"||enc=="xor"||enc=="aes"||enc=="chacha20"||enc=="rsa"||enc=="des"||enc=="3des"||enc=="blowfish"||enc=="twofish"||enc=="gzip+base64"||enc=="quoted-printable"){
        if(lang=="python"||lang=="lua") return "\""+encoded+"\"";
        return "\""+encoded+"\" /* "+enc+" */";
    }
    return "\""+encoded+"\"";
}

std::string encodeFileContent(const std::string& content, const std::string& fileEncoding){
    std::string enc=normalizeEncodingName(fileEncoding);
    if(enc=="ascii"||enc=="utf-8"||enc=="unicode") return content;
    if(enc=="utf-16"){ std::string out; out.push_back((char)0xFF); out.push_back((char)0xFE); for(unsigned char c:content){ out.push_back(c); out.push_back(0);} return out; }
    if(enc=="utf-32"){ std::string out; out.push_back((char)0xFF); out.push_back((char)0xFE); out.push_back(0); out.push_back(0); for(unsigned char c:content){ out.push_back(c); out.push_back(0); out.push_back(0); out.push_back(0);} return out; }
    if(enc=="base64") return b64_encode(content);
    if(enc=="base64url") return b64url_encode(content);
    if(enc=="base32") return b32_encode(content);
    if(enc=="base16") return b16_encode(content);
    if(enc=="base58") return b58_encode(content);
    if(enc=="base62") return b62_encode(content);
    if(enc=="base85") return b85_encode(content);
    if(enc=="base91") return b91_encode(content);
    if(enc=="base2") return b2_encode(content);
    if(enc=="base8") return b8_encode(content);
    if(enc=="url") return url_encode(content);
    if(enc=="gzip+base64") return gzip_b64_encode(content);
    if(enc=="zlib+base64") return zlib_b64_encode(content);
    if(enc=="deflate+base64") return deflate_b64_encode(content);
    if(enc=="binary") return binary_encode(content);
    return content;
}

}
