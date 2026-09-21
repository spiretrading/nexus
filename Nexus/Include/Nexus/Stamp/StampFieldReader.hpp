#ifndef NEXUS_STAMP_FIELD_READER_HPP
#define NEXUS_STAMP_FIELD_READER_HPP
#include <algorithm>
#include <concepts>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/throw_exception.hpp>
#include "Nexus/Stamp/StampMessage.hpp"

namespace Nexus {

  /**
   * Reads typed fields from a STAMP message section.
   * Field values remain valid while the section's source buffer is unchanged.
   */
  class StampFieldReader {
    public:

      /**
       * Constructs a reader for a section, rejecting duplicate identifiers with
       * the same index.
       * @param section The fields to read.
       */
      explicit StampFieldReader(const StampMessage::Section& section);

      /**
       * Reads a required scalar field, rejecting nonzero indices.
       * @param identifier The field identifier.
       * @param parser Converts the field's text to a value.
       */
      template<typename F> requires std::invocable<F&, std::string_view> &&
        (!std::is_void_v<std::invoke_result_t<F&, std::string_view>>)
      auto read(std::uint16_t identifier, F parser) const;

      /**
       * Reads a required field at a specified index.
       * @param identifier The field identifier.
       * @param index The record index.
       * @param parser Converts the field's text to a value.
       */
      template<typename F> requires std::invocable<F&, std::string_view> &&
        (!std::is_void_v<std::invoke_result_t<F&, std::string_view>>)
      auto read(std::uint16_t identifier, std::uint16_t index, F parser) const;

      /**
       * Reads a scalar field if present, rejecting nonzero indices.
       * @param identifier The field identifier.
       * @param parser Converts the field's text to a value, including empty
       *   text. Not invoked when the field is absent.
       */
      template<typename F> requires std::invocable<F&, std::string_view> &&
        (!std::is_void_v<std::invoke_result_t<F&, std::string_view>>)
      auto read_optional(std::uint16_t identifier, F parser) const;

      /**
       * Reads a field at a specified index if present.
       * @param identifier The field identifier.
       * @param index The record index.
       * @param parser Converts the field's text to a value, including empty
       *        text. Not invoked when the field is absent.
       */
      template<typename F> requires std::invocable<F&, std::string_view> &&
        (!std::is_void_v<std::invoke_result_t<F&, std::string_view>>)
      auto read_optional(
        std::uint16_t identifier, std::uint16_t index, F parser) const;

      /**
       * Returns one past the highest index among the selected identifiers,
       * or zero if none are present. Missing intermediate indices are allowed.
       * @param identifiers The fields belonging to the records being counted.
       */
      std::uint16_t get_count(
        std::initializer_list<std::uint16_t> identifiers) const;

    private:
      std::vector<StampField> m_fields;
  };

  inline StampFieldReader::StampFieldReader(
      const StampMessage::Section& section) {
    for(auto& field : section) {
      m_fields.push_back(field);
    }
    auto key = [] (const auto& field) {
      return std::pair(field.m_identifier, field.m_index);
    };
    std::ranges::sort(m_fields, {}, key);
    if(std::ranges::adjacent_find(m_fields, {}, key) != m_fields.end()) {
      boost::throw_with_location(
        StampParserException("Duplicate STAMP field."));
    }
  }

  template<typename F> requires std::invocable<F&, std::string_view> &&
    (!std::is_void_v<std::invoke_result_t<F&, std::string_view>>)
  auto StampFieldReader::read(std::uint16_t identifier, F parser) const {
    if(get_count({identifier}) > 1) {
      boost::throw_with_location(
        StampParserException("Unexpected indexed STAMP field."));
    }
    return read(identifier, 0, parser);
  }

  template<typename F> requires std::invocable<F&, std::string_view> &&
    (!std::is_void_v<std::invoke_result_t<F&, std::string_view>>)
  auto StampFieldReader::read(
      std::uint16_t identifier, std::uint16_t index, F parser) const {
    if(auto value = read_optional(identifier, index, parser)) {
      return *value;
    }
    boost::throw_with_location(StampParserException("Missing STAMP field."));
  }

  template<typename F> requires std::invocable<F&, std::string_view> &&
    (!std::is_void_v<std::invoke_result_t<F&, std::string_view>>)
  auto StampFieldReader::read_optional(
      std::uint16_t identifier, F parser) const {
    if(get_count({identifier}) > 1) {
      boost::throw_with_location(
        StampParserException("Unexpected indexed STAMP field."));
    }
    return read_optional(identifier, 0, parser);
  }

  template<typename F> requires std::invocable<F&, std::string_view> &&
    (!std::is_void_v<std::invoke_result_t<F&, std::string_view>>)
  auto StampFieldReader::read_optional(
      std::uint16_t identifier, std::uint16_t index, F parser) const {
    using Value = std::invoke_result_t<F&, std::string_view>;
    auto key = std::pair(identifier, index);
    auto i = std::ranges::lower_bound(m_fields, key, {},
      [] (const auto& field) {
        return std::pair(field.m_identifier, field.m_index);
      });
    if(i == m_fields.end() || i->m_identifier != identifier ||
        i->m_index != index) {
      return boost::optional<Value>();
    }
    return boost::optional<Value>(parser(i->m_value));
  }

  inline std::uint16_t StampFieldReader::get_count(
      std::initializer_list<std::uint16_t> identifiers) const {
    auto count = std::uint16_t(0);
    for(auto identifier : identifiers) {
      auto i = std::ranges::upper_bound(m_fields, identifier, {},
        &StampField::m_identifier);
      if(i != m_fields.begin()) {
        --i;
        if(i->m_identifier == identifier) {
          count = std::max(count, std::uint16_t(i->m_index + 1));
        }
      }
    }
    return count;
  }
}

#endif
