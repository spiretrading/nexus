#ifndef CXA_PITCH_BLOCK_HPP
#define CXA_PITCH_BLOCK_HPP
#include <cstdint>
#include <iterator>
#include <string_view>
#include <boost/throw_exception.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchHeader.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchMessage.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"

namespace Nexus {

  /** Stores a sequenced unit header and the messages that follow it. */
  class CxaPitchBlock {
    public:

      /** Iterates over the messages within a block. */
      class Iterator {
        public:

          /** The message type produced by this iterator. */
          using value_type = CxaPitchMessage;

          /** The type representing a distance between iterator positions. */
          using difference_type = std::ptrdiff_t;

          /** The traversal supported by this iterator. */
          using iterator_concept = std::input_iterator_tag;

          const CxaPitchMessage& operator *() const;
          const CxaPitchMessage* operator ->() const;
          Iterator& operator ++();
          Iterator operator ++(int);
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

      Iterator begin() const;
      std::default_sentinel_t end() const;

    private:
      CxaPitchHeader m_header;
      std::string_view m_payload;
  };

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

  inline CxaPitchBlock::Iterator CxaPitchBlock::Iterator::operator ++(int) {
    auto previous = *this;
    ++*this;
    return previous;
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
