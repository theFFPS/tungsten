#include <tungsten/crypto/ecdsa.hpp>
#include <openssl/param_build.h>
#include <openssl/ec.h>

namespace tungsten {

ecdsa_secp256k1_ctx_t& ecdsa_secp256k1_ctx_t::operator=(ecdsa_secp256k1_ctx_t&& other) noexcept {
    if (this != &other) {
        m_key = std::move(other.m_key);
        m_can_sign = other.m_can_sign;
        m_can_verify = other.m_can_verify;
        other.m_can_sign = other.m_can_verify = false;
    }
    return *this;
}

std::vector<uint8_t> ecdsa_secp256k1_ctx_t::generate() {
    pkey_ctx_t ctx (EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr));
    if (!ctx) throw std::runtime_error("ecdsa_secp256k1_ctx_t failed to create pkey_ctx_t");
    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) throw std::runtime_error("ecdsa_secp256k1_ctx_t failed to initialize keygen");
    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, const_cast<char*>(algo_name().c_str()), 0);
    params[1] = OSSL_PARAM_construct_end();
    if (EVP_PKEY_CTX_set_params(ctx.get(), params) <= 0) {
        throw std::runtime_error("ecdsa_secp256k1_ctx_t failed to set curve parameter");
    }
    EVP_PKEY* raw_key = nullptr;
    if (EVP_PKEY_generate(ctx.get(), &raw_key) <= 0) throw std::runtime_error("ecdsa_secp256k1_ctx_t failed to generate keyring");
    m_key.reset(raw_key);

    std::vector<uint8_t> pubkey = this->pubkey();
    m_can_sign = m_can_verify = true;
    return pubkey;
}

std::vector<uint8_t> ecdsa_secp256k1_ctx_t::pubkey() const {
    if (!m_key) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: key structure is empty");
    bio_t bio(BIO_new(BIO_s_mem()));
    if (!bio) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to create bio_t");
    if (i2d_PUBKEY_bio(bio.get(), m_key.get()) != 1) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to write public key to DER");

    char* data = nullptr;
    long len = BIO_get_mem_data(bio.get(), &data);
    if (len <= 0) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to export public key");

    const auto* udata = reinterpret_cast<const uint8_t*>(data);
    return std::vector<uint8_t>(udata, udata + len);
}

std::string ecdsa_secp256k1_ctx_t::pubpem() const {
    if (!m_key) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: key structure is empty");
    bio_t bio(BIO_new(BIO_s_mem()));
    if (!bio) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to create bio_t");
    if (PEM_write_bio_PUBKEY(bio.get(), m_key.get()) != 1) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to write public key to PEM");

    char* data = nullptr;
    long len = BIO_get_mem_data(bio.get(), &data);
    if (len <= 0) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to export public key");

    return std::string(data, static_cast<size_t>(len));
}

bool ecdsa_secp256k1_ctx_t::set_pubkey(std::span<const uint8_t> pubkey) {
    bio_t bio(BIO_new_mem_buf(pubkey.data(), static_cast<int>(pubkey.size())));
    if (!bio) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to create bio_t");

    EVP_PKEY* raw_key = nullptr;
    if (!d2i_PUBKEY_bio(bio.get(), &raw_key)) {
        m_can_verify = false;
        return false;
    }

    m_key.reset(raw_key);
    m_can_verify = (m_key != nullptr);
    return m_can_verify;
}

bool ecdsa_secp256k1_ctx_t::set_pubpem(std::string_view pubkey) {
    bio_t bio(BIO_new_mem_buf(pubkey.data(), static_cast<int>(pubkey.size())));
    if (!bio) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to create bio_t");

    m_key.reset(PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr));
    m_can_verify = (m_key != nullptr);
    return m_can_verify;
}

std::vector<uint8_t, secure_allocator_t<uint8_t>> ecdsa_secp256k1_ctx_t::privkey() const {
    if (!m_key) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: key structure is empty");
    bio_t bio(BIO_new(BIO_s_secmem()));
    if (!bio) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to create bio_t");
    if (i2d_PrivateKey_bio(bio.get(), m_key.get()) != 1) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to write private key to DER");

    char* data = nullptr;
    long len = BIO_get_mem_data(bio.get(), &data);
    if (len <= 0) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to export private key");

    const auto* udata = reinterpret_cast<const uint8_t*>(data);
    return std::vector<uint8_t, secure_allocator_t<uint8_t>>(udata, udata + len);
}

bool ecdsa_secp256k1_ctx_t::set_privkey(std::span<const uint8_t> privkey) {
    bio_t bio(BIO_new_mem_buf(privkey.data(), static_cast<int>(privkey.size())));
    if (!bio) throw std::runtime_error("ecdsa_secp256k1_ctx_t error: failed to create bio_t");

    EVP_PKEY* raw_key = nullptr;
    if (!d2i_PrivateKey_bio(bio.get(), &raw_key)) {
        m_can_sign = false;
        return false;
    }

    m_key.reset(raw_key);
    m_can_sign = (m_key != nullptr);
    return m_can_sign;
}

