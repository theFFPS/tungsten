#include "tungsten/config/credential_storage.hpp"
#include "tungsten/crypto/kdf.hpp"
#include "tungsten/crypto/ecdsa.hpp"
#include <exception>
#include <iostream>
#include <tungsten/crypto/shared.hpp>
#include <sys/resource.h>

using namespace tungsten;

void stop() {
    OPENSSL_cleanup();
}

void disable_core_dumps() {
    struct rlimit rl;
    rl.rlim_cur = 0;
    rl.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rl);
}
int main() {
    disable_core_dumps();
    if (CRYPTO_secure_malloc_init(1024*1024, 16) != 1) {
        throw std::runtime_error("tungsten::main error: failed to initialize OpenSSL secure heap.");
    }

    try {
        credential_storage_entry_t entry;
        std::vector<uint8_t> pub_key, sig;
        std::vector<uint8_t> salt (16);
        random(salt.data(), 16);
        std::string test = "Hello World!";
        {
            ecdsa_secp256r1_ctx_t dsa ("SHA256");
            pub_key = dsa.generate();
            auto priv_key = dsa.privkey();

            credential_holder_t<uint8_t> holder (priv_key.size()), key_holder (32);
            std::copy(priv_key.begin(), priv_key.end(), holder.begin());
            memset_explicit(priv_key.data(), 0, priv_key.size());

            std::string pwd;
            std::getline(std::cin, pwd);
            auto key_kdf = argon2id(std::span<const uint8_t>((const uint8_t*) pwd.data(), pwd.size()), salt, 32, 256*1024*1024, 4);
            std::copy(key_kdf.begin(), key_kdf.end(), key_holder.begin());
            memset_explicit(key_kdf.data(), 0, key_kdf.size());
            memset_explicit(pwd.data(), 0, pwd.size());

            entry = credential_storage_entry_t::lock(
                credential_storage_entry_type_t::private_key,
                credential_storage_entry_encryption_type_t::encrypted,
                "test",
                "test",
                holder,
                key_holder
            );
        }

        ecdsa_secp256r1_ctx_t dsa ("SHA256"), dsa2 ("SHA256");
        dsa.set_pubkey(pub_key);
        dsa2.set_pubkey(pub_key);

        credential_holder_t<uint8_t> key_holder (32);

        std::string pwd;
        std::getline(std::cin, pwd);
        auto key_kdf = argon2id(std::span<const uint8_t>((const uint8_t*) pwd.data(), pwd.size()), salt, 32, 256*1024*1024, 4);
        std::copy(key_kdf.begin(), key_kdf.end(), key_holder.begin());
        memset_explicit(key_kdf.data(), 0, key_kdf.size());
        memset_explicit(pwd.data(), 0, pwd.size());

        credential_holder_t<uint8_t> key (entry.credential_size());
        entry.unlock(key, key_holder);
        dsa.set_privkey(key);

        sig = dsa.sign(std::span<const uint8_t>((const uint8_t*) test.data(), test.size()));
        std::cout << "verify: " << (int) dsa2.verify(std::span<const uint8_t>((const uint8_t*) test.data(), test.size()), sig) << "\n";
    } catch (std::exception& e) {
        std::cout << e.what() << "\n";
    }
    stop();
}
