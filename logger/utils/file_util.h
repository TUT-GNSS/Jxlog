#pragma once

#include <stdint.h>
#include <filesystem>

namespace logger {
namespace filesystem {
size_t GetFileSize(const std::filesystem::path& file_path);
}  // namespace filesystem
}  // namespace logger