std::vector<uint8_t> ecdsa_secp256k1_ctx_t::sign(std::span<const uint8_t> data) {
    std::vector<uint8_t> signature (sigsize());
    if (!m_can_sign) throw std::runtime_error("ecdsa_secp256k1_ctx_t cannot sign without private key");
    hash_ctx_t ctx (EVP_MD_CTX_new());
    if (!ctx) throw std::runtime_error("ecdsa_secp256k1_ctx_t failed to create EVP_MD_CTX for signing");
    EVP_MD* md = EVP_MD_fetch(nullptr, m_hash.c_str(), nullptr);
    if (!md) throw std::runtime_error("ecdsa_secp256k1_ctx_t failed to fetch " + m_hash + " provider");
    if (EVP_DigestSignInit(ctx.get(), nullptr, md, nullptr, m_key.get()) != 1) {
        EVP_MD_free(md);
        throw std::runtime_error("ecdsa_secp256k1_ctx_t failed to initialize signer");
    }
    EVP_MD_free(md);
    size_t sig_len = sigsize();
    if (EVP_DigestSign(ctx.get(), signature.data(), &sig_len, data.data(), data.size()) != 1)
        throw std::runtime_error("ecdsa_secp256k1_ctx_t failed to sign");
    signature.resize(sig_len);
    return signature;
}

bool ecdsa_secp256k1_ctx_t::verify(std::span<const uint8_t> data, std::span<const uint8_t> signature) {
    if (!m_can_verify) throw std::runtime_error("ecdsa_secp256k1_ctx_t cannot verify without public key");
    hash_ctx_t ctx(EVP_MD_CTX_new());
    if (!ctx) return false;
    EVP_MD* md = EVP_MD_fetch(nullptr, m_hash.c_str(), nullptr);
    if (!md) return false;
    if (EVP_DigestVerifyInit(ctx.get(), nullptr, md, nullptr, m_key.get()) != 1) {
        EVP_MD_free(md);
        return false;
    }
    EVP_MD_free(md);
    int result = EVP_DigestVerify(ctx.get(), signature.data(), signature.size(), data.data(), data.size());
    if (result < 0) return false;
    return (result == 1);
}

bool ecdsa_secp256k1_ctx_t::can_sign() const { return m_can_sign; }
bool ecdsa_secp256k1_ctx_t::can_verify() const { return m_can_verify; }
std::string ecdsa_secp256k1_ctx_t::algo_name() const { return "secp256k1"; }
size_t ecdsa_secp256k1_ctx_t::sigsize() const { return 72; }

ecdsa_secp256r1_ctx_t& ecdsa_secp256r1_ctx_t::operator=(ecdsa_secp256r1_ctx_t&& other) noexcept {
    if (this != &other) {
        m_key = std::move(other.m_key);
        m_can_sign = other.m_can_sign;
        m_can_verify = other.m_can_verify;
        other.m_can_sign = other.m_can_verify = false;
    }
    return *this;
}

std::vector<uint8_t> ecdsa_secp256r1_ctx_t::generate() {
    pkey_ctx_t ctx (EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr));
    if (!ctx) throw std::runtime_error("ecdsa_secp256r1_ctx_t failed to create pkey_ctx_t");
    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) throw std::runtime_error("ecdsa_secp256r1_ctx_t failed to initialize keygen");
    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, const_cast<char*>(algo_name().c_str()), 0);
    params[1] = OSSL_PARAM_construct_end();
    if (EVP_PKEY_CTX_set_params(ctx.get(), params) <= 0) {
        throw std::runtime_error("ecdsa_secp256r1_ctx_t failed to set curve parameter");
    }
    EVP_PKEY* raw_key = nullptr;
    if (EVP_PKEY_generate(ctx.get(), &raw_key) <= 0) throw std::runtime_error("ecdsa_secp256r1_ctx_t failed to generate keyring");
    m_key.reset(raw_key);

    std::vector<uint8_t> pubkey = this->pubkey();
    m_can_sign = m_can_verify = true;
    return pubkey;
}

std::vector<uint8_t> ecdsa_secp256r1_ctx_t::pubkey() const {
    if (!m_key) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: key structure is empty");
    bio_t bio(BIO_new(BIO_s_mem()));
    if (!bio) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to create bio_t");
    if (i2d_PUBKEY_bio(bio.get(), m_key.get()) != 1) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to write public key to DER");

    char* data = nullptr;
    long len = BIO_get_mem_data(bio.get(), &data);
    if (len <= 0) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to export public key");

    const auto* udata = reinterpret_cast<const uint8_t*>(data);
    return std::vector<uint8_t>(udata, udata + len);
}

std::string ecdsa_secp256r1_ctx_t::pubpem() const {
    if (!m_key) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: key structure is empty");
    bio_t bio(BIO_new(BIO_s_mem()));
    if (!bio) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to create bio_t");
    if (PEM_write_bio_PUBKEY(bio.get(), m_key.get()) != 1) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to write public key to PEM");

    char* data = nullptr;
    long len = BIO_get_mem_data(bio.get(), &data);
    if (len <= 0) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to export public key");

    return std::string(data, static_cast<size_t>(len));
}

