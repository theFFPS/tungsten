/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

module;
#include <tungsten/crypto.hpp>

export module tungsten.crypto:types;

import std;

export namespace tungsten {

struct cipher_ctx_deleter_t {
    void operator()(EVP_CIPHER_CTX* ctx) const {
        EVP_CIPHER_CTX_free(ctx);
    }
};
struct md_ctx_deleter_t {
    void operator()(EVP_MD_CTX* ctx) const {
        EVP_MD_CTX_free(ctx);
    }
};
struct pkey_ctx_deleter_t {
    void operator()(EVP_PKEY_CTX* ctx) const {
        EVP_PKEY_CTX_free(ctx);
    }
};
struct pkey_deleter_t {
    void operator()(EVP_PKEY* key) const {
        EVP_PKEY_free(key);
    }
};
struct bio_deleter_t {
    void operator()(BIO* bio) const {
        BIO_free_all(bio);
    }
};
struct bn_deleter_t {
    void operator()(BIGNUM* p) const {
        BN_clear_free(p);
    }
};

using cipher_ctx_t = std::unique_ptr<EVP_CIPHER_CTX, cipher_ctx_deleter_t>;
using hash_ctx_t = std::unique_ptr<EVP_MD_CTX, md_ctx_deleter_t>;
using pkey_ctx_t = std::unique_ptr<EVP_PKEY_CTX, pkey_ctx_deleter_t>;
using pkey_t = std::unique_ptr<EVP_PKEY, pkey_deleter_t>;
using bio_t = std::unique_ptr<BIO, bio_deleter_t>;
using bn_ptr_t = std::unique_ptr<BIGNUM, bn_deleter_t>;

void random(std::uint8_t *dest, std::size_t length) {
    if (RAND_bytes(dest, static_cast<int>(length)) != 1) {
        throw std::runtime_error(std::format("tungsten::random error: failed to generate random array of bytes size of {}", length));
    }
}
bool safe_compare(std::span<const std::uint8_t> a, std::span<const std::uint8_t> b) {
    if (a.size() != b.size()) return false;
    return sodium_memcmp(a.data(), b.data(), a.size()) == 0;
}

template <typename T> struct secure_allocator_t {
    using value_type = T;

    secure_allocator_t() noexcept = default;

    template <typename U> secure_allocator_t(const secure_allocator_t<U>&) noexcept {}

    T* allocate(std::size_t n) {
        if (n == 0) return nullptr;

        if (!CRYPTO_secure_malloc_initialized()) {
            throw std::runtime_error("secure_allocator_t::allocate error: OpenSSL secure heap is not initialized in main.");
        }

        void* ptr = OPENSSL_secure_zalloc(n * sizeof(T));
        if (!ptr) throw std::bad_alloc();
        return reinterpret_cast<T*>(ptr);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        if (!p) return;
        OPENSSL_secure_clear_free(p, n * sizeof(T));
    }

    bool operator==(const secure_allocator_t&) const noexcept { return true; }
    bool operator!=(const secure_allocator_t&) const noexcept { return false; }
};

}
