#include "formatter/formatter.h"

namespace logger{

class DefaultFormatter : public Formatter{
public:
  void Format(const LogMsg& msg, std::string& dest) override;
};

}// namespace logger