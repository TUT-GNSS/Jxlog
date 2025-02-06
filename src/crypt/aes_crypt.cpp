#include "crypt/aes_crypt.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace logger {
namespace crypt {

AESCrypt::AESCrypt(std::string key) : key_(std::move(key)) {
    if (key_.size() != 16 && key_.size() != 24 && key_.size() != 32) {
        throw std::invalid_argument("AES key must be 16, 24, or 32 bytes long");
    }
    iv_ = GenerateIV(); // 生成一个随机的初始化向量
}

std::string AESCrypt::GenerateKey() {
    std::string key(32, 0); // 默认生成 256 位（32 字节）的密钥
    if (!RAND_bytes(reinterpret_cast<unsigned char*>(&key[0]), key.size())) {
        throw std::runtime_error("Failed to generate AES key");
    }
    return key;
}

std::string AESCrypt::GenerateIV() {
    std::string iv(16, 0); // AES 的 IV 固定为 16 字节
    if (!RAND_bytes(reinterpret_cast<unsigned char*>(&iv[0]), iv.size())) {
        throw std::runtime_error("Failed to generate AES IV");
    }
    return iv;
}

void AESCrypt::Encrypt(const void* input, size_t input_size, std::string& output) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP cipher context");
    }

    // 根据密钥长度选择 AES 模式
    const EVP_CIPHER* cipher = nullptr;
    if (key_.size() == 16) {
        cipher = EVP_aes_128_cbc();
    } else if (key_.size() == 24) {
        cipher = EVP_aes_192_cbc();
    } else if (key_.size() == 32) {
        cipher = EVP_aes_256_cbc();
    } else {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Invalid AES key size");
    }

    // 初始化加密操作
    if (1 != EVP_EncryptInit_ex(ctx, cipher, nullptr,
                                reinterpret_cast<const unsigned char*>(key_.data()),
                                reinterpret_cast<const unsigned char*>(iv_.data()))) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }

    // 计算输出缓冲区大小
    int block_size = EVP_CIPHER_CTX_block_size(ctx);
    std::vector<unsigned char> output_buffer(input_size + block_size);
    int output_len = 0;

    // 执行加密
    if (1 != EVP_EncryptUpdate(ctx, output_buffer.data(), &output_len,
                               reinterpret_cast<const unsigned char*>(input), input_size)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to encrypt data");
    }

    // 结束加密
    int final_len = 0;
    if (1 != EVP_EncryptFinal_ex(ctx, output_buffer.data() + output_len, &final_len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize encryption");
    }
    output_len += final_len;

    // 将加密结果存储到 output 中
    output.assign(reinterpret_cast<char*>(output_buffer.data()), output_len);

    EVP_CIPHER_CTX_free(ctx);
}

std::string AESCrypt::Decrypt(const void* data, size_t size) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP cipher context");
    }

    // 根据密钥长度选择 AES 模式
    const EVP_CIPHER* cipher = nullptr;
    if (key_.size() == 16) {
        cipher = EVP_aes_128_cbc();
    } else if (key_.size() == 24) {
        cipher = EVP_aes_192_cbc();
    } else if (key_.size() == 32) {
        cipher = EVP_aes_256_cbc();
    } else {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Invalid AES key size");
    }

    // 初始化解密操作
    if (1 != EVP_DecryptInit_ex(ctx, cipher, nullptr,
                                reinterpret_cast<const unsigned char*>(key_.data()),
                                reinterpret_cast<const unsigned char*>(iv_.data()))) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }

    // 计算输出缓冲区大小
    std::vector<unsigned char> output_buffer(size);
    int output_len = 0;

    // 执行解密
    if (1 != EVP_DecryptUpdate(ctx, output_buffer.data(), &output_len,
                               reinterpret_cast<const unsigned char*>(data), size)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to decrypt data");
    }

    // 结束解密
    int final_len = 0;
    if (1 != EVP_DecryptFinal_ex(ctx, output_buffer.data() + output_len, &final_len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize decryption");
    }
    output_len += final_len;

    // 将解密结果存储到字符串中
    std::string result(reinterpret_cast<char*>(output_buffer.data()), output_len);

    EVP_CIPHER_CTX_free(ctx);
    return result;
}

}  // namespace crypt
}  // namespace logger