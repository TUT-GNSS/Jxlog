#include "utils/file_util.h"

namespace logger {
namespace filesystem {
size_t GetFileSize(const std::filesystem::path& file_path) {
  if (std::filesystem::exists(file_path)) {
    return std::filesystem::file_size(file_path);
  }
}
}  // namespace filesystem
}  // namespace logger