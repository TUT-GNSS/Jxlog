#include "crypt/crypt.h"

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

// 使用CryptoPP的byte类型
using CryptoPP::byte;

// 将二进制密钥转换为十六进制字符串
std::string BinaryKeyToHex(const std::string& binary_key) {
  std::string hex_key;
  CryptoPP::HexEncoder encoder;
  encoder.Attach(new CryptoPP::StringSink(hex_key)); // 将编码后的数据输出到字符串
  encoder.Put(reinterpret_cast<const byte*>(binary_key.data()), binary_key.size());
  encoder.MessageEnd();
  return hex_key;
}

// 将十六进制密钥转换为二进制字符串
std::string HexKeyToBinary(const std::string& hex_key) {
  std::string binary_key;
  CryptoPP::HexDecoder decoder;
  decoder.Attach(new CryptoPP::StringSink(binary_key)); // 将解码后的数据输出到字符串
  decoder.Put(reinterpret_cast<const byte*>(hex_key.data()), hex_key.size());
  decoder.MessageEnd();
  return binary_key;
}

// 生成ECDH密钥对
std::tuple<std::string, std::string> GenECDHKey() {
  CryptoPP::AutoSeededRandomPool rnd; // 自动种子随机数生成器
  // 使用secp256r1椭圆曲线
  CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CryptoPP::ASN1::secp256r1());
  CryptoPP::SecByteBlock priv(dh.PrivateKeyLength()), pub(dh.PublicKeyLength());
  dh.GenerateKeyPair(rnd, priv, pub); // 生成密钥对
  return std::make_tuple(std::string(reinterpret_cast<const char*>(priv.data()), priv.size()),
                         std::string(reinterpret_cast<const char*>(pub.data()), pub.size()));
}

// 根据客户端私钥和服务器公钥生成ECDH共享密钥
std::string GenECDHSharedSecret(const std::string& client_pri, const std::string& server_pub) {
  using CryptoPP::byte;
  CryptoPP::AutoSeededRandomPool rnd;
  CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CryptoPP::ASN1::secp256r1());
  CryptoPP::SecByteBlock shared(dh.AgreedValueLength());
  CryptoPP::SecByteBlock pub(reinterpret_cast<const byte*>(server_pub.data()), server_pub.size());
  CryptoPP::SecByteBlock priv(reinterpret_cast<const byte*>(client_pri.data()), client_pri.size());
  if (!dh.Agree(shared, priv, pub)) {
    throw std::runtime_error("Failed to reach shared secret");
  }
  return std::string(reinterpret_cast<const char*>(shared.data()), shared.size());
}

}  // namespace crypt
}  // namespace logger