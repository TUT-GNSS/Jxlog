#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <streambuf>
#include <string>
#include <vector>

#include "decode_formatter.h"
#include "src/compress/zstd_compress.h"
#include "src/compress/zlib_compress.h"
#include "src/crypt/aes_crypt.h"
#include "src/helpers/internal_log.h"
#include "src/sinks/effective_sink.h"

using namespace logger;
using namespace logger::detail;

std::unique_ptr<DecodeFormatter> decode_formatter;

// 读取二进制文件到内存
std::vector<char> ReadFile(const std::string& file_path) {
  // 创建输入文件流对象，以二进制模式打开文件
  std::ifstream ifs(file_path, std::ios::binary);
  if (!ifs) {
    LOG_ERROR("ReadFile: open file failed");
    return {};
  }
  // 定位到文件末尾以获取文件大小 将文件指针移动到文件末尾
  // 第一个参数是偏移量（0表示不偏移）
  // 第二个参数是基准位置（std::ios::end表示以文件末尾为基准）
  ifs.seekg(0, std::ios::end);
  // 获取文件大小（字节数）
  // 返回当前文件指针的位置（即文件末尾的位置）
  auto file_size = ifs.tellg();
  // 重置文件指针到文件开头
  ifs.seekg(0, std::ios::beg);
  std::vector<char> buffer(file_size);
  ifs.read(buffer.data(), file_size);
  return buffer;
}

void AppendDataToFile(const std::string& file_path, const std::string& data) {
  std::ofstream ofs(file_path, std::ios::binary | std::ios::app);
  ofs.write(data.data(), data.size());
}

std::unique_ptr<logger::compress::Compression> decompress;

void DecodeItemData(char* data, size_t size, logger::crypt::Crypt* crypt, std::string& output) {
  std::string decrypted = crypt->Decrypt(data, size);
  std::string decompressed = decompress->Uncompress(decrypted.data(), decrypted.size());
  EffectiveMsg msg;
  msg.ParseFromString(decompressed);
  std::string assemble;
  decode_formatter->Format(msg, assemble);
  output.append(assemble);
}

void DecodeChunkData(char* data,
                     size_t size,
                     const std::string& cli_pub_key,
                     const std::string& svr_pri_key,
                     const std::string& iv,
                     std::string& output) {
  std::cout << "decode chunk :" << size << std::endl;
  // 服务器私钥
  std::string svr_pri_key_bin = crypt::HexKeyToBinary(svr_pri_key);
  // 计算共享密钥
  std::string shared_secret = crypt::GenECDHSharedSecret(svr_pri_key_bin, cli_pub_key);
  // 创建解压缩对象
  std::unique_ptr<crypt::AESCrypt> crypt = std::make_unique<crypt::AESCrypt>(shared_secret);
  // 设置IV
  crypt->SetIV(iv);

  size_t offset = 0;
  size_t count = 0;
  while (offset < size) {
    ++count;
    if (count % 1000 == 0) {
      std::cout << "decode item " << count << std::endl;
    }
    ItemHeader* item_header = reinterpret_cast<ItemHeader*>(data + offset);
    if (item_header->magic != ItemHeader::kMagic) {
      LOG_ERROR("DecodeChunkData: invalid item magic");
      return;
    }
    // 跳过ItemHeader
    offset += sizeof(ItemHeader);
    DecodeItemData(data + offset, item_header->size, crypt.get(), output);
    // 跳到下一个ItemHeader
    offset += item_header->size;
    output.push_back('\n');  // 尾部插入换行
  }
}

void DecodeFile(const std::string& input_file_path, const std::string& pri_key, const std::string& output_file_path) {
  // 取出文件中数据
  auto input = ReadFile(input_file_path);
  if (input.size() < sizeof(ChunkHeader)) {
    LOG_ERROR("DecodeFile: input file is too small");
    return;
  }
  // 得到文件头的chunk_header
  auto chunk_header = reinterpret_cast<ChunkHeader*>(input.data());
  // 判断魔数是否正确
  if (chunk_header->magic != ChunkHeader::kMagic) {
    LOG_ERROR("DecodeFile: invalid file magic");
    return;
  }
  size_t offset = 0;
  size_t file_size = input.size();
  std::string output;
  output.reserve(1024 * 1024);
  while (offset < file_size) {
    ChunkHeader* chunk_header = reinterpret_cast<ChunkHeader*>(input.data() + offset);
    if (chunk_header->magic != ChunkHeader::kMagic) {
      LOG_ERROR("DecodeFile: invalid chunk magic");
      return;
    }
    output.clear();
    // 跳至数据
    offset += sizeof(ChunkHeader);
    DecodeChunkData(input.data() + offset, chunk_header->size, std::string(chunk_header->pub_key, 65), pri_key, chunk_header->iv, output);
    // 跳至下一ChunkHeader
    offset += chunk_header->size;
    // 数据输出到文件
    AppendDataToFile(output_file_path, output);
  }
}

int main(int argc, char* argv[]) {
  // ./decode <file_path> <pri_key> <output_file>
  if (argc != 4) {
    std::cerr << "Usage: ./decode <file_path> <pri_key> <output_file>" << std::endl;
    return 1;
  }
  std::string input_file_path = argv[1];
  std::string pri_key = argv[2];
  std::string output_file_path = argv[3];

  decode_formatter = std::make_unique<DecodeFormatter>();
  decode_formatter->SetPattern("[%l][%D:%S][%p:%t][%F:%f:%#]%v");
  decompress = std::make_unique<logger::compress::ZstdCompression>();
  DecodeFile(input_file_path, pri_key, output_file_path);
  return 0;
}