bool ecdsa_secp256r1_ctx_t::set_pubkey(std::span<const uint8_t> pubkey) {
    bio_t bio(BIO_new_mem_buf(pubkey.data(), static_cast<int>(pubkey.size())));
    if (!bio) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to create bio_t");

    EVP_PKEY* raw_key = nullptr;
    if (!d2i_PUBKEY_bio(bio.get(), &raw_key)) {
        m_can_verify = false;
        return false;
    }

    m_key.reset(raw_key);
    m_can_verify = (m_key != nullptr);
    return m_can_verify;
}

bool ecdsa_secp256r1_ctx_t::set_pubpem(std::string_view pubkey) {
    bio_t bio(BIO_new_mem_buf(pubkey.data(), static_cast<int>(pubkey.size())));
    if (!bio) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to create bio_t");

    m_key.reset(PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr));
    m_can_verify = (m_key != nullptr);
    return m_can_verify;
}

std::vector<uint8_t, secure_allocator_t<uint8_t>> ecdsa_secp256r1_ctx_t::privkey() const {
    if (!m_key) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: key structure is empty");
    bio_t bio(BIO_new(BIO_s_secmem()));
    if (!bio) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to create bio_t");
    if (i2d_PrivateKey_bio(bio.get(), m_key.get()) != 1) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to write private key to DER");

    char* data = nullptr;
    long len = BIO_get_mem_data(bio.get(), &data);
    if (len <= 0) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to export private key");

    const auto* udata = reinterpret_cast<const uint8_t*>(data);
    return std::vector<uint8_t, secure_allocator_t<uint8_t>>(udata, udata + len);
}

bool ecdsa_secp256r1_ctx_t::set_privkey(std::span<const uint8_t> privkey) {
    bio_t bio(BIO_new_mem_buf(privkey.data(), static_cast<int>(privkey.size())));
    if (!bio) throw std::runtime_error("ecdsa_secp256r1_ctx_t error: failed to create bio_t");

    EVP_PKEY* raw_key = nullptr;
    if (!d2i_PrivateKey_bio(bio.get(), &raw_key)) {
        m_can_sign = false;
        return false;
    }

    m_key.reset(raw_key);
    m_can_sign = (m_key != nullptr);
    return m_can_sign;
}

std::vector<uint8_t> ecdsa_secp256r1_ctx_t::sign(std::span<const uint8_t> data) {
    std::vector<uint8_t> signature (sigsize());
    if (!m_can_sign) throw std::runtime_error("ecdsa_secp256r1_ctx_t cannot sign without private key");
    hash_ctx_t ctx (EVP_MD_CTX_new());
    if (!ctx) throw std::runtime_error("ecdsa_secp256r1_ctx_t failed to create EVP_MD_CTX for signing");
    EVP_MD* md = EVP_MD_fetch(nullptr, m_hash.c_str(), nullptr); if (!md) throw std::runtime_error("ecdsa_secp256r1_ctx_t failed to fetch " + m_hash + " provider");
    if (EVP_DigestSignInit(ctx.get(), nullptr, md, nullptr, m_key.get()) != 1) {
        EVP_MD_free(md);
        throw std::runtime_error("ecdsa_secp256r1_ctx_t failed to initialize signer");
    }
    EVP_MD_free(md);
    size_t sig_len = sigsize();
    if (EVP_DigestSign(ctx.get(), signature.data(), &sig_len, data.data(), data.size()) != 1)
        throw std::runtime_error("ecdsa_secp256r1_ctx_t failed to sign");
    signature.resize(sig_len);
    return signature;
}

bool ecdsa_secp256r1_ctx_t::verify(std::span<const uint8_t> data, std::span<const uint8_t> signature) {
    if (!m_can_verify) throw std::runtime_error("ecdsa_secp256r1_ctx_t cannot verify without public key");
    hash_ctx_t ctx(EVP_MD_CTX_new());
    if (!ctx) return false;
    EVP_MD* md = EVP_MD_fetch(nullptr, m_hash.c_str(), nullptr);
    if (!md) return false;
    if (EVP_DigestVerifyInit(ctx.get(), nullptr, md, nullptr, m_key.get()) != 1) {
        EVP_MD_free(md);
        return false;
    }
    EVP_MD_free(md);
    int result = EVP_DigestVerify(ctx.get(), signature.data(), signature.size(), data.data(), data.size());
    if (result < 0) return false;
    return (result == 1);
}

bool ecdsa_secp256r1_ctx_t::can_sign() const { return m_can_sign; }
bool ecdsa_secp256r1_ctx_t::can_verify() const { return m_can_verify; }
std::string ecdsa_secp256r1_ctx_t::algo_name() const { return "prime256v1"; }
size_t ecdsa_secp256r1_ctx_t::sigsize() const { return 72; }

}
