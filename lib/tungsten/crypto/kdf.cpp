#include <tungsten/crypto/kdf.hpp>
#include <tungsten/crypto/shared.hpp>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/core_names.h>
#include <stdexcept>
#include <format>
#include <sodium.h>

namespace tungsten {

std::vector<uint8_t, secure_allocator_t<uint8_t>> hkdf_sha3_512(std::span<const uint8_t> data, std::span<const uint8_t> salt, std::span<const uint8_t> info, size_t olen) {
    if (data.empty()) throw std::runtime_error("tungsten::hkdf error: input data cannot be empty");
    if (!olen) throw std::runtime_error("tungsten::hkdf error: output length must be positive");
    std::vector<uint8_t, secure_allocator_t<uint8_t>> output (olen);
    size_t outlen = olen;
    if (olen > 255 * 64) throw std::runtime_error("tungsten::hkdf error: output length too large");
    pkey_ctx_t ctx (EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr));
    if (!ctx) throw std::runtime_error("tungsten::hkdf error: failed to create derive context");
    if (EVP_PKEY_derive_init(ctx.get()) <= 0) throw std::runtime_error("tungsten::hkdf error: failed to initialize derive context");
    if (EVP_PKEY_CTX_set_hkdf_md(ctx.get(), EVP_sha3_512()) <= 0) throw std::runtime_error("tungsten::hkdf error: failed to set hash function");
    if (EVP_PKEY_CTX_set1_hkdf_key(ctx.get(), data.data(), data.size()) <= 0)
        throw std::runtime_error("tungsten::hkdf error: failed to set input data");
    if (!salt.empty()) if (EVP_PKEY_CTX_set1_hkdf_salt(ctx.get(), salt.data(), salt.size()) <= 0)
        throw std::runtime_error("tungsten::hkdf error: failed to set salt");
    if (!info.empty()) if (EVP_PKEY_CTX_add1_hkdf_info(ctx.get(), info.data(), info.size()) <= 0)
        throw std::runtime_error("tungsten::hkdf error: failed to set info");
    if (EVP_PKEY_derive(ctx.get(), output.data(), &outlen) <= 0) throw std::runtime_error("hkdf failed to derive key");
    if (outlen != olen) throw std::runtime_error("tungsten::hkdf error: output length mismatch");
    return output;
}
std::vector<uint8_t, secure_allocator_t<uint8_t>> argon2id(std::span<const uint8_t> password, std::span<const uint8_t> salt, size_t olen, size_t memcost, uint32_t iter) {
    if (password.empty()) throw std::runtime_error("tungsten::sodium_argon2 error: password cannot be empty");

    if (salt.size() != crypto_pwhash_SALTBYTES) {
        throw std::runtime_error(std::format("tungsten::sodium_argon2 error: salt must be exactly {} bytes", crypto_pwhash_SALTBYTES));
    }
    if (!olen) throw std::runtime_error("tungsten::sodium_argon2 error: output length must be positive");

    std::vector<uint8_t, secure_allocator_t<uint8_t>> output(olen);

    int result = crypto_pwhash(
        output.data(), output.size(), reinterpret_cast<const char*>(password.data()),
        password.size(), salt.data(),
        iter, memcost,
        crypto_pwhash_ALG_ARGON2ID13
    );

    if (result != 0) {
        throw std::runtime_error("tungsten::sodium_argon2 error: derivation failed (likely out of memory)");
    }

    return output;
}

}
