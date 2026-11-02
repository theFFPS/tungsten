#pragma once

#include "shared.hpp"
#include <tungsten/util/encoding.hpp>
#include <stdexcept>
#include <vector>
#include <span>
#include <format>

namespace tungsten {

template<const EVP_MD* (*hash_fn)()> struct shake_hash_t {
private:
    hash_ctx_t m_ctx;
    std::vector<uint8_t> m_digest;
    unsigned m_len;
    bool m_finalized = false;
public:
    shake_hash_t(size_t len) : m_ctx(EVP_MD_CTX_new()), m_len(len) {
        if (!m_ctx) throw std::runtime_error("shake_hash_t error: failed to create hash_ctx_t");
        reset();
    }

    shake_hash_t(const shake_hash_t&) = delete;
    shake_hash_t& operator=(const shake_hash_t&) = delete;
    shake_hash_t(shake_hash_t&& other) noexcept : m_ctx(std::move(other.m_ctx)), m_digest(std::move(other.m_digest)), m_len(other.m_len), m_finalized(other.m_finalized) {}
    shake_hash_t& operator=(shake_hash_t&& other) noexcept {
        if (this != &other) {
            m_ctx = std::move(other.m_ctx);
            m_digest = std::move(other.m_digest);
            m_len = other.m_len;
            m_finalized = other.m_finalized;
        }
        return *this;
    }

    void update(std::span<const uint8_t> data) {
        if (data.empty()) return;
        if (EVP_DigestUpdate(m_ctx.get(), data.data(), data.size()) != 1) throw std::runtime_error("shake_hash_t error: failed to update SHAKE digest");
        m_finalized = false;
    }
    void reset() {
        if (EVP_DigestInit_ex(m_ctx.get(), hash_fn(), nullptr) != 1) throw std::runtime_error("shake_hash_t error: failed to reset SHAKE digest");
        m_digest.clear();
        m_finalized = false;
    }
    void finalize() {
        if (m_finalized) return;
        m_digest.resize(m_len);
        if (EVP_DigestFinalXOF(m_ctx.get(), m_digest.data(), m_len) != 1) throw std::runtime_error("shake_hash_t error: failed to finalize SHAKE digest");
        m_finalized = true;
    }

    std::vector<uint8_t> to_bytes() {
        finalize();
        return m_digest;
    }
    std::string to_string() {
        finalize();
        return to_hex_str(m_digest);
    }
};

using shake_256_t = shake_hash_t<EVP_shake256>;

template<typename algo> bool verify_shake(std::span<const uint8_t> data, std::span<const uint8_t> expected) {
    algo shake (expected.size());
    shake.update(data);
    const auto& digest = shake.to_bytes();
    return std::equal(digest.begin(), digest.end(), expected.begin(), expected.end());
}

template<typename algo> bool verify_shake(std::span<const uint8_t> data, std::string_view expected) {
    if (expected.size() % 2) {
        throw std::runtime_error(std::format("tungsten::verify_shake error: malformed hash '{}'", expected));
    }
    algo shake (expected.size() / 2);
    shake.update(data);
    return expected == shake.to_string();
}

}
