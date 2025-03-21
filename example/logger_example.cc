#include <iostream>

#include <chrono>

#include "logger/logger.h"
#include "logger/sinks/console_sink.h"
#include "logger/sinks/effective_sink.h"

std::string GenerateRandomString(int length) {
  std::string str;
  str.reserve(length);
  for (int i = 0; i < length; ++i) {
    str.push_back('a' + rand() % 26);
  }
  return str;
}
int main() {
  std::cout << "Logger Example Start!" << std::endl;
  auto [server_private_key, server_public_key] = logger::crypt::GenECDHKey();
  std::shared_ptr<logger::sink::Sink> sink = std::make_shared<logger::sink::ConsoleSink>();
  logger::sink::EffectiveSink::Conf conf;
  conf.dir = "./example";
  conf.prefix = "loggerdemo";
  conf.pub_key = logger::crypt::BinaryKeyToHex(server_public_key);
  std::cout << "pri key: " << logger::crypt::BinaryKeyToHex(server_private_key).data() << "\n";
  {
    std::shared_ptr<logger::sink::Sink> effective_sink = std::make_shared<logger::sink::EffectiveSink>(conf);
    logger::Logger handle({effective_sink});
    std::string str = GenerateRandomString(2000);

    auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < 10; ++i) {
      handle.Log(logger::LogLevel::kInfo, logger::SourceLocation(), " test! Hello Jxlog!!!");
    }
    handle.Flush();
    auto end = std::chrono::steady_clock::now();
    std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    std::cout << "our logger diff: " << diff.count() << std::endl;
  }

  std::cout << "Logger Example End!" << std::endl;
  return 0;
}
