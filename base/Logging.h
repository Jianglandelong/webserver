#pragma once
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <cstring>
#include <iostream>
#include "LogStream.h"


class AsyncLogging;

class Logger {
 public:
  Logger(const char *fileName, int line);
  ~Logger();
  LogStream &stream() { return impl_.stream_; }

  static void setLogFileName(std::string fileName) { logFileName_ = fileName; }
  static std::string getLogFileName() { return logFileName_; }

 private:
  class Impl {
   public:
    Impl(const char *fileName, int line);
    void formatTime();

    LogStream stream_;
    int line_;
    std::string basename_;
  };
  Impl impl_;
  static std::string logFileName_;
};

// #define LOG Logger(__FILE__, __LINE__).stream()
// #define LOG_INFO Logger(__FILE__, __LINE__).stream()
// #define LOG_TRACE Logger(__FILE__, __LINE__).stream()
// #define LOG_DEBUG Logger(__FILE__, __LINE__).stream()
// #define LOG_WARN Logger(__FILE__, __LINE__).stream()
// #define LOG_ERROR Logger(__FILE__, __LINE__).stream()
// #define LOG_FATAL Logger(__FILE__, __LINE__).stream()
// #define LOG_SYSERR Logger(__FILE__, __LINE__).stream()
// #define LOG_SYSFATAL Logger(__FILE__, __LINE__).stream()

// #define LOG std::cout
// #define LOG_INFO std::cout
// #define LOG_TRACE std::cout
// #define LOG_DEBUG std::cout
// #define LOG_WARN std::cout
// #define LOG_ERROR std::cerr
// #define LOG_FATAL std::cerr
// #define LOG_SYSERR std::cerr
// #define LOG_SYSFATAL std::cerr