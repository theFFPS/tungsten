/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include "shared.hpp"
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/core_names.h>
#include <stdexcept>
#include <vector>
#include <span>
#include <tungsten/util/allocator.hpp>

namespace tungsten {

template<size_t key_size> struct rsa_ctx_t {
    static_assert(key_size >= 1024 && key_size <= 16384, "rsa_ctx_t error: key size must be between 1024 and 16384 bits");
private:
    pkey_t m_key;

    bool m_can_encrypt = false;
    bool m_can_decrypt = false;
public:
    rsa_ctx_t() {}

    rsa_ctx_t(const rsa_ctx_t&) = delete;
    rsa_ctx_t& operator=(const rsa_ctx_t&) = delete;

    rsa_ctx_t(rsa_ctx_t&& other) noexcept
        : m_key(std::move(other.m_key)),
          m_can_encrypt(other.m_can_encrypt),
          m_can_decrypt(other.m_can_decrypt) {
        other.m_can_decrypt = false;
        other.m_can_encrypt = false;
    }

    rsa_ctx_t& operator=(rsa_ctx_t&& other) noexcept {
        if (this != &other) {
            m_key = std::move(other.m_key);
            m_can_encrypt = other.m_can_encrypt;
            m_can_decrypt = other.m_can_decrypt;
            other.m_can_decrypt = false;
            other.m_can_encrypt = false;
        }
        return *this;
    }

    std::vector<uint8_t> generate() {
        pkey_ctx_t ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr));
        if (!ctx) throw std::runtime_error("rsa_ctx_t error: failed to create pkey_ctx_t");

