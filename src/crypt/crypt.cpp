#include "crypt/crypt.h"

#include <openssl/ec.h>
#include <openssl/ecdh.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <tuple>
#include <string>
#include <cassert>
#include <iomanip>
#include <sstream>

namespace logger {
namespace crypt {

// 生成ECDH密钥对的函数，返回一个包含私钥和公钥的元组
std::tuple<std::string, std::string> GenECDHKey() {
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!key || !EC_KEY_generate_key(key)) {
        EC_KEY_free(key);
        throw std::runtime_error("Failed to generate ECDH key pair");
    }

    // 获取私钥
    const BIGNUM *priv_key = EC_KEY_get0_private_key(key);
    std::string private_key(BN_num_bytes(priv_key), 0);
    BN_bn2bin(priv_key, reinterpret_cast<unsigned char*>(&private_key[0]));

    // 获取公钥
    const EC_POINT *pub_key = EC_KEY_get0_public_key(key);
    std::string public_key(EC_POINT_point2oct(EC_KEY_get0_group(key), pub_key, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, nullptr), 0);
    EC_POINT_point2oct(EC_KEY_get0_group(key), pub_key, POINT_CONVERSION_UNCOMPRESSED, reinterpret_cast<unsigned char*>(&public_key[0]), public_key.size(), nullptr);

    EC_KEY_free(key);
    return std::make_tuple(private_key, public_key);
}

// 根据客户端私钥和服务器公钥生成ECDH共享密钥的函数
std::string GenECDHSharedSecret(const std::string& client_pri, const std::string& server_pub) {
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!key) {
        throw std::runtime_error("Failed to create EC key");
    }

    // 设置私钥
    BIGNUM *priv_key = BN_bin2bn(reinterpret_cast<const unsigned char*>(client_pri.data()), client_pri.size(), nullptr);
    if (!EC_KEY_set_private_key(key, priv_key)) {
        BN_free(priv_key);
        EC_KEY_free(key);
        throw std::runtime_error("Failed to set private key");
    }
    BN_free(priv_key);

    // 设置公钥
    EC_POINT *pub_key = EC_POINT_new(EC_KEY_get0_group(key));
    if (!EC_POINT_oct2point(EC_KEY_get0_group(key), pub_key, reinterpret_cast<const unsigned char*>(server_pub.data()), server_pub.size(), nullptr)) {
        EC_POINT_free(pub_key);
        EC_KEY_free(key);
        throw std::runtime_error("Failed to set public key");
    }

    // 计算共享密钥
    int field_size = EC_GROUP_get_degree(EC_KEY_get0_group(key));
    std::string secret((field_size + 7) / 8, 0);
    int secret_len = ECDH_compute_key(reinterpret_cast<unsigned char*>(&secret[0]), secret.size(), pub_key, key, nullptr);
    if (secret_len <= 0) {
        EC_POINT_free(pub_key);
        EC_KEY_free(key);
        throw std::runtime_error("Failed to compute shared secret");
    }

    EC_POINT_free(pub_key);
    EC_KEY_free(key);
    return secret;
}

// 将二进制密钥转换为十六进制字符串的函数
std::string BinaryKeyToHex(const std::string& binary_key) {
    std::ostringstream oss;
    for (unsigned char byte : binary_key) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

// 将十六进制密钥转换为二进制字符串的函数
std::string HexKeyToBinary(const std::string& hex_key) {
    std::string binary_key;
    for (size_t i = 0; i < hex_key.length(); i += 2) {
        std::string byteString = hex_key.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoul(byteString, nullptr, 16));
        binary_key.push_back(byte);
    }
    return binary_key;
}

}  // namespace crypt
}  // namespace logger