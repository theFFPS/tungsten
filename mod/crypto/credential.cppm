/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

module;
#include <tungsten/crypto.hpp>

export module tungsten.crypto:credential;

import std;
import :types;
import :aes;

export namespace tungsten {

enum class credential_storage_entry_type_t : std::uint8_t {
    public_key, private_key, signature, api_key, generic_secret
};
enum class credential_storage_entry_encryption_type_t : std::uint8_t {
    encrypted, not_encrypted
};

}

namespace tungsten {

inline void secure_zero(void* p, std::size_t n) noexcept {
    if (p && n) memset_explicit(p, 0, n);
}
template <typename T> inline void secure_zero(T& container) noexcept {
    if (!container.empty()) memset_explicit(container.data(), 0, container.size() * sizeof(typename T::value_type));
}

std::vector<std::uint8_t> make_aad(credential_storage_entry_type_t type,
                              credential_storage_entry_encryption_type_t enc,
                              std::uint16_t name_size,
                              std::uint16_t purpose_size,
                              std::uint16_t credential_size,
                              const std::uint8_t* name,
                              const std::uint8_t* purpose) {
    std::vector<std::uint8_t> aad;
    aad.reserve(1 + 1 + 2 + 2 + 2 + name_size + purpose_size);
    aad.push_back(static_cast<std::uint8_t>(type));
    aad.push_back(static_cast<std::uint8_t>(enc));
    auto push16 = [&aad](std::uint16_t v) {
        aad.push_back(static_cast<std::uint8_t>( v        & 0xFF));
        aad.push_back(static_cast<std::uint8_t>((v >> 8)  & 0xFF));
    };
    push16(name_size);
    push16(purpose_size);
    push16(credential_size);
    aad.insert(aad.end(), name,    name    + name_size);
    aad.insert(aad.end(), purpose, purpose + purpose_size);
    return aad;
}

}

export namespace tungsten {

template<typename T> requires std::integral<T> struct credential_holder_t {
private:
    std::vector<T, secure_allocator_t<T>> m_credential;
public:
    explicit credential_holder_t(std::size_t size) : m_credential(size) {}
    ~credential_holder_t() noexcept { cleanse(); }

    void cleanse() noexcept {
        if (!m_credential.empty()) memset_explicit(m_credential.data(), 0, m_credential.size() * sizeof(T));
    }

    credential_holder_t(const credential_holder_t&) = delete;
    credential_holder_t& operator=(const credential_holder_t&) = delete;
    credential_holder_t(credential_holder_t&&) = delete;
    credential_holder_t& operator=(credential_holder_t&&) = delete;

    std::size_t size() const noexcept { return m_credential.size(); }
    T*       data()       noexcept { return m_credential.data(); }
    const T* data() const noexcept { return m_credential.data(); }

    using iterator               = typename std::vector<T, secure_allocator_t<T>>::iterator;
    using const_iterator         = typename std::vector<T, secure_allocator_t<T>>::const_iterator;
    using reverse_iterator       = typename std::vector<T, secure_allocator_t<T>>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<T, secure_allocator_t<T>>::const_reverse_iterator;

    iterator begin() noexcept { return m_credential.begin(); }
    iterator end()   noexcept { return m_credential.end(); }
    const_iterator begin() const noexcept { return m_credential.begin(); }
    const_iterator end()   const noexcept { return m_credential.end(); }
    reverse_iterator rbegin() noexcept { return m_credential.rbegin(); }
    reverse_iterator rend()   noexcept { return m_credential.rend(); }
    const_reverse_iterator rbegin() const noexcept { return m_credential.rbegin(); }
    const_reverse_iterator rend()   const noexcept { return m_credential.rend(); }
    const_iterator cbegin() const noexcept { return m_credential.cbegin(); }
    const_iterator cend()   const noexcept { return m_credential.cend(); }
    const_reverse_iterator crbegin() const noexcept { return m_credential.crbegin(); }
    const_reverse_iterator crend()   const noexcept { return m_credential.crend(); }
};

struct credential_storage_entry_t {
    credential_storage_entry_type_t            m_type;
    credential_storage_entry_encryption_type_t m_encryption_type;
    std::uint16_t m_name_size;
    std::uint16_t m_purpose_size;
    std::uint16_t m_credential_size;
    std::uint8_t  m_credential_name[128];
    std::uint8_t  m_credential_purpose[512];
    std::uint8_t  m_credential_data[32120];

