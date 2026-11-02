/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

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
