#include "sinks/sink.h"

namespace logger {
namespace sink {

class ConsoleSink : public Sink {
 public:
  ConsoleSink();

  ~ConsoleSink() override = default;

  void Log(const LogMsg& msg) override;

  void SetFormatter(std::unique_ptr<formatter::Formatter> formatter) override;

 private:
  std::unique_ptr<formatter::Formatter> formatter_;
};

}  // namespace sink
}  // namespace logger