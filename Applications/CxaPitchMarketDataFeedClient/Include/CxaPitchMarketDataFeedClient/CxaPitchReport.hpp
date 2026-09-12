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
   * @param f The callable writing the report to a stream.
   */
  template<std::invocable<std::stringstream&> F>
  void print(F&& f) {
    auto out = std::stringstream();
    std::invoke(std::forward<F>(f), out);
    out << '\n';
    std::cout << out.view() << std::flush;
  }
}

#endif
