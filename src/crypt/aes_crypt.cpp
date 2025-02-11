#include "crypt/aes_crypt.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <string>
#include <vector>
#include<iostream>
#include "helpers/internal_log.h"

namespace logger {
namespace crypt {

AESCrypt::AESCrypt(std::string key) : key_(std::move(key)) {
    if (key_.size() != 16 && key_.size() != 24 && key_.size() != 32) {
      LOG_ERROR("AES key must be 16, 24, or 32 bytes long");
    }
    // iv_ = GenerateIV(); // 生成一个随机的初始化向量
}

std::string AESCrypt::GenerateKey() {
    std::string key(16, 0); // 默认生成 128 位（16 字节）的密钥
    if (!RAND_bytes(reinterpret_cast<unsigned char*>(&key[0]), key.size())) {
      LOG_ERROR("Failed to generate AES key");
      return "";
    }
    return key;
}

std::string AESCrypt::GenerateIV() {
    std::string iv(16, 0); // AES 的 IV 固定为 16 字节
    if (!RAND_bytes(reinterpret_cast<unsigned char*>(&iv[0]), iv.size())) {
      LOG_ERROR("Failed to generate AES IV");
      return "";
    }
    return iv;
}

void AESCrypt::Encrypt(const void* input, size_t input_size, std::string& output) {

    // 生成随机IV（每次加密时独立生成）
    std::string iv = GenerateIV();

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
      LOG_ERROR("Failed to create EVP cipher context");
      return;
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
        LOG_ERROR("Invalid AES key size");
        return;
    }

    // 初始化加密操作
    if (!EVP_EncryptInit_ex(ctx, cipher, nullptr,
                                reinterpret_cast<const unsigned char*>(key_.data()),
                                reinterpret_cast<const unsigned char*>(iv.data()))) {
        EVP_CIPHER_CTX_free(ctx);
        LOG_ERROR("Failed to initialize encryption");
        return;
    }

    // 计算输出缓冲区大小
    int block_size = EVP_CIPHER_CTX_block_size(ctx);
    std::vector<unsigned char> output_buffer(input_size + block_size);
    int output_len = 0;

    // 执行加密
    if (!EVP_EncryptUpdate(ctx, output_buffer.data(), &output_len,
                               reinterpret_cast<const unsigned char*>(input), input_size)) {
        EVP_CIPHER_CTX_free(ctx);
        LOG_ERROR("Failed to encrypt data");
        return;
    }

    // 结束加密
    int final_len = 0;
    if (!EVP_EncryptFinal_ex(ctx, output_buffer.data() + output_len, &final_len)) {
        EVP_CIPHER_CTX_free(ctx);
        LOG_ERROR("Failed to finalize encryption");
        return;
    }
    output_len += final_len;

    // 输出格式：IV(16字节) + 密文
    output.clear();
    output.reserve(16 + output_len);
    output.append(iv.data(), iv.size());
    output.append(reinterpret_cast<char*>(output_buffer.data()), output_len);

    EVP_CIPHER_CTX_free(ctx);
}

std::string AESCrypt::Decrypt(const void* data, size_t size) {

    // 提取IV和密文
    if (size < 16) {
        LOG_ERROR("Ciphertext too short");
        return "";
    }
    const unsigned char* iv_ptr = static_cast<const unsigned char*>(data);
    std::string iv(reinterpret_cast<const char*>(iv_ptr), 16);
    const unsigned char* ciphertext = iv_ptr + 16;
    size_t ciphertext_size = size - 16;


    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        LOG_ERROR("Failed to create EVP cipher context");
        return "";
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
        LOG_ERROR("Invalid AES key size");
        return "";
    }

    // 初始化解密操作
    if (!EVP_DecryptInit_ex(ctx, cipher, nullptr,
                                reinterpret_cast<const unsigned char*>(key_.data()),
                                reinterpret_cast<const unsigned char*>(iv.data()))) {
        EVP_CIPHER_CTX_free(ctx);
        LOG_ERROR("Failed to initialize decryption");
        return "";
    }

    // 计算输出缓冲区大小
    std::vector<unsigned char> output_buffer(size);
    int output_len = 0;

    // 执行解密
    if (!EVP_DecryptUpdate(ctx, output_buffer.data(), &output_len,
                               ciphertext, ciphertext_size)) {
        EVP_CIPHER_CTX_free(ctx);
        LOG_ERROR("Failed to decrypt data");
        return "";
    }
    // 结束解密
    int final_len = 0;
    if (!EVP_DecryptFinal_ex(ctx, output_buffer.data() + output_len, &final_len)) {
        EVP_CIPHER_CTX_free(ctx);
        LOG_ERROR("Failed to finalize decryption");
        return "";
    }
    output_len += final_len;

    // 将解密结果存储到字符串中
    std::string result(reinterpret_cast<char*>(output_buffer.data()), output_len);

    EVP_CIPHER_CTX_free(ctx);
    return result;
}

}  // namespace crypt
}  // namespace logger