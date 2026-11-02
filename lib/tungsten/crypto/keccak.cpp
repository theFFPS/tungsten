#include <tungsten/util/encoding.hpp>
#include <tungsten/crypto/keccak.hpp>

namespace tungsten {

keccak256_hash_t& keccak256_hash_t::operator=(keccak256_hash_t&& other) noexcept {
    if (this != &other) {
        EVP_MD_free(m_md);

        m_ctx = std::move(other.m_ctx);
        m_digest = std::move(other.m_digest);
        m_len = other.m_len;
        m_finalized = other.m_finalized;
        m_md = other.m_md;

        other.m_md = nullptr;
    }
    return *this;
}

void keccak256_hash_t::update(std::span<const uint8_t> data) {
    if (data.empty()) return;
    if (EVP_DigestUpdate(m_ctx.get(), data.data(), data.size()) != 1) throw std::runtime_error("keccak256_hash_t error: failed to update KECCAK256 digest");
    m_finalized = false;
}
void keccak256_hash_t::reset() {
    if (EVP_DigestInit_ex(m_ctx.get(), m_md, nullptr) != 1) throw std::runtime_error("keccak256_hash_t error: failed to reset KECCAK256 digest");
    m_digest.clear();
    m_finalized = false;
}
void keccak256_hash_t::finalize() {
    if (m_finalized) return;
    m_digest.resize(static_cast<size_t>(EVP_MD_get_size(m_md)));
    if (EVP_DigestFinal_ex(m_ctx.get(), m_digest.data(), &m_len) != 1) throw std::runtime_error("keccak256_hash_t error: failed to finalize KECCAK256 digest");
    m_finalized = true;
}

std::vector<uint8_t> keccak256_hash_t::to_bytes() {
    finalize();
    return m_digest;
}
std::string keccak256_hash_t::to_string() {
    finalize();
    return to_hex_str(m_digest);
}

bool verify_keccak256(std::span<const uint8_t> data, std::span<const uint8_t> expected) {
    keccak256_hash_t keccak;
    keccak.update(data);
    const auto& digest = keccak.to_bytes();
    return std::equal(digest.begin(), digest.end(), expected.begin(), expected.end());
}

bool verify_keccak256(std::span<const uint8_t> data, std::string_view expected) {
    keccak256_hash_t keccak;
    keccak.update(data);
    return expected == keccak.to_string();
}

}
