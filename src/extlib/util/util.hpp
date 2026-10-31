#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <cstdarg>
#include <sstream>
#include <filesystem>

#include "lib_recomp.hpp"

namespace fs = std::filesystem;

// Macros
std::vector<std::string> split_string(const std::string& s, const std::string& delims);
long long get_modified_time_u64(fs::path path);

#endif