#ifndef NEXUS_STAMP_MESSAGE_HPP
#define NEXUS_STAMP_MESSAGE_HPP
#include <iterator>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Stamp/StampField.hpp"

namespace Nexus {

  /** A STAMP message view, valid while its source buffer is unchanged. */
  struct StampMessage {

    /** The character introducing the control header. */
    static constexpr auto CONTROL_HEADER = '\x01';

    /** The character introducing the business content. */
    static constexpr auto BUSINESS_CONTENT = '\x1c';

    /** The optional character terminating a message. */
    static constexpr auto CONTROL_TRAILER = '\x1d';

    /** An iterable view of a message's fields within one section. */
    class Section {
      public:

        /** Iterates over the fields in their transmitted order. */
        class Iterator {
          public:

            /** The field type produced by this iterator. */
            using value_type = StampField;

            /** The type representing a distance between iterator positions. */
            using difference_type = std::ptrdiff_t;

            /** The traversal supported by this iterator. */
            using iterator_concept = std::input_iterator_tag;

            const StampField& operator *() const;
            const StampField* operator ->() const;
            Iterator& operator ++();
            Iterator operator ++(int);
            bool operator ==(std::default_sentinel_t) const;

          private:
            friend class Section;
            std::string_view m_source;
            StampField m_field;

            explicit Iterator(std::string_view source);
        };

        /** Returns the field with the specified identifier at index zero. */
        boost::optional<StampField> find(std::uint16_t identifier) const;

        /** Returns the field with the specified identifier and index. */
        boost::optional<StampField> find(
          std::uint16_t identifier, std::uint16_t index) const;

        Iterator begin() const;
        std::default_sentinel_t end() const;

      private:
        friend struct StampMessage;
        std::string_view m_source;

        explicit Section(std::string_view source);
    };

    /** The message's control-header fields. */
    Section m_control_header;

    /** The message's business-content fields. */
    Section m_business_content;

    /**
     * Parses and validates a complete STAMP message.
     * @param source The message, excluding the transport framing and header.
     */
    static StampMessage parse(std::string_view source);
  };

  inline const StampField& StampMessage::Section::Iterator::operator *() const {
    return m_field;
  }

  inline const StampField* StampMessage::Section::Iterator::operator ->()
      const {
    return &m_field;
  }

  inline StampMessage::Section::Iterator&
      StampMessage::Section::Iterator::operator ++() {
    auto size =
      m_field.m_value.data() - m_source.data() + m_field.m_value.size();
    m_source.remove_prefix(size);
    if(!m_source.empty()) {
      m_field = StampField::parse(
        m_source.substr(0, m_source.find(StampField::SEPARATOR, 1)));
    }
    return *this;
  }

  inline StampMessage::Section::Iterator
      StampMessage::Section::Iterator::operator ++(int) {
    auto previous = *this;
    ++*this;
    return previous;
  }

  inline bool StampMessage::Section::Iterator::operator ==(
      std::default_sentinel_t) const {
    return m_source.empty();
  }

  inline StampMessage::Section::Iterator::Iterator(std::string_view source)
    : m_source(source),
      m_field(StampField::parse(
        source.substr(0, source.find(StampField::SEPARATOR, 1)))) {}

  inline boost::optional<StampField> StampMessage::Section::find(
      std::uint16_t identifier) const {
    return find(identifier, 0);
  }

  inline boost::optional<StampField> StampMessage::Section::find(
      std::uint16_t identifier, std::uint16_t index) const {
    for(auto& field : *this) {
      if(field.m_identifier == identifier && field.m_index == index) {
        return field;
      }
    }
    return boost::none;
  }

  inline StampMessage::Section::Iterator StampMessage::Section::begin() const {
    return Iterator(m_source);
  }

  inline std::default_sentinel_t StampMessage::Section::end() const {
    return {};
  }

  inline StampMessage::Section::Section(std::string_view source)
      : m_source(source) {
    if(source.empty()) {
      boost::throw_with_location(StampParserException("Empty STAMP section."));
    }
    for(auto i = begin(); i != end(); ++i) {}
  }

  inline StampMessage StampMessage::parse(std::string_view source) {
    if(source.empty() || source.front() != CONTROL_HEADER) {
      boost::throw_with_location(
        StampParserException("Missing STAMP control header."));
    }
    source.remove_prefix(1);
    auto separator = source.find(BUSINESS_CONTENT);
    if(separator == std::string_view::npos) {
      boost::throw_with_location(
        StampParserException("Missing STAMP business content."));
    }
    if(source.back() == CONTROL_TRAILER) {
      source.remove_suffix(1);
    }
    return StampMessage(Section(source.substr(0, separator)),
      Section(source.substr(separator + 1)));
  }
}

#endif
