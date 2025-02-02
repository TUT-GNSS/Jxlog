#include "crypt/aes_crypt.h"

#include "cryptopp/aes.h"
#include "cryptopp/base64.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/eccrypto.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"
#include "cryptopp/modes.h"
#include "cryptopp/oids.h"
#include "cryptopp/osrng.h"


namespace logger {
namespace crypt {

// detail命名空间，用于封装AES加密和解密的具体实现细节
namespace detail {
using CryptoPP::byte;

// 生成AES密钥，使用Crypto++库的AutoSeededRandomPool生成随机密钥
static std::string GenerateKey() {
  CryptoPP::AutoSeededRandomPool rnd;
  byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
  rnd.GenerateBlock(key, sizeof(key));
  // 将生成的密钥转换为十六进制字符串并返回
  return BinaryKeyToHex(std::string(reinterpret_cast<const char*>(key), sizeof(key)));
}

// 生成初始化向量（IV），使用Crypto++库的AutoSeededRandomPool生成随机IV
static std::string GenerateIV() {
  CryptoPP::AutoSeededRandomPool rnd;
  byte iv[CryptoPP::AES::BLOCKSIZE];
  rnd.GenerateBlock(iv, sizeof(iv));
  // 将生成的IV转换为十六进制字符串并返回
  return BinaryKeyToHex(std::string(reinterpret_cast<const char*>(iv), sizeof(iv)));
}

// 加密函数，使用AES和CBC模式进行加密
void Encrypt(const void* input, size_t input_size, std::string& output, const std::string& key, const std::string& iv) {
  // 创建AES加密对象
  CryptoPP::AES::Encryption aes_encryption(reinterpret_cast<const byte*>(key.data()), key.size());
  // 创建CBC模式加密对象
  CryptoPP::CBC_Mode_ExternalCipher::Encryption cbc_encryption(aes_encryption,
                                                               reinterpret_cast<const byte*>(iv.data()));
  // 创建加密流变换过滤器，并将加密后的数据输出到字符串
  CryptoPP::StreamTransformationFilter stf_encryptor(cbc_encryption, new CryptoPP::StringSink(output));
  stf_encryptor.Put(reinterpret_cast<const byte*>(input), input_size);
  stf_encryptor.MessageEnd();
}

// 解密函数，使用AES和CBC模式进行解密
static std::string Decrypt(const void* data, size_t size, const std::string& key, const std::string& iv) {
  std::string decryptedtext;
  // 创建AES解密对象
  CryptoPP::AES::Decryption aes_decryption(reinterpret_cast<const byte*>(key.data()), key.size());
  // 创建CBC模式解密对象
  CryptoPP::CBC_Mode_ExternalCipher::Decryption cbc_decryption(aes_decryption,
                                                               reinterpret_cast<const byte*>(iv.data()));

  // 创建解密流变换过滤器，并将解密后的数据输出到字符串
  CryptoPP::StreamTransformationFilter stf_decryptor(cbc_decryption, new CryptoPP::StringSink(decryptedtext));
  stf_decryptor.Put(reinterpret_cast<const byte*>(data), size);
  stf_decryptor.MessageEnd();
  return decryptedtext;
}
}  // namespace detail

// 初始化密钥和IV
AESCrypt::AESCrypt(std::string key) {
  key_ = std::move(key);
  iv_ = "dad0c0012340080a"; // 这里硬编码了一个IV，实际应用中应随机生成
}

void AESCrypt::Encrypt(const void* input, size_t input_size, std::string& output) {
  detail::Encrypt(input, input_size, output, key_, iv_);
}

std::string AESCrypt::Decrypt(const void* data, size_t size) {
  return detail::Decrypt(data, size, key_, iv_);
}

std::string AESCrypt::GenerateKey() {
  return detail::GenerateKey();
}

std::string AESCrypt::GenerateIV() {
  return detail::GenerateIV();
}

}  // namespace crypt
}  // namespace logger