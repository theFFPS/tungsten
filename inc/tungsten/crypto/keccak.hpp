#pragma once

#include "shared.hpp"
#include <vector>
#include <span>

namespace tungsten {

struct keccak256_hash_t {
private:
    hash_ctx_t m_ctx;
    std::vector<uint8_t> m_digest;
    unsigned m_len;
    bool m_finalized = false;

    EVP_MD *m_md;
public:
    keccak256_hash_t() : m_ctx(EVP_MD_CTX_new()), m_md(EVP_MD_fetch(NULL, "KECCAK-256", NULL)) {
        if (!m_ctx) throw std::runtime_error("keccak256_hash_t error: failed to create hash_ctx_t");
        if (!m_md)  throw std::runtime_error("keccak256_hash_t error: failed to fetch KECCAK-256 algorithm");
        reset();
    }

    keccak256_hash_t(const keccak256_hash_t&) = delete;
    keccak256_hash_t& operator=(const keccak256_hash_t&) = delete;

    keccak256_hash_t(keccak256_hash_t&& other) noexcept
        : m_ctx(std::move(other.m_ctx)), m_digest(std::move(other.m_digest)), m_len(other.m_len), m_finalized(other.m_finalized), m_md(other.m_md) {
        other.m_md = nullptr;
    }
    keccak256_hash_t& operator=(keccak256_hash_t&& other) noexcept;

    ~keccak256_hash_t() {
        EVP_MD_free(m_md);
    }

    void update(std::span<const uint8_t> data);
    void reset();
    void finalize();

    std::vector<uint8_t> to_bytes();
    std::string to_string();
};

bool verify_keccak256(std::span<const uint8_t> data, std::span<const uint8_t> expected);
bool verify_keccak256(std::span<const uint8_t> data, std::string_view expected);

}
