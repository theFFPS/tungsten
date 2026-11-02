#pragma once

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/core_names.h>
#include <vector>
#include <span>
#include "shared.hpp"
#include <tungsten/util/allocator.hpp>

namespace tungsten {

struct ed25519_ctx_t {
private:
    pkey_t m_key;
    bool m_can_sign = false;
    bool m_can_verify = false;
public:
    ed25519_ctx_t() = default;
    ed25519_ctx_t(const ed25519_ctx_t&) = delete;
    ed25519_ctx_t& operator=(const ed25519_ctx_t&) = delete;

    ed25519_ctx_t(ed25519_ctx_t&& other) noexcept {
        m_key = std::move(other.m_key);
        m_can_sign = other.m_can_sign;
        m_can_verify = other.m_can_verify;
        other.m_can_sign = other.m_can_verify = false;
    }
    ed25519_ctx_t& operator=(ed25519_ctx_t&& other) noexcept;

    std::vector<uint8_t> generate();

    std::vector<uint8_t> pubkey() const;
    std::string pubpem() const;
    bool set_pubkey(std::span<const uint8_t> pubkey);
    bool set_pubpem(std::string_view pubkey);

    std::vector<uint8_t, secure_allocator_t<uint8_t>> privkey() const;
    bool set_privkey(std::span<const uint8_t> privkey);

    std::vector<uint8_t> sign(std::span<const uint8_t> data);
    bool verify(std::span<const uint8_t> data, std::span<const uint8_t> signature);

    bool can_sign() const;
    bool can_verify() const;
    std::string algo_name() const;
    size_t sigsize() const;
};

}
