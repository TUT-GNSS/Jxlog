#include "sinks/console_sink.h"

#include "formatter/default_formatter.h"

namespace logger{

ConsoleSink::ConsoleSink() : formatter_(std::make_unique<DefaultFormatter>()) {}

void ConsoleSink::Log(const LogMsg& msg){
  std::cout << "ConsoleSink Log" << "\n";
}

void ConsoleSink::SetFormatter(std::unique_ptr<Formatter> formatter){
   std::cout << "ConsoleSink Log" << "\n"; 
}

} // namespace logger