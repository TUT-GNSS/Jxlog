#include <iostream>

#include <chrono>

#include "src/logger.h"
#include "src/sinks/console_sink.h"
#include "src/sinks/effective_sink.h"

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
  conf.dir = "./logger";
  conf.prefix = "loggerdemo";
  conf.pub_key = logger::crypt::BinaryKeyToHex(server_public_key);
  std::cout<<"pri key: "<<logger::crypt::BinaryKeyToHex(server_private_key).data()<<"\n";
  // private key FAA5BBE9017C96BF641D19D0144661885E831B5DDF52539EF1AB4790C05E665E
  {
    std::shared_ptr<logger::sink::Sink> effective_sink = std::make_shared<logger::sink::EffectiveSink>(conf);
    logger::Logger handle("test", {effective_sink});
    std::string str = GenerateRandomString(2000);

    auto begin = std::chrono::system_clock::now();
    for (int i = 0; i < 10; ++i) {
      handle.Log(logger::LogLevel::kInfo, logger::SourceLocation(), " test! Hello Jxlog!!!");
    }
    effective_sink->Flush();
    auto end = std::chrono::system_clock::now();
    std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    std::cout << "our logger diff: " << diff.count() << std::endl;
  }

  std::cout << "Logger Example End!" << std::endl;
  return 0;
}
