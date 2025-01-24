#include "formatter/default_formatter.h"

namespace logger
{

void DefaultFormatter::Format(const LogMsg& msg, std::string* dest){
  std::cout << "DefaultFormatter Format" << '\n';
}

} // namespace logger


