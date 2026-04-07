#ifndef NEXUS_SOUP_BIN_TCP_DATA_TYPES_HPP
#define NEXUS_SOUP_BIN_TCP_DATA_TYPES_HPP
#include <cstdint>
#include <string>
#include <Beam/Pointers/Out.hpp>
#include <boost/lexical_cast.hpp>

namespace Nexus {

  /**
   * Parses a left-padded alpha-numeric field.
   * @param length The length of the field.
   * @param cursor The first character of the field.
   * @return A string containing the field.
   */
  inline std::string parse_left_padded_alpha_numeric(
      std::size_t length, Beam::Out<const char*> cursor) {
    auto token = *cursor;
    auto remaining_length = length;
    while(remaining_length > 0 && *token == ' ') {
      ++token;
      --remaining_length;
    }
    *cursor += length;
    return std::string(token, *cursor);
  }

  /**
   * Parses a left-padded numeric field.
   * @param length The length of the field.
   * @param cursor The first character of the field.
   * @return The field's numeric value.
   */
  template<typename T>
  T parse_left_padded_numeric(
      std::size_t length, Beam::Out<const char*> cursor) {
    auto value = parse_left_padded_alpha_numeric(length, Beam::out(cursor));
    return boost::lexical_cast<T>(value);
  }
}

#endif
