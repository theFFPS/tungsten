#pragma once

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <stdexcept>
#include <format>
#include <vector>
#include <span>
#include "shared.hpp"
#include <tungsten/util/allocator.hpp>

namespace tungsten {

template<size_t key_bitsize> struct aes_gcm_ctx_t {
    static_assert(key_bitsize == 128 || key_bitsize == 192 || key_bitsize == 256, "aes_gcm_ctx_t error: invalid key size. Must be 128, 192, or 256 bits.");
private:
    cipher_ctx_t m_enc_ctx, m_dec_ctx;

    constexpr const EVP_CIPHER* cipher() const {
        if constexpr (key_bitsize == 128) return EVP_aes_128_gcm();
        else if constexpr (key_bitsize == 192) return EVP_aes_192_gcm();
        else if constexpr (key_bitsize == 256) return EVP_aes_256_gcm();
    }
public:
    explicit aes_gcm_ctx_t(std::span<const uint8_t> key) {
        if (key.size() != key_bitsize / 8) throw std::runtime_error(std::format("aes_gcm_ctx_t error: invalid key size for AES-{}", key_bitsize));

        m_enc_ctx.reset(EVP_CIPHER_CTX_new());
        if (!m_enc_ctx) throw std::runtime_error("aes_gcm_ctx_t error: failed to create encryption context");
        m_dec_ctx.reset(EVP_CIPHER_CTX_new());
        if (!m_dec_ctx) throw std::runtime_error("aes_gcm_ctx_t error: failed to create decryption context");

        if (EVP_EncryptInit_ex(m_enc_ctx.get(), cipher(), nullptr, nullptr, nullptr) != 1 ||
            EVP_DecryptInit_ex(m_dec_ctx.get(), cipher(), nullptr, nullptr, nullptr) != 1) {
            throw std::runtime_error("aes_gcm_ctx_t error: failed to bind cipher type");
        }
        if (EVP_CIPHER_CTX_ctrl(m_enc_ctx.get(), EVP_CTRL_GCM_SET_IVLEN, 12, nullptr) != 1 ||
            EVP_CIPHER_CTX_ctrl(m_dec_ctx.get(), EVP_CTRL_GCM_SET_IVLEN, 12, nullptr) != 1) {
            throw std::runtime_error("aes_gcm_ctx_t error: failed to set IV length");
        }
        if (EVP_EncryptInit_ex(m_enc_ctx.get(), nullptr, nullptr, key.data(), nullptr) != 1 ||
            EVP_DecryptInit_ex(m_dec_ctx.get(), nullptr, nullptr, key.data(), nullptr) != 1) {
            throw std::runtime_error("aes_gcm_ctx_t error: failed to initialize contexts with key");
        }
    }

    std::vector<uint8_t> encrypt(std::span<const uint8_t> plain_text, std::span<const uint8_t> aad = {}) {
        std::vector<uint8_t> result (28);
        random(result.data(), 12);

        int len;
        std::vector<uint8_t> cipher_text (plain_text.size() + EVP_MAX_BLOCK_LENGTH);
        if (EVP_EncryptInit_ex(m_enc_ctx.get(), nullptr, nullptr, nullptr, result.data()) != 1)
            throw std::runtime_error("aes_gcm_ctx::encrypt error: failed to initialize encryptor");
        if (!aad.empty()) if (EVP_EncryptUpdate(m_enc_ctx.get(), nullptr, &len, aad.data(), aad.size()) != 1)
            throw std::runtime_error("aes_gcm_ctx::encrypt error: failed to apply AAD");
        if (EVP_EncryptUpdate(m_enc_ctx.get(), cipher_text.data(), &len, plain_text.data(), plain_text.size()) != 1)
            throw std::runtime_error("aes_gcm_ctx::encrypt error: failed to apply plain_text");
        int ct_len = len;
        if (EVP_EncryptFinal_ex(m_enc_ctx.get(), cipher_text.data() + len, &len) != 1)
            throw std::runtime_error("aes_gcm_ctx::encrypt error: failed to initialize encrypt");
        if (EVP_CIPHER_CTX_ctrl(m_enc_ctx.get(), EVP_CTRL_GCM_GET_TAG, 16, result.data() + 12) != 1)
            throw std::runtime_error("aes_gcm_ctx::encrypt error: failed to generate GCM tag");
        result.append_range(std::span<const uint8_t>(cipher_text.begin(), cipher_text.begin() + ct_len + len));
        return result;
    }
    std::vector<uint8_t, secure_allocator_t<uint8_t>> decrypt(std::span<const uint8_t> cipher_text, std::span<const uint8_t> aad = {}) {
        if (cipher_text.size() < 28) throw std::runtime_error("aes_gcm_ctx::decrypt error: cipher_text too short to contain IV and tag");

        std::span<const uint8_t> iv = cipher_text.subspan(0, 12), tag = cipher_text.subspan(12, 16), ct_data = cipher_text.subspan(28, cipher_text.size() - 28);

        int len;
        std::vector<uint8_t, secure_allocator_t<uint8_t>> plain_text (ct_data.size() + EVP_MAX_BLOCK_LENGTH);
        if (EVP_DecryptInit_ex(m_dec_ctx.get(), nullptr, nullptr, nullptr, iv.data()) != 1)
            throw std::runtime_error("aes_gcm_ctx::decrypt error: failed to initialize decryptor");
        if (!aad.empty()) if (EVP_DecryptUpdate(m_dec_ctx.get(), nullptr, &len, aad.data(), aad.size()) != 1)
            throw std::runtime_error("aes_gcm_ctx::decrypt error: failed to apply AAD");
        if (EVP_DecryptUpdate(m_dec_ctx.get(), plain_text.data(), &len, ct_data.data(), ct_data.size()) != 1)
            throw std::runtime_error("aes_gcm_ctx::decrypt error: failed to apply cipher_text");
        int pt_len = len;
        if (EVP_CIPHER_CTX_ctrl(m_dec_ctx.get(), EVP_CTRL_GCM_SET_TAG, 16, const_cast<uint8_t*>(tag.data())) != 1)
            throw std::runtime_error("aes_gcm_ctx::decrypt error: failed to verify GCM tag");
        if (EVP_DecryptFinal_ex(m_dec_ctx.get(), plain_text.data() + len, &len) != 1)
            throw std::runtime_error("aes_gcm_ctx::decrypt error: failed to decrypt");
        plain_text.resize(pt_len + len);
        return plain_text;
    }

    constexpr size_t key_size() const { return key_bitsize; }
};

template<size_t key_bitsize> struct aes_cfb8_ctx_t {
    static_assert(key_bitsize == 128 || key_bitsize == 192 || key_bitsize == 256, "aes_cfb8_ctx_t error: invalid key size. Must be 128, 192, or 256 bits.");
private:
    bool m_initialized = false;
    cipher_ctx_t m_enc_ctx, m_dec_ctx;

    constexpr const EVP_CIPHER* cipher() const {
        if constexpr (key_bitsize == 128) return EVP_aes_128_cfb8();
        else if constexpr (key_bitsize == 192) return EVP_aes_192_cfb8();
        else if constexpr (key_bitsize == 256) return EVP_aes_256_cfb8();
    }
public:
    explicit aes_cfb8_ctx_t(std::span<const uint8_t> key, std::span<const uint8_t> iv) {
        if (key.size() != key_bitsize / 8) throw std::runtime_error(std::format("aes_cfb8_ctx_t error: invalid key size for AES-{}", key_bitsize));

        m_enc_ctx.reset(EVP_CIPHER_CTX_new());
        if (!m_enc_ctx) throw std::runtime_error("aes_cfb8_ctx_t error: failed to create encryption context");
        m_dec_ctx.reset(EVP_CIPHER_CTX_new());
        if (!m_dec_ctx) throw std::runtime_error("aes_cfb8_ctx_t error: failed to create decryption context");
        if (EVP_EncryptInit_ex(m_enc_ctx.get(), cipher(), nullptr, key.data(), iv.data()) != 1)
            throw std::runtime_error("aes_cfb8_ctx_t error: failed to initialize encryption context");
        if (EVP_DecryptInit_ex(m_dec_ctx.get(), cipher(), nullptr, key.data(), iv.data()) != 1)
            throw std::runtime_error("aes_cfb8_ctx_t error: failed to initialize decryption context");

        m_initialized = true;
    }

    std::vector<uint8_t> encrypt(std::span<const uint8_t> plain_text) {
        int len;
        std::vector<uint8_t> cipher_text (plain_text.size() + EVP_MAX_BLOCK_LENGTH);
        if (EVP_EncryptUpdate(m_enc_ctx.get(), cipher_text.data(), &len, plain_text.data(), plain_text.size()) != 1)
            throw std::runtime_error("aes_cfb8_ctx::encrypt error: failed to apply plain_text");
        int ct_len = len;
        if (EVP_EncryptFinal_ex(m_enc_ctx.get(), cipher_text.data() + len, &len) != 1)
            throw std::runtime_error("aes_cfb8_ctx::encrypt error: failed to initialize encrypt");
        cipher_text.resize(ct_len + len);
        return cipher_text;
    }
    std::vector<uint8_t, secure_allocator_t<uint8_t>> decrypt(std::span<const uint8_t> cipher_text) {
        int len;
        std::vector<uint8_t, secure_allocator_t<uint8_t>> plain_text (cipher_text.size() + EVP_MAX_BLOCK_LENGTH);
        if (EVP_DecryptUpdate(m_dec_ctx.get(), plain_text.data(), &len, cipher_text.data(), cipher_text.size()) != 1)
            throw std::runtime_error("aes_cfb8_ctx::decrypt error: failed to apply cipher_text");
        int pt_len = len;
        if (EVP_DecryptFinal_ex(m_dec_ctx.get(), plain_text.data() + len, &len) != 1)
            throw std::runtime_error("aes_cfb8_ctx::decrypt error: failed to decrypt");
        plain_text.resize(pt_len + len);
        return plain_text;
    }

    constexpr size_t key_size() const { return key_bitsize; }
};

using aes256_gcm_t = aes_gcm_ctx_t<256>;
using aes128_cfb8_t = aes_cfb8_ctx_t<128>;

}
