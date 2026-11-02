/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <openssl/evp.h>
#include <memory>
#include <span>

namespace tungsten {

struct cipher_ctx_deleter_t { void operator()(EVP_CIPHER_CTX* ctx) const; };
struct md_ctx_deleter_t { void operator()(EVP_MD_CTX* ctx) const; };
struct pkey_ctx_deleter_t { void operator()(EVP_PKEY_CTX* ctx) const; };
struct pkey_deleter_t { void operator()(EVP_PKEY* key) const; };
struct bio_deleter_t { void operator()(BIO* bio) const; };
struct bn_deleter_t { void operator()(BIGNUM* p) const; };

using cipher_ctx_t = std::unique_ptr<EVP_CIPHER_CTX, cipher_ctx_deleter_t>;
using hash_ctx_t = std::unique_ptr<EVP_MD_CTX, md_ctx_deleter_t>;
using pkey_ctx_t = std::unique_ptr<EVP_PKEY_CTX, pkey_ctx_deleter_t>;
using pkey_t = std::unique_ptr<EVP_PKEY, pkey_deleter_t>;
using bio_t = std::unique_ptr<BIO, bio_deleter_t>;
using bn_ptr_t = std::unique_ptr<BIGNUM, bn_deleter_t>;

void random(uint8_t *dest, size_t length);
bool safe_compare(std::span<const uint8_t> a, std::span<const uint8_t> b);

}
