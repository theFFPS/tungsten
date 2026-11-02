#pragma once

#include <tungsten/util/allocator.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <cstring>
#include <concepts>

namespace tungsten {

enum class credential_storage_entry_type_t : uint8_t {
    public_key, private_key, signature, api_key, generic_secret
};
enum class credential_storage_entry_encryption_type_t : uint8_t {
    encrypted, not_encrypted
};

template<typename T> requires std::integral<T> struct credential_holder_t {
private:
    std::vector<T, secure_allocator_t<uint8_t>> m_credential;
public:
    explicit credential_holder_t(size_t size) : m_credential(size) {}
    ~credential_holder_t() noexcept { cleanse(); }

    void cleanse() noexcept {
        if (!m_credential.empty()) memset_explicit(m_credential.data(), 0, m_credential.size() * sizeof(T));
    }

    credential_holder_t(const credential_holder_t&) = delete;
    credential_holder_t& operator=(const credential_holder_t&) = delete;
    credential_holder_t(credential_holder_t&&) = delete;
    credential_holder_t& operator=(credential_holder_t&&) = delete;

    size_t size() const noexcept { return m_credential.size(); }
    T*       data()       noexcept { return m_credential.data(); }
    const T* data() const noexcept { return m_credential.data(); }

    using iterator               = typename std::vector<T, secure_allocator_t<uint8_t>>::iterator;
    using const_iterator         = typename std::vector<T, secure_allocator_t<uint8_t>>::const_iterator;
    using reverse_iterator       = typename std::vector<T, secure_allocator_t<uint8_t>>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<T, secure_allocator_t<uint8_t>>::const_reverse_iterator;

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
    uint16_t m_name_size;
    uint16_t m_purpose_size;
    uint16_t m_credential_size;
    uint8_t  m_credential_name[128];
    uint8_t  m_credential_purpose[512];
    uint8_t  m_credential_data[32120];

    credential_storage_entry_t() = default;
    credential_storage_entry_t(const credential_storage_entry_t&) = default;
    credential_storage_entry_t& operator=(const credential_storage_entry_t&) = default;
    credential_storage_entry_t(credential_storage_entry_t&&) = default;
    credential_storage_entry_t& operator=(credential_storage_entry_t&&) = default;
    ~credential_storage_entry_t() noexcept {
        memset_explicit(this, 0, sizeof(*this));
    }

    credential_storage_entry_type_t type() const noexcept;
    std::string_view name() const;
    std::string_view purpose() const;
    size_t credential_size() const noexcept;

    void unlock(credential_holder_t<uint8_t>& dest, credential_holder_t<uint8_t>& key);

    static credential_storage_entry_t lock(
        credential_storage_entry_type_t type,
        credential_storage_entry_encryption_type_t encryption_type,
        std::string_view name,
        std::string_view purpose,
        credential_holder_t<uint8_t>& credential,
        credential_holder_t<uint8_t>& key
    );
} __attribute__((packed));

struct credential_storage_t {
    std::string m_credential_storage_name;
    std::vector<credential_storage_entry_t> m_credentials;
};
struct credential_storage_disk_t {
    uint32_t m_magic;
    uint8_t  m_reserved[4];
    uint32_t m_flags;
    uint32_t m_checksum;
    uint64_t m_credential_storage_name_size;
    uint64_t m_credential_storage_name_off;
    uint64_t m_credential_storage_credentials_count;
    uint64_t m_credential_storage_credentials_start;
    uint64_t m_credential_storage_data_size;
    uint64_t m_credential_storage_data_off;
} __attribute__((packed));

}
