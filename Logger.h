#pragma once

#include <Logging.h>

#include <cstring>
#include <iostream>
#include <string>

// #define LOG std::cout
// #define LOG_INFO std::cout
// #define LOG_TRACE std::cout
// #define LOG_DEBUG std::cout
// #define LOG_WARN std::cout
// #define LOG_ERROR std::cerr
// #define LOG_FATAL std::cerr
// #define LOG_SYSERR std::cerr
// #define LOG_SYSFATAL std::cerr

#define LOG Logger(__FILE__, __LINE__).stream()
#define LOG_INFO Logger(__FILE__, __LINE__).stream()
#define LOG_TRACE Logger(__FILE__, __LINE__).stream()
#define LOG_DEBUG Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__).stream()