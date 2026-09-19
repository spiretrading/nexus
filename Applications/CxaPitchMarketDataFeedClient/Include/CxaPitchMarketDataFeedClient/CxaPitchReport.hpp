#ifndef CXA_PITCH_REPORT_HPP
#define CXA_PITCH_REPORT_HPP
#include <concepts>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <utility>

namespace Nexus {

  /**
   * Writes a report to the console followed by a newline.
   * @param writer The callable writing the report to a stream.
   */
  template<typename F> requires
    std::invocable<F, std::stringstream&> &&
      (!std::is_member_pointer_v<std::remove_cvref_t<F>>)
  void print(F&& writer) {
    auto stream = std::stringstream();
    std::forward<F>(writer)(stream);
    stream << '\n';
    std::cout << stream.view() << std::flush;
  }
}

#endif