    credential_storage_entry_t() = default;
    credential_storage_entry_t(const credential_storage_entry_t&) = default;
    credential_storage_entry_t& operator=(const credential_storage_entry_t&) = default;
    credential_storage_entry_t(credential_storage_entry_t&&) = default;
    credential_storage_entry_t& operator=(credential_storage_entry_t&&) = default;
    ~credential_storage_entry_t() noexcept {
        memset_explicit(&m_credential_data, 0, sizeof(m_credential_data));
    }

    credential_storage_entry_type_t type() const noexcept {
        return m_type;
    }
    std::string_view name() const {
        const std::size_t n = std::min<std::size_t>(m_name_size, sizeof(credential_storage_entry_t::m_credential_name));
        return std::string_view(reinterpret_cast<const char*>(m_credential_name), n);
    }
    std::string_view purpose() const {
        const std::size_t n = std::min<std::size_t>(m_purpose_size, sizeof(credential_storage_entry_t::m_credential_purpose));
        return std::string_view(reinterpret_cast<const char*>(m_credential_purpose), n);
    }
    std::size_t credential_size() const noexcept {
        return m_credential_size;
    }

    void unlock(credential_holder_t<std::uint8_t>& dest, credential_holder_t<std::uint8_t>& key) {
        if (m_name_size > sizeof(credential_storage_entry_t::m_credential_name) || m_purpose_size > sizeof(credential_storage_entry_t::m_credential_purpose) || m_credential_size > sizeof(credential_storage_entry_t::m_credential_data)) {
            secure_zero(key.data(), key.size());
            throw std::runtime_error("credential_storage_entry_t::unlock error: corrupted entry header");
        }
        if (m_encryption_type != credential_storage_entry_encryption_type_t::encrypted &&
            m_encryption_type != credential_storage_entry_encryption_type_t::not_encrypted) {
            secure_zero(key.data(), key.size());
            throw std::runtime_error("credential_storage_entry_t::unlock error: unknown encryption type");
        }
        if (dest.size() != credential_size()) {
            secure_zero(key.data(), key.size());
            throw std::runtime_error("credential_storage_entry_t::unlock error: mismatched credential size");
        }

        if (m_encryption_type == credential_storage_entry_encryption_type_t::not_encrypted) {
            secure_zero(key.data(), key.size());
            if (m_credential_size > sizeof(credential_storage_entry_t::m_credential_data)) {
                throw std::runtime_error("credential_storage_entry_t::unlock error: credential too large");
            }
            std::copy_n(m_credential_data, m_credential_size, dest.begin());
            return;
        }

        if (key.size() != 32) {
            secure_zero(key.data(), key.size());
            throw std::runtime_error("credential_storage_entry_t::unlock error: invalid key size");
        }

        auto aad = make_aad(m_type, m_encryption_type,
                            m_name_size, m_purpose_size, m_credential_size,
                            m_credential_name, m_credential_purpose);

        std::vector<std::uint8_t, tungsten::secure_allocator_t<std::uint8_t>> plain;
        {
            aes256_gcm_t aes(key);
            plain = aes.decrypt(std::span<const std::uint8_t>(m_credential_data, sizeof(credential_storage_entry_t::m_credential_data)), aad);
            secure_zero(key.data(), key.size());
        }

        if (plain.size() < credential_size()) {
            secure_zero(plain);
            throw std::runtime_error("credential_storage_entry_t::unlock error: decrypted data too short");
        }
        std::copy_n(plain.begin(), credential_size(), dest.begin());
        secure_zero(plain);
    }

