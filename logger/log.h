#pragma once

#include "log_common.h"
#include "log_factory.h"
#include "logger.h"

#define EXT_LOGGER_INIT(log) logger::LogFactory::Instance().SetLogger(log)

#define LOGGER_CALL(log, level, ...)                                                                      \
  if (log) {                                                                                              \
    (log)->Log(logger::SourceLocation{__FILE__, __LINE__, static_cast<const char*>(__FUNCTION__)}, level, \
                  __VA_ARGS__);                                                                              \
  }

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_TRACE
#define LOG_LOGGER_TRACE(log, ...) LOGGER_CALL(log, logger::LogLevel::kTrace, __VA_ARGS__)
#define EXT_LOG_TRACE(...) LOG_LOGGER_TRACE(logger::LogFactory::Instance().GetLogger(), __VA_ARGS__)
#else
#define LOG_LOGGER_TRACE(log, ...) (void)0
#define EXT_LOG_TRACE(...) (void)0
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_DEBUG
#define LOG_LOGGER_DEBUG(log, ...) LOGGER_CALL(log, logger::LogLevel::kDebug, __VA_ARGS__)
#define EXT_LOG_DEBUG(...) LOG_LOGGER_DEBUG(logger::LogFactory::Instance().GetLogger(), __VA_ARGS__)
#else
#define LOG_LOGGER_DEBUG(log, ...) (void)0
#define EXT_LOG_DEBUG(...) (void)0
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_INFO
#define LOG_LOGGER_INFO(log, ...) LOGGER_CALL(log, logger::LogLevel::kInfo, __VA_ARGS__)
#define EXT_LOG_INFO(...) LOG_LOGGER_INFO(logger::LogFactory::Instance().GetLogger(), __VA_ARGS__)
#else
#define LOG_LOGGER_INFO(log, ...) (void)0
#define EXT_LOG_INFO(...) (void)0
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_WARN
#define LOG_LOGGER_WARN(log, ...) LOGGER_CALL(log, logger::LogLevel::kWarn, __VA_ARGS__)
#define EXT_LOG_WARN(...) LOG_LOGGER_WARN(logger::LogFactory::Instance().GetLogger(), __VA_ARGS__)
#else
#define LOG_LOGGER_WARN(log, ...) (void)0
#define EXT_LOG_WARN(...) (void)0
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_ERROR
#define LOG_LOGGER_ERROR(log, ...) LOGGER_CALL(log, logger::LogLevel::kError, __VA_ARGS__)
#define EXT_LOG_ERROR(...) LOG_LOGGER_ERROR(logger::LogFactory::Instance().GetLogger(), __VA_ARGS__)
#else
#define LOG_LOGGER_ERROR(log, ...) (void)0
#define EXT_LOG_ERROR(...) (void)0
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_CRITICAL
#define LOG_LOGGER_CRITICAL(log, ...) LOGGER_CALL(log, logger::LogLevel::kFatal, __VA_ARGS__)
#define EXT_LOG_CRITICAL(...) LOG_LOGGER_CRITICAL(logger::LogFactory::Instance().GetLogger(), __VA_ARGS__)
#else
#define LOG_LOGGER_CRITICAL(log, ...) (void)0
#define EXT_LOG_CRITICAL(...) (void)0
#endif
