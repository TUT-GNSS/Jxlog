#pragma once

#include "crypt/crypt.h"

namespace logger {
namespace crypt {

class AESCrypt final : public Crypt {
 public:
  // 接受一个密钥字符串
  AESCrypt(std::string key);
  ~AESCrypt() override = default;

  // 用于生成一个新的AES密钥
  static std::string GenerateKey();

  // 用于生成一个新的初始化向量（IV）
  static std::string GenerateIV();

  // 加密数据
  void Encrypt(const void* input, size_t input_size,
               std::string& output) override;

  // 解密数据
  std::string Decrypt(const void* data, size_t size) override;

 private:
  // 存储AES密钥
  std::string key_;
  // 存储初始化向量（IV）
  std::string iv_;
};

}  // namespace crypt
}  // namespace logger