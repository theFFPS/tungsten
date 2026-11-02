#include <tungsten/util/encoding.hpp>
#include <tungsten/crypto/shared.hpp>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <stdexcept>
#include <algorithm>

namespace tungsten {

std::string base64_t::encode(std::span<const uint8_t> in) {
    if (in.empty()) return {};

    size_t expected_out = 4 * ((in.size() + 2) / 3);
    std::string result(expected_out, '\0');

    int n = EVP_EncodeBlock(reinterpret_cast<uint8_t*>(result.data()), in.data(), static_cast<int>(in.size()));
    if (n < 0 || static_cast<size_t>(n) != expected_out) {
        throw std::runtime_error("base64_t::encode error: failed to encode base64");
    }
    return result;
}

std::vector<uint8_t> base64_t::decode(std::string_view in) {
    if (in.empty()) return {};
    if (in.size() % 4 != 0) throw std::invalid_argument("base64_t::decode error: input length must be a multiple of 4");

    std::vector<uint8_t> out_buffer((in.size() / 4) * 3 + 3);

    std::unique_ptr<EVP_ENCODE_CTX, decltype(&EVP_ENCODE_CTX_free)> ctx(EVP_ENCODE_CTX_new(), EVP_ENCODE_CTX_free);
    if (!ctx) throw std::runtime_error("base64_t::decode error: failed to create context");

    EVP_DecodeInit(ctx.get());

    int out_len = 0;
    if (EVP_DecodeUpdate(ctx.get(), out_buffer.data(), &out_len, reinterpret_cast<const uint8_t*>(in.data()), static_cast<int>(in.size())) < 0) {
        throw std::runtime_error("base64_t::decode error: failed update processing");
    }

    int tail_len = 0;
    if (EVP_DecodeFinal(ctx.get(), out_buffer.data() + out_len, &tail_len) < 0) {
        throw std::runtime_error("base64_t::decode error: failed final execution processing");
    }

    out_buffer.resize(static_cast<size_t>(out_len + tail_len));
    return out_buffer;
}

constexpr static char base58_chars[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
std::string base58_t::encode(std::span<const uint8_t> in) {
    if (in.empty()) return {};

    size_t zeros = 0;
    while (zeros < in.size() && in[zeros] == 0) ++zeros;

    if (zeros == in.size()) {
        return std::string(zeros, '1');
    }

    bn_ptr_t bn(BN_bin2bn(in.data() + zeros, static_cast<int>(in.size() - zeros), nullptr));
    if (!bn) throw std::runtime_error("base58_t::encode error: failed to initialize bignum");

    std::string out;
    out.reserve((in.size() - zeros) * 138 / 100 + 1);

    while (!BN_is_zero(bn.get())) {
        BN_ULONG rem = BN_div_word(bn.get(), 58);
        if (rem == static_cast<BN_ULONG>(-1)) {
            throw std::runtime_error("base58_t::encode error: failed to divide bignum");
        }
        out.push_back(base58_chars[rem]);
    }

    out.append(zeros, '1');
    std::reverse(out.begin(), out.end());
    return out;
}

std::vector<uint8_t> base58_t::decode(std::string_view in) {
    if (in.empty()) return {};

    int8_t map[256];
    std::fill(std::begin(map), std::end(map), static_cast<int8_t>(-1));
    for (int i = 0; i < 58; ++i) {
        map[static_cast<unsigned char>(base58_chars[i])] = static_cast<int8_t>(i);
    }

    size_t zeros = 0;
    while (zeros < in.size() && in[zeros] == '1') ++zeros;

    bn_ptr_t bn(BN_new());
    if (!bn) throw std::runtime_error("base58_t::decode error: failed to initialize bignum");
    BN_zero(bn.get());

    for (size_t i = zeros; i < in.size(); ++i) {
        int8_t v = map[static_cast<unsigned char>(in[i])];
        if (v < 0) throw std::invalid_argument(std::string("base58: invalid character '") + in[i] + "'");

        if (!BN_mul_word(bn.get(), 58) || !BN_add_word(bn.get(), static_cast<BN_ULONG>(v))) {
            throw std::runtime_error("base58_t::decode error: failed to multiply/add bignum");
        }
    }

    int nbytes = BN_num_bytes(bn.get());

    std::vector<uint8_t> tmp(nbytes > 0 ? nbytes : 1, 0);
    int written = BN_bn2bin(bn.get(), tmp.data());

    std::vector<uint8_t> out(zeros, 0);
    out.insert(out.end(), tmp.begin(), tmp.begin() + written);

    return out;
}

std::string to_hex_str(std::span<const uint8_t> in) {
    std::string str;
    str.reserve(in.size() * 2);
    static const char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < in.size(); ++i) {
        str += hex[in[i] >> 4];
        str += hex[in[i] & 0x0F];
    }
    return str;
}

}
