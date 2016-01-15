#ifndef NEXUS_STAMPMESSAGE_HPP
#define NEXUS_STAMPMESSAGE_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/StampProtocol/StampMessage.hpp"
#include "Nexus/StampProtocol/StampPacket.hpp"
#include "Nexus/StampProtocol/StampProtocol.hpp"

namespace Nexus {
namespace StampProtocol {
namespace Details {
  inline const char* memstr(const char* source, std::size_t size,
      const char* pattern) {
    std::size_t patternLength = std::strlen(pattern);
    const char* token = source;
    while(true) {
      const char* result = static_cast<const char*>(std::memchr(token, *pattern,
        size - (token - source)));
      if(result == nullptr ||
          std::memcmp(result, pattern, patternLength) == 0) {
        return result;
      }
      token = result + 1;
    }
    return nullptr;
  }

  inline bool FindValue(const char* source, std::size_t sourceSize, int index,
      int order, Beam::Out<const char*> valueStart,
      Beam::Out<const char*> valueEnd) {
    if(source == nullptr) {
      return false;
    }
    char fieldIdentifier[16];
    int length;
    if(order == -1) {
      length = std::sprintf(fieldIdentifier, "\x1e%d=", index);
    } else {
      length = std::sprintf(fieldIdentifier, "\x1e%d.%d=", index, order);
    }
    const char* field = memstr(source, sourceSize, fieldIdentifier);
    if(field == nullptr) {
      return false;
    }
    *valueStart = field + length;
    *valueEnd = std::strpbrk(*valueStart, "\x1e\x03");
    return true;
  }

  template<typename T>
  boost::optional<T> GetBusinessFieldHelper(int index, int order,
      const char* source, std::size_t sourceSize) {
    boost::optional<std::string> field = GetBusinessFieldHelper<std::string>(
      index, order, source, sourceSize);
    if(!field.is_initialized()) {
      return boost::optional<T>();
    }
    return boost::lexical_cast<T>(*field);
  }

  template<>
  boost::optional<std::string> GetBusinessFieldHelper<std::string>(int index,
      int order, const char* source, std::size_t sourceSize) {
    const char* valueStart;
    const char* valueEnd;
    if(!FindValue(source, sourceSize, index, order, Beam::Store(valueStart),
        Beam::Store(valueEnd))) {
      return boost::optional<std::string>();
    }
    return std::string(valueStart, valueEnd - valueStart);
  }

  template<>
  boost::optional<Money> GetBusinessFieldHelper<Money>(int index,
      int order, const char* source, std::size_t sourceSize) {
    boost::optional<std::string> value = GetBusinessFieldHelper<std::string>(
      index, order, source, sourceSize);
    if(!value.is_initialized()) {
      return boost::optional<Money>();
    }
    return Money::FromValue(*value);
  }

  template<>
  boost::optional<Side> GetBusinessFieldHelper<Side>(int index,
      int order, const char* source, std::size_t sourceSize) {
    boost::optional<std::string> value = GetBusinessFieldHelper<std::string>(
      index, order, source, sourceSize);
    if(!value.is_initialized()) {
      return boost::optional<Side>();
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
    return boost::optional<Side>();
  }

  template<>
  boost::optional<boost::posix_time::ptime> GetBusinessFieldHelper<
      boost::posix_time::ptime>(int index, int order, const char* source,
      std::size_t sourceSize) {
    boost::optional<std::string> value = GetBusinessFieldHelper<std::string>(
      index, order, source, sourceSize);
    if(!value.is_initialized() || value->size() < 16) {
      return boost::optional<boost::posix_time::ptime>();
    }
    int y = boost::lexical_cast<int>(value->substr(0, 4));
    int m = boost::lexical_cast<int>(value->substr(4, 2));
    int d = boost::lexical_cast<int>(value->substr(6, 2));
    int hr = boost::lexical_cast<int>(value->substr(8, 2));
    int mn = boost::lexical_cast<int>(value->substr(10, 2));
    int sec = boost::lexical_cast<int>(value->substr(12, 2));
    int mill = boost::lexical_cast<int>(value->substr(14, 2));
    boost::posix_time::ptime timestamp(boost::gregorian::date(y, m, d),
      boost::posix_time::hours(hr) + boost::posix_time::minutes(mn) +
      boost::posix_time::seconds(sec) +
      boost::posix_time::milliseconds(10 * mill));
    return timestamp;
  }
}

  /*! \class StampMessage
      \brief Stores and provides access to the contents of a STAMP message.
   */
  class StampMessage {
    public:

      //! Constructs a StampMessage.
      /*!
        \param header The message header.
        \param data A pointer to the message's raw data.
        \param size The size of the data.
      */
      StampMessage(const StampHeader& header, const char* data,
        std::size_t size);

      //! Returns the message header.
      const StampHeader& GetHeader() const;

      //! Returns the business content data.
      const char* GetBusinessContentData() const;

      //! Returns the size of the business message.
      std::size_t GetBusinessContentSize() const;

      //! Returns a field within the business content section of the message.
      /*!
        \param index The index of the field to retrieve.
        \return The field with the specified index, or <i>nullptr</i> iff no
                such field exists.
      */
      template<typename T>
      boost::optional<T> GetBusinessField(int index) const;

      //! Returns a field within the business content section of the message.
      /*!
        \param index The index of the field to retrieve.
        \param order The order in which the field appears in the specified
               <i>index</i>.
        \return The field with the specified index and order or <i>nullptr</i>
                iff no such field exists.
      */
      template<typename T>
      boost::optional<T> GetBusinessField(int index, int order) const;

    private:
      StampHeader m_header;
      const char* m_controlSection;
      std::size_t m_controlSectionSize;
      const char* m_businessContent;
      std::size_t m_businessContentSize;
  };

  inline StampMessage::StampMessage(const StampHeader& header, const char* data,
      std::size_t size)
      : m_header(header) {
    const char* token = data;
    m_controlSection = token;
    m_businessContent = static_cast<const char*>(std::memchr(m_controlSection,
      '\x1c', size));
    if(m_businessContent == nullptr) {
      m_controlSectionSize = size;
    } else {
      m_controlSectionSize = static_cast<std::size_t>(
        m_businessContent - m_controlSection);
      ++m_businessContent;
      m_businessContentSize = size - m_controlSectionSize;
    }
  }

  inline const StampHeader& StampMessage::GetHeader() const {
    return m_header;
  }

  inline const char* StampMessage::GetBusinessContentData() const {
    return m_businessContent;
  }

  inline std::size_t StampMessage::GetBusinessContentSize() const {
    return m_businessContentSize;
  }

  template<typename T>
  boost::optional<T> StampMessage::GetBusinessField(int index) const {
    return Details::GetBusinessFieldHelper<T>(index, -1, m_businessContent,
      m_businessContentSize);
  }

  template<typename T>
  boost::optional<T> StampMessage::GetBusinessField(int index,
      int order) const {
    return Details::GetBusinessFieldHelper<T>(index, order, m_businessContent,
      m_businessContentSize);
  }
}
}

#endif
