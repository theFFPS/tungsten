#include <format>
#include <tungsten/crypto/shared.hpp>
#include <openssl/rand.h>
#include <sodium.h>

namespace tungsten {

void cipher_ctx_deleter_t::operator()(EVP_CIPHER_CTX* ctx) const {
    EVP_CIPHER_CTX_free(ctx);
}
void md_ctx_deleter_t::operator()(EVP_MD_CTX* ctx) const {
    EVP_MD_CTX_free(ctx);
}
void pkey_ctx_deleter_t::operator()(EVP_PKEY_CTX* ctx) const {
    EVP_PKEY_CTX_free(ctx);
}
void pkey_deleter_t::operator()(EVP_PKEY* key) const {
    EVP_PKEY_free(key);
}
void bio_deleter_t::operator()(BIO* key) const {
    BIO_free_all(key);
}
void bn_deleter_t::operator()(BIGNUM* p) const {
    BN_clear_free(p);
}

void random(uint8_t *dest, size_t length) {
    if (RAND_bytes(dest, static_cast<int>(length)) != 1) {
        throw std::runtime_error(std::format("tungsten::random error: failed to generate random array of bytes size of {}", length));
    }
}
bool safe_compare(std::span<const uint8_t> a, std::span<const uint8_t> b) {
    if (a.size() != b.size()) return false;
    return sodium_memcmp(a.data(), b.data(), a.size()) == 0;
}

}
