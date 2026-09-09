#ifndef CXA_PITCH_REPORT_HPP
#define CXA_PITCH_REPORT_HPP
#include <iostream>
#include <sstream>
#include <utility>

namespace Nexus {

  /**
   * Writes a report to the console as a single line.
   * @param f The callable writing the report to a stream.
   */
  template<typename F>
  void print(F&& f) {
    auto out = std::stringstream();
    std::forward<F>(f)(out);
    out << '\n';
    std::cout << out.str() << std::flush;
  }
}

#endif
