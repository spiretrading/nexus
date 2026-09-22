#ifndef NEXUS_STAMP_FIELD_HPP
#define NEXUS_STAMP_FIELD_HPP
#include <algorithm>
#include <charconv>
#include <cstdint>
#include <string_view>
#include <boost/throw_exception.hpp>
#include "Nexus/Stamp/StampParserException.hpp"

namespace Nexus {

  /** A field view, valid while its source buffer is unchanged. */
  struct StampField {

    /** The character introducing a field. */
    static constexpr auto SEPARATOR = '\x1e';

    /** Identifies the field's meaning. */
    std::uint16_t m_identifier;

    /** The repeating-group index, or zero when omitted. */
    std::uint16_t m_index;

    /** The field value, without applying any business-specific default. */
    std::string_view m_value;

    /**
     * Parses a complete field including its leading separator.
     * @param source The field to parse.
     */
    static StampField parse(std::string_view source);
  };

  inline StampField StampField::parse(std::string_view source) {
    if(source.empty() || source.front() != SEPARATOR) {
      boost::throw_with_location(
        StampParserException("Missing STAMP field separator."));
    }
    auto separator = source.find('=');
    if(separator == std::string_view::npos) {
      boost::throw_with_location(
        StampParserException("Missing STAMP field value separator."));
    }
    auto parse_number = [] (std::string_view source, std::uint16_t minimum) {
      constexpr auto MAXIMUM_DIGITS = 5;
      constexpr auto MAXIMUM_VALUE = 9999;
      if(source.empty() || source.size() > MAXIMUM_DIGITS) {
        boost::throw_with_location(
          StampParserException("Invalid STAMP field identifier or index."));
      }
      auto value = std::uint16_t();
      auto [end, error] =
        std::from_chars(source.data(), source.data() + source.size(), value);
      auto is_invalid = error != std::errc() ||
        end != source.data() + source.size() || value < minimum ||
        value > MAXIMUM_VALUE;
      if(is_invalid) {
        boost::throw_with_location(
          StampParserException("Invalid STAMP field identifier or index."));
      }
      return value;
    };
    auto identifier = source.substr(1, separator - 1);
    auto index_separator = identifier.find('.');
    auto field = StampField();
    field.m_identifier = parse_number(identifier.substr(0, index_separator), 1);
    if(index_separator != std::string_view::npos) {
      field.m_index = parse_number(identifier.substr(index_separator + 1), 0);
    }
    field.m_value = source.substr(separator + 1);
    auto has_invalid_character =
      std::ranges::any_of(field.m_value, [] (auto character) {
        return character < ' ' || character > '~';
      });
    if(has_invalid_character) {
      boost::throw_with_location(
        StampParserException("Invalid STAMP field value."));
    }
    return field;
  }

}

#endif