        if (EVP_PKEY_keygen_init(ctx.get()) <= 0) throw std::runtime_error("rsa_ctx_t failed to initialize keygen");
        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), static_cast<int>(key_size)) <= 0) throw std::runtime_error("rsa_ctx_t failed to set key size");

        bn_ptr_t e (BN_new());
        if (!e) throw std::runtime_error("rsa_ctx_t error: failed to create BIGNUM for exponent");

        if (BN_set_word(e.get(), RSA_F4) != 1) throw std::runtime_error("rsa_ctx_t error: failed to set exponent value");

        if (EVP_PKEY_CTX_set1_rsa_keygen_pubexp(ctx.get(), e.get()) <= 0)
            throw std::runtime_error("rsa_ctx_t error: failed to apply public exponent");

        EVP_PKEY* raw_key = nullptr;
        if (EVP_PKEY_generate(ctx.get(), &raw_key) <= 0) throw std::runtime_error("rsa_ctx_t error: failed to generate keyring");
        m_key.reset(raw_key);

        m_can_encrypt = true;
        m_can_decrypt = true;
        return this->pubkey();
    }

    std::vector<uint8_t> pubkey() const {
        if (!m_key) throw std::runtime_error("rsa_ctx_t error: key structure is empty");
        bio_t bio(BIO_new(BIO_s_mem()));
        if (!bio) throw std::runtime_error("rsa_ctx_t error: failed to create bio_t");
        if (i2d_PUBKEY_bio(bio.get(), m_key.get()) != 1) throw std::runtime_error("rsa_ctx_t error: failed to write public key to DER");

        char* data = nullptr;
        long len = BIO_get_mem_data(bio.get(), &data);
        if (len <= 0) throw std::runtime_error("rsa_ctx_t error: failed to export public key");

        const auto* udata = reinterpret_cast<const uint8_t*>(data);
        return std::vector<uint8_t>(udata, udata + len);
    }

    std::string pubpem() const {
        if (!m_key) throw std::runtime_error("rsa_ctx_t error: key structure is empty");
        bio_t bio(BIO_new(BIO_s_mem()));
        if (!bio) throw std::runtime_error("rsa_ctx_t error: failed to create bio_t");
        if (PEM_write_bio_PUBKEY(bio.get(), m_key.get()) != 1) throw std::runtime_error("rsa_ctx_t error: failed to write public key to PEM");

        char* data = nullptr;
        long len = BIO_get_mem_data(bio.get(), &data);
        if (len <= 0) throw std::runtime_error("rsa_ctx_t error: failed to export public key");

        return std::string(data, static_cast<size_t>(len));
    }

    bool set_pubkey(std::span<const uint8_t> pubkey) {
        bio_t bio(BIO_new_mem_buf(pubkey.data(), static_cast<int>(pubkey.size())));
        if (!bio) throw std::runtime_error("rsa_ctx_t error: failed to create bio_t");

        EVP_PKEY* raw_key = nullptr;
        if (!d2i_PUBKEY_bio(bio.get(), &raw_key)) {
            m_can_encrypt = false;
            return false;
        }

        m_key.reset(raw_key);
        m_can_encrypt = (m_key != nullptr);
        return m_can_encrypt;
    }

    bool set_pubpem(std::string_view pubkey) {
        bio_t bio(BIO_new_mem_buf(pubkey.data(), static_cast<int>(pubkey.size())));
        if (!bio) throw std::runtime_error("rsa_ctx_t error: failed to create bio_t");

        m_key.reset(PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr));
        m_can_encrypt = (m_key != nullptr);
        return m_can_encrypt;
    }

    std::vector<uint8_t, secure_allocator_t<uint8_t>> privkey() const {
        if (!m_key) throw std::runtime_error("rsa_ctx_t error: key structure is empty");
        bio_t bio(BIO_new(BIO_s_secmem()));
        if (!bio) throw std::runtime_error("rsa_ctx_t error: failed to create bio_t");
        if (i2d_PrivateKey_bio(bio.get(), m_key.get()) != 1) throw std::runtime_error("rsa_ctx_t error: failed to write private key to DER");

        char* data = nullptr;
        long len = BIO_get_mem_data(bio.get(), &data);
        if (len <= 0) throw std::runtime_error("rsa_ctx_t error: failed to export private key");

        const auto* udata = reinterpret_cast<const uint8_t*>(data);
        return std::vector<uint8_t, secure_allocator_t<uint8_t>>(udata, udata + len);
    }

    bool set_privkey(std::span<const uint8_t> privkey) {
        bio_t bio(BIO_new_mem_buf(privkey.data(), static_cast<int>(privkey.size())));
        if (!bio) throw std::runtime_error("rsa_ctx_t error: failed to create bio_t");

        EVP_PKEY* raw_key = nullptr;
        if (!d2i_PrivateKey_bio(bio.get(), &raw_key)) {
            m_can_decrypt = false;
            return false;
        }

        m_key.reset(raw_key);
        m_can_decrypt = (m_key != nullptr);
        return m_can_decrypt;
    }

    std::vector<uint8_t> encrypt(std::span<const uint8_t> plaintext) {
        if (!m_can_encrypt || !m_key) throw std::runtime_error("rsa_ctx_t error: cannot encrypt without public key");
        pkey_ctx_t ctx(EVP_PKEY_CTX_new_from_pkey(nullptr, m_key.get(), nullptr));
        if (!ctx) throw std::runtime_error("rsa_ctx_t error: failed to create encryption context");
        if (EVP_PKEY_encrypt_init(ctx.get()) <= 0) throw std::runtime_error("rsa_ctx_t error: failed to initialize encryption");
        if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) <= 0) throw std::runtime_error("rsa_ctx_t error: failed to set PKCS#1 v1.5 padding");

        size_t outlen = 0;
        if (EVP_PKEY_encrypt(ctx.get(), nullptr, &outlen, plaintext.data(), plaintext.size()) <= 0)
            throw std::runtime_error("rsa_ctx_t error: failed to determine cipher_text size");

        std::vector<uint8_t> cipher_text(outlen);
        if (EVP_PKEY_encrypt(ctx.get(), cipher_text.data(), &outlen, plaintext.data(), plaintext.size()) <= 0)
            throw std::runtime_error("rsa_ctx_t error: encryption failed");

        cipher_text.resize(outlen);
        return cipher_text;
    }
    std::vector<uint8_t, secure_allocator_t<uint8_t>> decrypt(std::span<const uint8_t> cipher_text) {
        if (!m_can_decrypt || !m_key) throw std::runtime_error("rsa_ctx_t error: cannot decrypt without private key");
        pkey_ctx_t ctx(EVP_PKEY_CTX_new_from_pkey(nullptr, m_key.get(), nullptr));
        if (!ctx) throw std::runtime_error("rsa_ctx_t error: failed to create decryption context");
        if (EVP_PKEY_decrypt_init(ctx.get()) <= 0)
            throw std::runtime_error("rsa_ctx_t error: failed to initialize decryption");
        if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) <= 0)
            throw std::runtime_error("rsa_ctx_t error: failed to set PKCS#1 v1.5 padding");
        size_t outlen = 0;
        if (EVP_PKEY_decrypt(ctx.get(), nullptr, &outlen, cipher_text.data(), cipher_text.size()) <= 0)
            throw std::runtime_error("rsa_ctx_t error: failed to determine plaintext size");
        std::vector<uint8_t, secure_allocator_t<uint8_t>> plaintext(outlen);
        if (EVP_PKEY_decrypt(ctx.get(), plaintext.data(), &outlen, cipher_text.data(), cipher_text.size()) <= 0)
            throw std::runtime_error("rsa_ctx_t error: decryption failed");
        plaintext.resize(outlen);
        return plaintext;
    }

    bool can_encrypt() const { return m_can_encrypt; }
    bool can_decrypt() const { return m_can_decrypt; }
};

using rsa_ctx_1024_t = rsa_ctx_t<1024>;

}
