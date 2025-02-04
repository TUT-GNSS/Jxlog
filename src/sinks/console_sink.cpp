#include "formatter/default_formatter.h"
#include "sinks/console_sink.h"

namespace logger {
namespace sink {

ConsoleSink::ConsoleSink()
    : formatter_(std::make_unique<formatter::DefaultFormatter>()) {}

void ConsoleSink::Log(const LogMsg& msg) {
  std::cout << "ConsoleSink Log" << "\n";
  std::string str;
  formatter_->Format(msg, str);
  fmt::print("format:{}\n", str);
}

void ConsoleSink::SetFormatter(
    std::unique_ptr<formatter::Formatter> formatter) {
  std::cout << "ConsoleSink SetFormatter" << "\n";
}

}  // namespace sink
}  // namespace logger