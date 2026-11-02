/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/crypto/mlkem.hpp>

namespace tungsten {

mlkem1024_ctx_t& mlkem1024_ctx_t::operator=(mlkem1024_ctx_t&& other) noexcept {
    if (this != &other) {
        m_key = std::move(other.m_key);
        m_can_encapsulate = other.m_can_encapsulate;
        m_can_decapsulate = other.m_can_decapsulate;
        other.m_can_encapsulate = other.m_can_decapsulate = false;
    }
    return *this;
}

std::vector<uint8_t> mlkem1024_ctx_t::generate() {
    pkey_ctx_t ctx(EVP_PKEY_CTX_new_from_name(nullptr, "ML-KEM-1024", nullptr));
    if (!ctx) throw std::runtime_error("mlkem1024_ctx_t error: failed to create pkey_ctx_t");

    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) throw std::runtime_error("mlkem1024_ctx_t error: failed to initialize keygen");

    EVP_PKEY* raw_key = nullptr;
    if (EVP_PKEY_generate(ctx.get(), &raw_key) <= 0) throw std::runtime_error("mlkem1024_ctx_t error: failed to generate keyring");
    m_key.reset(raw_key);

    m_can_encapsulate = true;
    m_can_decapsulate = true;
    return this->pubkey();
}

std::vector<uint8_t> mlkem1024_ctx_t::pubkey() const {
    if (!m_key) throw std::runtime_error("mlkem1024_ctx_t error: key structure is empty");
    bio_t bio(BIO_new(BIO_s_mem()));
    if (!bio) throw std::runtime_error("mlkem1024_ctx_t error: failed to create bio_t");
    if (i2d_PUBKEY_bio(bio.get(), m_key.get()) != 1) throw std::runtime_error("mlkem1024_ctx_t error: failed to write public key to DER");

    char* data = nullptr;
    long len = BIO_get_mem_data(bio.get(), &data);
    if (len <= 0) throw std::runtime_error("mlkem1024_ctx_t error: failed to export public key");

    const auto* udata = reinterpret_cast<const uint8_t*>(data);
    return std::vector<uint8_t>(udata, udata + len);
}

std::string mlkem1024_ctx_t::pubpem() const {
    if (!m_key) throw std::runtime_error("mlkem1024_ctx_t error: key structure is empty");
    bio_t bio(BIO_new(BIO_s_mem()));
    if (!bio) throw std::runtime_error("mlkem1024_ctx_t error: failed to create bio_t");
    if (PEM_write_bio_PUBKEY(bio.get(), m_key.get()) != 1) throw std::runtime_error("mlkem1024_ctx_t error: failed to write public key to PEM");

    char* data = nullptr;
    long len = BIO_get_mem_data(bio.get(), &data);
    if (len <= 0) throw std::runtime_error("mlkem1024_ctx_t error: failed to export public key");

    return std::string(data, static_cast<size_t>(len));
}

bool mlkem1024_ctx_t::set_pubkey(std::span<const uint8_t> pubkey) {
    bio_t bio(BIO_new_mem_buf(pubkey.data(), static_cast<int>(pubkey.size())));
    if (!bio) throw std::runtime_error("mlkem1024_ctx_t error: failed to create bio_t");

    EVP_PKEY* raw_key = nullptr;
    if (!d2i_PUBKEY_bio(bio.get(), &raw_key)) {
        m_can_encapsulate = false;
        return false;
    }

    m_key.reset(raw_key);
    m_can_encapsulate = (m_key != nullptr);
    return m_can_encapsulate;
}

bool mlkem1024_ctx_t::set_pubpem(std::string_view pubkey) {
    bio_t bio(BIO_new_mem_buf(pubkey.data(), static_cast<int>(pubkey.size())));
    if (!bio) throw std::runtime_error("mlkem1024_ctx_t error: failed to create bio_t");

    m_key.reset(PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr));
    m_can_encapsulate = (m_key != nullptr);
    return m_can_encapsulate;
}

