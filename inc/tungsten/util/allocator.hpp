#pragma once

#include <openssl/crypto.h>
#include <stdexcept>
#include <new>
#include <cstddef>

namespace tungsten {

template <typename T> struct secure_allocator_t {
    using value_type = T;

    secure_allocator_t() noexcept = default;

    template <typename U>
    secure_allocator_t(const secure_allocator_t<U>&) noexcept {}

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
