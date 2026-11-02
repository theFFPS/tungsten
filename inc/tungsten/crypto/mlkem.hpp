#pragma once

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/core_names.h>
#include <openssl/x509.h>
#include <vector>
#include <string>
#include <span>
#include <utility>
#include "shared.hpp"
#include <tungsten/util/allocator.hpp>

namespace tungsten {

struct mlkem1024_ctx_t {
private:
    pkey_t m_key;

    bool m_can_encapsulate = false;
    bool m_can_decapsulate = false;
public:
    mlkem1024_ctx_t() {}

    mlkem1024_ctx_t(const mlkem1024_ctx_t&) = delete;
    mlkem1024_ctx_t& operator=(const mlkem1024_ctx_t&) = delete;

    mlkem1024_ctx_t(mlkem1024_ctx_t&& other) noexcept
        : m_key(std::move(other.m_key)),
          m_can_encapsulate(other.m_can_encapsulate),
          m_can_decapsulate(other.m_can_decapsulate) {
        other.m_can_encapsulate = other.m_can_decapsulate = false;
    }

    mlkem1024_ctx_t& operator=(mlkem1024_ctx_t&& other) noexcept;

    std::vector<uint8_t> generate();

    std::vector<uint8_t> pubkey() const;
    std::string pubpem() const;
    bool set_pubkey(std::span<const uint8_t> pubkey);
    bool set_pubpem(std::string_view pubkey);

    std::vector<uint8_t, secure_allocator_t<uint8_t>> privkey() const;
    bool set_privkey(std::span<const uint8_t> privkey);

    struct kem_result_t {
        std::vector<uint8_t> ciphertext;
        std::vector<uint8_t, secure_allocator_t<uint8_t>> shared_secret;
    };

    kem_result_t encapsulate();
    std::vector<uint8_t, secure_allocator_t<uint8_t>> decapsulate(std::span<const uint8_t> cipher_text);

    bool can_encapsulate() const;
    bool can_decapsulate() const;

    size_t ct_size() const;
    size_t ss_size() const;
};

}
