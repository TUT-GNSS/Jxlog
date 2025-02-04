#pragma once

#include <memory>
#include <string>
#include <tuple>

namespace logger {
namespace crypt {

// 生成ECDH密钥对的函数，返回一个包含私钥和公钥的元组
std::tuple<std::string, std::string> GenECDHKey();

// 根据客户端私钥和服务器公钥生成ECDH共享密钥的函数
std::string GenECDHSharedSecret(const std::string& client_pri,
                                const std::string& server_pub);

// 将二进制密钥转换为十六进制字符串的函数
std::string BinaryKeyToHex(const std::string& binary_key);

// 将十六进制密钥转换为二进制字符串的函数
std::string HexKeyToBinary(const std::string& hex_key);

// Crypt加密类
class Crypt {
 public:
  virtual ~Crypt() = default;

  // 用于加密数据，输入为数据指针和大小，输出为加密后的字符串
  virtual void Encrypt(const void* input, size_t input_size,
                       std::string& output) = 0;

  // 用于解密数据，输入为数据指针和大小，返回解密后的字符串
  virtual std::string Decrypt(const void* data, size_t size) = 0;
};

}  // namespace crypt
}  // namespace logger