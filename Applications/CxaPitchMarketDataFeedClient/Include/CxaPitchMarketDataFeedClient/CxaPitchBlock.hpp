#ifndef CXA_PITCH_BLOCK_HPP
#define CXA_PITCH_BLOCK_HPP
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ostream>
#include <string_view>
#include <boost/throw_exception.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchCursor.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchHeader.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"

namespace Nexus {

  /** Stores a single CXA PITCH message. */
  struct CxaPitchMessage {

    /** The length of a message's length and type fields. */
    static constexpr auto HEADER_LENGTH = std::size_t(2);

    /** The length of this message, including its length and type fields. */
    std::uint8_t m_length;

    /** The type of this message. */
    std::uint8_t m_type;

    /** The first byte of this message's fields. */
    const char* m_payload;

    /**
     * Parses a CxaPitchMessage.
     * @param source The buffer to parse.
     * @return The CxaPitchMessage represented by the <i>source</i>.
     */
    static CxaPitchMessage parse(std::string_view source);

    /** Returns a cursor positioned at this message's first field. */
    CxaPitchCursor get_cursor() const;
  };

  /** Stores a sequenced unit header and the messages that follow it. */
  class CxaPitchBlock {
    public:

      /** Iterates over the messages within a block. */
      class Iterator {
        public:

          /** Returns the message at this position. */
          const CxaPitchMessage& operator *() const;

          /** Returns the message at this position. */
          const CxaPitchMessage* operator ->() const;

          /** Advances to the next message. */
          Iterator& operator ++();

          /** Returns whether every message has been iterated over. */
          bool operator ==(std::default_sentinel_t) const;

        private:
          friend class CxaPitchBlock;
          std::string_view m_source;
          std::uint8_t m_remaining;
          CxaPitchMessage m_message;

          Iterator(std::string_view source, std::uint8_t remaining);
      };

      /**
       * Parses a CxaPitchBlock.
       * @param source The buffer to parse.
       * @return The CxaPitchBlock represented by the <i>source</i>.
       */
      static CxaPitchBlock parse(std::string_view source);

      /** Returns this block's header. */
      const CxaPitchHeader& get_header() const;

      /** Returns an iterator to this block's first message. */
      Iterator begin() const;

      /** Returns the sentinel marking the end of this block's messages. */
      std::default_sentinel_t end() const;

    private:
      CxaPitchHeader m_header;
      std::string_view m_payload;
  };

  inline CxaPitchMessage CxaPitchMessage::parse(std::string_view source) {
    if(source.size() < HEADER_LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("PITCH message too short."));
    }
    auto cursor = CxaPitchCursor(source.data());
    auto message = CxaPitchMessage();
    message.m_length = cursor.read_uint8();
    message.m_type = cursor.read_uint8();
    if(message.m_length < HEADER_LENGTH || message.m_length > source.size()) {
      boost::throw_with_location(
        CxaPitchParserException("PITCH message length out of range."));
    }
    message.m_payload = source.data() + HEADER_LENGTH;
    return message;
  }

  inline CxaPitchCursor CxaPitchMessage::get_cursor() const {
    return CxaPitchCursor(m_payload);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const CxaPitchMessage& message) {
    static const auto DIGITS = std::string_view("0123456789ABCDEF");
    return out << "(unknown 0x" << DIGITS[message.m_type >> 4] <<
      DIGITS[message.m_type & 0x0F] << ' ' <<
      static_cast<int>(message.m_length) << ')';
  }

  inline CxaPitchBlock::Iterator::Iterator(
      std::string_view source, std::uint8_t remaining)
      : m_source(source),
        m_remaining(remaining),
        m_message() {
    if(m_remaining != 0) {
      m_message = CxaPitchMessage::parse(m_source);
    }
  }

  inline const CxaPitchMessage& CxaPitchBlock::Iterator::operator *() const {
    return m_message;
  }

  inline const CxaPitchMessage* CxaPitchBlock::Iterator::operator ->() const {
    return &m_message;
  }

  inline CxaPitchBlock::Iterator& CxaPitchBlock::Iterator::operator ++() {
    m_source.remove_prefix(m_message.m_length);
    --m_remaining;
    if(m_remaining != 0) {
      m_message = CxaPitchMessage::parse(m_source);
    }
    return *this;
  }

  inline bool CxaPitchBlock::Iterator::operator ==(
      std::default_sentinel_t) const {
    return m_remaining == 0;
  }

  inline CxaPitchBlock CxaPitchBlock::parse(std::string_view source) {
    auto block = CxaPitchBlock();
    block.m_header = CxaPitchHeader::parse(source);
    if(block.m_header.m_length < CxaPitchHeader::LENGTH ||
        block.m_header.m_length != source.size()) {
      boost::throw_with_location(
        CxaPitchParserException("Sequenced unit header length out of range."));
    }
    block.m_payload = source.substr(
      CxaPitchHeader::LENGTH, block.m_header.m_length - CxaPitchHeader::LENGTH);
    auto remaining = block.m_payload;
    for(auto i = 0; i != block.m_header.m_count; ++i) {
      auto message = CxaPitchMessage::parse(remaining);
      remaining.remove_prefix(message.m_length);
    }
    if(!remaining.empty()) {
      boost::throw_with_location(
        CxaPitchParserException("PITCH block message count mismatch."));
    }
    return block;
  }

  inline const CxaPitchHeader& CxaPitchBlock::get_header() const {
    return m_header;
  }

  inline CxaPitchBlock::Iterator CxaPitchBlock::begin() const {
    return Iterator(m_payload, m_header.m_count);
  }

  inline std::default_sentinel_t CxaPitchBlock::end() const {
    return std::default_sentinel;
  }
}

#endif
