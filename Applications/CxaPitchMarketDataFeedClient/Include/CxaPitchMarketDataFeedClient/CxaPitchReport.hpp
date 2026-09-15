#ifndef CXA_PITCH_REPORT_HPP
#define CXA_PITCH_REPORT_HPP
#include <concepts>
#include <functional>
#include <iostream>
#include <sstream>
#include <utility>

namespace Nexus {

  /**
   * Writes a report to the console followed by a newline.
   * @param writer The callable writing the report to a stream.
   */
  void print(std::invocable<std::stringstream&> auto&& writer) {
    auto stream = std::stringstream();
    std::invoke(std::forward<decltype(writer)>(writer), stream);
    stream << '\n';
    std::cout << stream.view() << std::flush;
  }
}

#endif
