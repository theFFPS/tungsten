/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <cstring>
#include <tungsten/config/credential_storage.hpp>
#include <stdexcept>
#include <string_view>
#include <algorithm>

#include <tungsten/crypto.hpp>
import tungsten.crypto;

namespace tungsten {


credential_storage_entry_type_t credential_storage_entry_t::type() const noexcept {
    return m_type;
}

std::string_view credential_storage_entry_t::name() const {
    const size_t n = std::min<size_t>(m_name_size, l_max_name_size);
    return std::string_view(reinterpret_cast<const char*>(m_credential_name), n);
}

std::string_view credential_storage_entry_t::purpose() const {
    const size_t n = std::min<size_t>(m_purpose_size, l_max_purpose_size);
    return std::string_view(reinterpret_cast<const char*>(m_credential_purpose), n);
}

size_t credential_storage_entry_t::credential_size() const noexcept {
    return m_credential_size;
}

void credential_storage_entry_t::unlock(credential_holder_t<uint8_t>& dest, credential_holder_t<uint8_t>& key) {
    if (m_name_size > l_max_name_size || m_purpose_size > l_max_purpose_size || m_credential_size > l_max_data_size) {
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
        if (m_credential_size > l_max_data_size) {
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

    std::vector<uint8_t, tungsten::secure_allocator_t<uint8_t>> plain;
    {
        aes256_gcm_t aes(key);
        plain = aes.decrypt(std::span<const uint8_t>(m_credential_data, l_max_data_size), aad);
        secure_zero(key.data(), key.size());
    }

    if (plain.size() < credential_size()) {
        secure_zero(plain);
        throw std::runtime_error("credential_storage_entry_t::unlock error: decrypted data too short");
    }
    std::copy_n(plain.begin(), credential_size(), dest.begin());
    secure_zero(plain);
}

credential_storage_entry_t credential_storage_entry_t::lock(
    credential_storage_entry_type_t type,
    credential_storage_entry_encryption_type_t encryption_type,
    std::string_view name,
    std::string_view purpose,
    credential_holder_t<uint8_t>& credential,
    credential_holder_t<uint8_t>& key)
{
    if (name.size() > l_max_name_size || purpose.size() > l_max_purpose_size) {
        secure_zero(credential.data(), credential.size());
        secure_zero(key.data(), key.size());
        throw std::runtime_error("credential_storage_entry_t::lock error: name/purpose too long");
    }
    const size_t cred_size = credential.size();

    const auto name_size    = static_cast<uint16_t>(name.size());
    const auto purpose_size = static_cast<uint16_t>(purpose.size());
    const auto cred_size16  = static_cast<uint16_t>(cred_size);

    auto aad = make_aad(type, encryption_type,
                        name_size, purpose_size, cred_size16,
                        reinterpret_cast<const uint8_t*>(name.data()),
                        reinterpret_cast<const uint8_t*>(purpose.data()));

    std::vector<uint8_t> secure_cred_data;

    if (encryption_type == credential_storage_entry_encryption_type_t::encrypted) {
        if (cred_size > l_max_pt) {
            secure_zero(credential.data(), credential.size());
            secure_zero(key.data(), key.size());
            throw std::runtime_error("credential_storage_entry_t::lock error: credential too large");
        }
        if (key.size() != 32) {
            secure_zero(credential.data(), credential.size());
            secure_zero(key.data(), key.size());
            throw std::runtime_error("credential_storage_entry_t::lock error: invalid key size");
        }

        std::vector<uint8_t> padded(l_max_pt, 0);
        try {
            std::copy_n(credential.begin(), cred_size, padded.begin());
            secure_zero(credential.data(), credential.size());

            aes256_gcm_t aes(key);
            auto encrypted = aes.encrypt(padded, aad);
            secure_zero(key.data(), key.size());
            secure_zero(padded);

            if (encrypted.size() != l_max_data_size) {
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
        if (cred_size > l_max_data_size) {
            secure_zero(credential.data(), credential.size());
            secure_zero(key.data(), key.size());
            throw std::runtime_error("credential_storage_entry_t::lock error: credential too large");
        }
        secure_cred_data.assign(l_max_data_size, 0);
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

}
