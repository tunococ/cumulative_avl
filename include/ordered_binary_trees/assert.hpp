#pragma once

#ifndef ASSERT

#ifdef NDEBUG
#define ASSERT(x) 0
#else
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>

namespace ordered_binary_trees {

bool report_assertion_error(
    char const* file,
    int line,
    char const* function,
    char const* expression) {
  std::ostringstream stream;
  stream << "Assertion error -- "
      "file: " << file <<
      ", line: " << line <<
      ", function: " << function <<
      ", expression: " << expression << '\n';
  std::string str(stream.str());
  std::cerr << str << std::flush;
  throw std::logic_error(str);
}

} // namespace ordered_binary_trees

#define ASSERT(x) (x) || ordered_binary_trees::report_assertion_error(__FILE__, __LINE__, __func__, #x)
#endif

#endif