std::vector<uint8_t, secure_allocator_t<uint8_t>> mlkem1024_ctx_t::privkey() const {
    if (!m_key) throw std::runtime_error("mlkem1024_ctx_t error: key structure is empty");
    bio_t bio(BIO_new(BIO_s_secmem()));
    if (!bio) throw std::runtime_error("mlkem1024_ctx_t error: failed to create bio_t");
    if (i2d_PrivateKey_bio(bio.get(), m_key.get()) != 1) throw std::runtime_error("mlkem1024_ctx_t error: failed to write private key to DER");

    char* data = nullptr;
    long len = BIO_get_mem_data(bio.get(), &data);
    if (len <= 0) throw std::runtime_error("mlkem1024_ctx_t error: failed to export private key");

    const auto* udata = reinterpret_cast<const uint8_t*>(data);
    return std::vector<uint8_t, secure_allocator_t<uint8_t>>(udata, udata + len);
}

bool mlkem1024_ctx_t::set_privkey(std::span<const uint8_t> privkey) {
    bio_t bio(BIO_new_mem_buf(privkey.data(), static_cast<int>(privkey.size())));
    if (!bio) throw std::runtime_error("mlkem1024_ctx_t error: failed to create bio_t");

    EVP_PKEY* raw_key = nullptr;
    if (!d2i_PrivateKey_bio(bio.get(), &raw_key)) {
        m_can_decapsulate = false;
        return false;
    }

    m_key.reset(raw_key);
    m_can_decapsulate = (m_key != nullptr);
    return m_can_decapsulate;
}

mlkem1024_ctx_t::kem_result_t mlkem1024_ctx_t::encapsulate() {
    if (!m_can_encapsulate || !m_key) throw std::runtime_error("mlkem1024_ctx_t error: cannot encapsulate without public key");

    pkey_ctx_t ctx(EVP_PKEY_CTX_new_from_pkey(nullptr, m_key.get(), nullptr));
    if (!ctx) throw std::runtime_error("mlkem1024_ctx_t error: failed to create pkey_ctx_t");
    if (EVP_PKEY_encapsulate_init(ctx.get(), nullptr) <= 0) throw std::runtime_error("mlkem1024_ctx_t error: failed to initialize encapsulator");

    std::vector<uint8_t> cipher_text(ct_size());
    std::vector<uint8_t, secure_allocator_t<uint8_t>> secret(ss_size());
    size_t ct_len = cipher_text.size();
    size_t ss_len = secret.size();

    if (EVP_PKEY_encapsulate(ctx.get(), cipher_text.data(), &ct_len, secret.data(), &ss_len) <= 0)
        throw std::runtime_error("mlkem1024_ctx_t error: failed to encapsulate");

    cipher_text.resize(ct_len);
    secret.resize(ss_len);
    return { std::move(cipher_text), std::move(secret) };
}

std::vector<uint8_t, secure_allocator_t<uint8_t>> mlkem1024_ctx_t::decapsulate(std::span<const uint8_t> cipher_text) {
    if (!m_can_decapsulate || !m_key) throw std::runtime_error("mlkem1024_ctx_t error: cannot decapsulate without private key");

    pkey_ctx_t ctx(EVP_PKEY_CTX_new_from_pkey(nullptr, m_key.get(), nullptr));
    if (!ctx) throw std::runtime_error("mlkem1024_ctx_t error: failed to create pkey_ctx_t");
    if (EVP_PKEY_decapsulate_init(ctx.get(), nullptr) <= 0) throw std::runtime_error("mlkem1024_ctx_t error: failed to initialize decapsulator");

    std::vector<uint8_t, secure_allocator_t<uint8_t>> secret(ss_size());
    size_t ss_len = secret.size();

    if (EVP_PKEY_decapsulate(ctx.get(), secret.data(), &ss_len, cipher_text.data(), cipher_text.size()) <= 0)
        throw std::runtime_error("mlkem1024_ctx_t error: failed to decapsulate");

    secret.resize(ss_len);
    return secret;
}

bool mlkem1024_ctx_t::can_encapsulate() const { return m_can_encapsulate && m_key != nullptr; }
bool mlkem1024_ctx_t::can_decapsulate() const { return m_can_decapsulate && m_key != nullptr; }

size_t mlkem1024_ctx_t::ct_size() const { return 1568; }
size_t mlkem1024_ctx_t::ss_size() const { return 32; }

}