    static credential_storage_entry_t lock(
        credential_storage_entry_type_t type,
        credential_storage_entry_encryption_type_t encryption_type,
        std::string_view name,
        std::string_view purpose,
        credential_holder_t<std::uint8_t>& credential,
        credential_holder_t<std::uint8_t>& key
    ) {
        if (name.size() > sizeof(credential_storage_entry_t::m_credential_name) || purpose.size() > sizeof(credential_storage_entry_t::m_credential_purpose)) {
            secure_zero(credential.data(), credential.size());
            secure_zero(key.data(), key.size());
            throw std::runtime_error("credential_storage_entry_t::lock error: name/purpose too long");
        }
        const std::size_t cred_size = credential.size();

        const auto name_size    = static_cast<std::uint16_t>(name.size());
        const auto purpose_size = static_cast<std::uint16_t>(purpose.size());
        const auto cred_size16  = static_cast<std::uint16_t>(cred_size);

        auto aad = make_aad(type, encryption_type,
                            name_size, purpose_size, cred_size16,
                            reinterpret_cast<const std::uint8_t*>(name.data()),
                            reinterpret_cast<const std::uint8_t*>(purpose.data()));

        std::vector<std::uint8_t> secure_cred_data;

        if (encryption_type == credential_storage_entry_encryption_type_t::encrypted) {
            if (cred_size > sizeof(credential_storage_entry_t::m_credential_data) - 28) {
                secure_zero(credential.data(), credential.size());
                secure_zero(key.data(), key.size());
                throw std::runtime_error("credential_storage_entry_t::lock error: credential too large");
            }
            if (key.size() != 32) {
                secure_zero(credential.data(), credential.size());
                secure_zero(key.data(), key.size());
                throw std::runtime_error("credential_storage_entry_t::lock error: invalid key size");
            }

            std::vector<std::uint8_t> padded(sizeof(credential_storage_entry_t::m_credential_data) - 28, 0);
            try {
                std::copy_n(credential.begin(), cred_size, padded.begin());
                secure_zero(credential.data(), credential.size());

                aes256_gcm_t aes(key);
                auto encrypted = aes.encrypt(padded, aad);
                secure_zero(key.data(), key.size());
                secure_zero(padded);

                if (encrypted.size() != sizeof(credential_storage_entry_t::m_credential_data)) {
                    secure_zero(encrypted);
                    throw std::runtime_error("credential_storage_entry_t::lock error: unexpected ciphertext length");
                }
                secure_cred_data.assign(encrypted.begin(), encrypted.end());
                secure_zero(encrypted);
            } catch (...) {
                secure_zero(credential.data(), credential.size());
                secure_zero(key.data(), key.size());
                secure_zero(padded);
                throw;
            }
        } else if (encryption_type == credential_storage_entry_encryption_type_t::not_encrypted) {
            if (cred_size > sizeof(credential_storage_entry_t::m_credential_data)) {
                secure_zero(credential.data(), credential.size());
                secure_zero(key.data(), key.size());
                throw std::runtime_error("credential_storage_entry_t::lock error: credential too large");
            }
            secure_cred_data.assign(sizeof(credential_storage_entry_t::m_credential_data), 0);
            std::copy_n(credential.begin(), cred_size, secure_cred_data.begin());
            secure_zero(credential.data(), credential.size());
            secure_zero(key.data(), key.size());
        } else {
            secure_zero(credential.data(), credential.size());
            secure_zero(key.data(), key.size());
            throw std::runtime_error("credential_storage_entry_t::lock error: unknown encryption type");
        }

        credential_storage_entry_t result{};
        result.m_type            = type;
        result.m_encryption_type = encryption_type;
        result.m_name_size       = name_size;
        result.m_purpose_size    = purpose_size;
        result.m_credential_size = cred_size16;
        std::copy_n(name.begin(),    name_size,    reinterpret_cast<char*>(result.m_credential_name));
        std::copy_n(purpose.begin(), purpose_size, reinterpret_cast<char*>(result.m_credential_purpose));
        std::copy(secure_cred_data.begin(), secure_cred_data.end(), result.m_credential_data);
        secure_zero(secure_cred_data);
        return result;
    }
} __attribute__((packed));

struct credential_storage_t {
    std::string m_credential_storage_name;
    std::vector<std::unique_ptr<credential_storage_entry_t>> m_credentials;
};
struct credential_storage_disk_t {
    std::uint32_t m_magic;
    std::uint8_t  m_reserved[4];
    std::uint32_t m_flags;
    std::uint32_t m_checksum;
    std::uint64_t m_credential_storage_name_size;
    std::uint64_t m_credential_storage_name_off;
    std::uint64_t m_credential_storage_credentials_count;
    std::uint64_t m_credential_storage_credentials_start;
    std::uint64_t m_credential_storage_data_size;
    std::uint64_t m_credential_storage_data_off;
} __attribute__((packed));

}
