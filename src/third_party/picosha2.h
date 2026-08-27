#ifndef PICOSHA2_H
#define PICOSHA2_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

namespace picosha2 {
typedef unsigned int word32;
typedef unsigned char byte;

namespace detail {
inline word32 mask_ndigits(size_t n) { return (1 << n) - 1; }
inline word32 rotr(word32 x, size_t n) { return (x >> n) | (x << (32 - n)); }

const static word32 add_to_k[] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef4a3f7, 0xc67178f2
};

class hash256_one_by_one {
public:
    hash256_one_by_one() { init(); }
    void init() {
        h_[0] = 0x6a09e667; h_[1] = 0xbb67ae85; h_[2] = 0x3c6ef372; h_[3] = 0xa54ff53a;
        h_[4] = 0x510e527f; h_[5] = 0x9b05688c; h_[6] = 0x1f83d9ab; h_[7] = 0x5be0cd19;
        data_length_ = 0;
        buffer_length_ = 0;
    }
    template <typename RaIt>
    void process(RaIt first, RaIt last) {
        for (; first != last; ++first) {
            buffer_[buffer_length_++] = static_cast<byte>(*first);
            if (buffer_length_ == 64) {
                transform();
                buffer_length_ = 0;
            }
        }
        data_length_ += std::distance(first, last);
    }
    void finish() {
        byte pack = 0x80;
        buffer_[buffer_length_++] = pack;
        if (buffer_length_ > 56) {
            while (buffer_length_ < 64) buffer_[buffer_length_++] = 0x00;
            transform();
            buffer_length_ = 0;
        }
        while (buffer_length_ < 56) buffer_[buffer_length_++] = 0x00;
        word32 bit_len = static_cast<word32>(data_length_ * 8);
        for (int i = 7; i >= 0; --i) buffer_[56 + i] = static_cast<byte>(bit_len >> ((7 - i) * 8));
        transform();
    }
    template <typename OutIt>
    void get_hash_bytes(OutIt first, OutIt last) {
        for (size_t i = 0; i < 8 && first != last; ++i) {
            for (int j = 3; j >= 0 && first != last; --j) {
                *first++ = static_cast<byte>((h_[i] >> (j * 8)) & 0xff);
            }
        }
    }
private:
    void transform() {
        word32 w[64];
        for (size_t i = 0; i < 16; ++i) {
            w[i] = (static_cast<word32>(buffer_[i * 4]) << 24) |
                   (static_cast<word32>(buffer_[i * 4 + 1]) << 16) |
                   (static_cast<word32>(buffer_[i * 4 + 2]) << 8) |
                   (static_cast<word32>(buffer_[i * 4 + 3]));
        }
        for (size_t i = 16; i < 64; ++i) {
            word32 s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            word32 s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        word32 a = h_[0], b = h_[1], c = h_[2], d = h_[3], e = h_[4], f = h_[5], g = h_[6], h = h_[7];
        for (size_t i = 0; i < 64; ++i) {
            word32 S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            word32 ch = (e & f) ^ ((~e) & g);
            word32 temp1 = h + S1 + ch + add_to_k[i] + w[i];
            word32 S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            word32 maj = (a & b) ^ (a & c) ^ (b & c);
            word32 temp2 = S0 + maj;
            h = g; g = f; f = e; e = d + temp1; d = c; c = b; b = a; a = temp1 + temp2;
        }
        h_[0] += a; h_[1] += b; h_[2] += c; h_[3] += d;
        h_[4] += e; h_[5] += f; h_[6] += g; h_[7] += h;
    }
    word32 h_[8];
    byte buffer_[64];
    size_t buffer_length_;
    size_t data_length_;
};
} // namespace detail

template <typename InIt>
void hash256_hex_string(InIt first, InIt last, std::string& hex_str) {
    detail::hash256_one_by_one hasher;
    hasher.process(first, last);
    hasher.finish();
    byte hash[32];
    hasher.get_hash_bytes(hash, hash + 32);
    std::stringstream ss;
    for (size_t i = 0; i < 32; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    hex_str = ss.str();
}

inline void hash256_hex_string(const std::string& src, std::string& hex_str) {
    hash256_hex_string(src.begin(), src.end(), hex_str);
}
} // namespace picosha2

#endif