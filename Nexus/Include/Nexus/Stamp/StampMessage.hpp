#ifndef NEXUS_STAMP_MESSAGE_HPP
#define NEXUS_STAMP_MESSAGE_HPP
#include <array>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Pointers/Out.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Stamp/StampMessage.hpp"
#include "Nexus/Stamp/StampPacket.hpp"

namespace Nexus {
namespace Details {
  inline const char* memstr(
      const char* source, std::size_t size, const char* pattern) {
    auto length = std::strlen(pattern);
    auto token = source;
    while(true) {
      auto result = static_cast<const char*>(
        std::memchr(token, *pattern, size - (token - source)));
      if(!result || std::memcmp(result, pattern, length) == 0) {
        return result;
      }
      token = result + 1;
    }
    return nullptr;
  }

  inline bool find_value(const char* source, std::size_t source_size, int index,
      int order, Beam::Out<const char*> value_start,
      Beam::Out<const char*> value_end) {
    if(!source) {
      return false;
    }
    auto field_identifier = std::array<char, 16>();
    auto length = [&] {
      if(order == -1) {
        return std::sprintf(field_identifier.data(), "\x1e%d=", index);
      }
      return std::sprintf(field_identifier.data(), "\x1e%d.%d=", index, order);
    }();
    auto field = memstr(source, source_size, field_identifier.data());
    if(!field) {
      return false;
    }
    *value_start = field + length;
    *value_end = std::strpbrk(*value_start, "\x1e\x03");
    if(!*value_end) {
      *value_end = source + source_size;
    }
    return true;
  }

  template<typename T>
  boost::optional<T> get_business_field_helper(
    int index, int order, const char* source, std::size_t source_size);

  template<>
  boost::optional<std::string> get_business_field_helper<std::string>(
      int index, int order, const char* source, std::size_t source_size) {
    auto value_start = static_cast<const char*>(nullptr);
    auto value_end = static_cast<const char*>(nullptr);
    if(!find_value(source, source_size, index, order, Beam::out(value_start),
        Beam::out(value_end))) {
      return boost::none;
    }
    return std::string(value_start, value_end - value_start);
  }

  template<>
  boost::optional<Money> get_business_field_helper<Money>(
      int index, int order, const char* source, std::size_t source_size) {
    auto value =
      get_business_field_helper<std::string>(index, order, source, source_size);
    if(!value) {
      return boost::none;
    }
    return try_parse_money(*value);
  }

  template<>
  boost::optional<Side> get_business_field_helper<Side>(
      int index, int order, const char* source, std::size_t source_size) {
    auto value =
      get_business_field_helper<std::string>(index, order, source, source_size);
    if(!value) {
      return boost::none;
    }
    if(*value == "Buy") {
      return Side(Side::BID);
    } else if(*value == "Sell") {
      return Side(Side::ASK);
    } else if(*value == "BuySide") {
      return Side(Side::BID);
    } else if(*value == "SellSide") {
      return Side(Side::ASK);
    } else if(*value == "NA") {
      return Side(Side::NONE);
    }
    return boost::none;
  }

  template<>
  boost::optional<boost::posix_time::ptime>
      get_business_field_helper<boost::posix_time::ptime>(
        int index, int order, const char* source, std::size_t source_size) {
    auto value =
      get_business_field_helper<std::string>(index, order, source, source_size);
    if(!value || value->size() < 16) {
      return boost::none;
    }
    auto y = boost::lexical_cast<int>(value->substr(0, 4));
    auto m = boost::lexical_cast<int>(value->substr(4, 2));
    auto d = boost::lexical_cast<int>(value->substr(6, 2));
    auto hr = boost::lexical_cast<int>(value->substr(8, 2));
    auto mn = boost::lexical_cast<int>(value->substr(10, 2));
    auto sec = boost::lexical_cast<int>(value->substr(12, 2));
    auto mill = boost::lexical_cast<int>(value->substr(14, 2));
    auto timestamp = boost::posix_time::ptime(
      boost::gregorian::date(static_cast<unsigned short>(y),
        static_cast<unsigned short>(m), static_cast<unsigned short>(d)),
      boost::posix_time::hours(hr) + boost::posix_time::minutes(mn) +
        boost::posix_time::seconds(sec) +
          boost::posix_time::milliseconds(10 * mill));
    return timestamp;
  }

  template<typename T>
  boost::optional<T> get_business_field_helper(
      int index, int order, const char* source, std::size_t source_size) {
    auto field =
      get_business_field_helper<std::string>(index, order, source, source_size);
    if(!field) {
      return boost::none;
    }
    return boost::lexical_cast<T>(*field);
  }
}

  /** Stores and provides access to the contents of a STAMP message. */
  class StampMessage {
    public:

      /**
       * Constructs a StampMessage.
       * @param header The message header.
       * @param data A pointer to the message's raw data.
       * @param size The size of the data.
       */
      StampMessage(
        const StampHeader& header, const char* data, std::size_t size) noexcept;

      /** Returns the message header. */
      const StampHeader& get_header() const;

      /** Returns the business content data. */
      const char* get_business_content_data() const;

      /** Returns the size of the business message. */
      std::size_t get_business_content_size() const;

      /**
       * Returns a field within the business content section of the message.
       * @param index The index of the field to retrieve.
       * @return The field with the specified index, or <i>nullptr</i> iff no
       *         such field exists.
       */
      template<typename T>
      boost::optional<T> get_business_field(int index) const;

      /**
       * Returns a field within the business content section of the message.
       * @param index The index of the field to retrieve.
       * @param order The order in which the field appears in the specified
       *        <i>index</i>.
       * @return The field with the specified index and order or <i>nullptr</i>
       *         iff no such field exists.
       */
      template<typename T>
      boost::optional<T> get_business_field(int index, int order) const;

    private:
      StampHeader m_header;
      const char* m_control_section;
      std::size_t m_control_section_size;
      const char* m_business_content;
      std::size_t m_business_content_size;
  };

  inline StampMessage::StampMessage(
      const StampHeader& header, const char* data, std::size_t size) noexcept
    : m_header(header) {
    auto token = data;
    m_control_section = token;
    m_business_content =
      static_cast<const char*>(std::memchr(m_control_section, '\x1c', size));
    if(m_business_content) {
      m_control_section_size =
        static_cast<std::size_t>(m_business_content - m_control_section);
      ++m_business_content;
      m_business_content_size = size - m_control_section_size - 1;
    } else {
      m_control_section_size = size;
      m_business_content_size = 0;
    }
  }

  inline const StampHeader& StampMessage::get_header() const {
    return m_header;
  }

  inline const char* StampMessage::get_business_content_data() const {
    return m_business_content;
  }

  inline std::size_t StampMessage::get_business_content_size() const {
    return m_business_content_size;
  }

  template<typename T>
  boost::optional<T> StampMessage::get_business_field(int index) const {
    return Details::get_business_field_helper<T>(
      index, -1, m_business_content, m_business_content_size);
  }

  template<typename T>
  boost::optional<T> StampMessage::get_business_field(
      int index, int order) const {
    return Details::get_business_field_helper<T>(
      index, order, m_business_content, m_business_content_size);
  }
}

#endif
