#ifndef CXA_PITCH_SEQUENCER_HPP
#define CXA_PITCH_SEQUENCER_HPP
#include <cstddef>
#include <cstdint>
#include <map>
#include <string_view>
#include <utility>
#include <vector>
#include <Beam/IO/SharedBuffer.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"

namespace Nexus {

  /** Stores a range of sequences that are missing from every feed. */
  struct CxaPitchGap {

    /** The first sequence that is missing. */
    std::uint32_t m_sequence;

    /** The number of sequences that are missing. */
    std::uint32_t m_count;
  };

  /** Merges the blocks received from a unit's feeds into one ordered stream. */
  class CxaPitchSequencer {
    public:

      /**
       * Constructs a CxaPitchSequencer.
       * @param feeds The number of feeds to merge.
       * @param liveness How long a feed may be silent before it is excluded.
       */
      CxaPitchSequencer(
        int feeds, boost::posix_time::time_duration liveness);

      /**
       * Adds a block received from a feed.
       * @param feed The index of the feed that received the block.
       * @param block The block that was received.
       * @param timestamp The time that the block was received.
       */
      void add(int feed, const CxaPitchBlock& block,
        boost::posix_time::ptime timestamp);

      /**
       * Advances the time used to determine whether a feed is silent.
       * @param timestamp The current time.
       */
      void update(boost::posix_time::ptime timestamp);

      /** Returns the next message in sequence. */
      boost::optional<CxaPitchMessage> read();

      /** Returns the range of sequences that are missing from every feed. */
      boost::optional<CxaPitchGap> get_gap() const;

      /**
       * Sets the sequence of the next message to return.
       * @param sequence The sequence to expect.
       */
      void reset(std::uint32_t sequence);

    private:
      struct Feed {
        std::uint32_t m_position;
        bool m_is_active;
        boost::posix_time::ptime m_timestamp;
      };
      boost::posix_time::time_duration m_liveness;
      std::vector<Feed> m_feeds;
      std::map<std::uint32_t, Beam::SharedBuffer> m_messages;
      Beam::SharedBuffer m_payload;
      std::uint32_t m_expected;
      bool m_is_initialized;

      void expire(boost::posix_time::ptime timestamp);
  };

  inline CxaPitchSequencer::CxaPitchSequencer(
    int feeds, boost::posix_time::time_duration liveness)
    : m_liveness(liveness),
      m_feeds(feeds),
      m_expected(0),
      m_is_initialized(false) {}

  inline void CxaPitchSequencer::add(int feed, const CxaPitchBlock& block,
      boost::posix_time::ptime timestamp) {
    if(feed < 0 || feed >= static_cast<int>(m_feeds.size())) {
      boost::throw_with_location(
        CxaPitchParserException("Feed index out of range."));
    }
    auto& source = m_feeds[feed];
    source.m_is_active = true;
    source.m_timestamp = timestamp;
    expire(timestamp);
    auto& header = block.get_header();
    if(header.m_sequence == 0) {
      return;
    }
    if(!m_is_initialized) {
      m_expected = header.m_sequence;
      m_is_initialized = true;
    }
    auto position = header.m_sequence + header.m_count;
    if(position > source.m_position) {
      source.m_position = position;
    }
    auto sequence = header.m_sequence;
    for(auto& message : block) {
      if(sequence >= m_expected && !m_messages.contains(sequence)) {
        m_messages.emplace(sequence, Beam::SharedBuffer(
          message.m_payload - CxaPitchMessage::HEADER_LENGTH,
          message.m_length));
      }
      ++sequence;
    }
  }

  inline void CxaPitchSequencer::update(boost::posix_time::ptime timestamp) {
    expire(timestamp);
  }

  inline boost::optional<CxaPitchMessage> CxaPitchSequencer::read() {
    if(!m_is_initialized) {
      return boost::none;
    }
    auto entry = m_messages.find(m_expected);
    if(entry == m_messages.end()) {
      return boost::none;
    }
    m_payload = std::move(entry->second);
    m_messages.erase(entry);
    ++m_expected;
    return CxaPitchMessage::parse(
      std::string_view(m_payload.get_data(), m_payload.get_size()));
  }

  inline boost::optional<CxaPitchGap> CxaPitchSequencer::get_gap() const {
    if(!m_is_initialized || m_messages.contains(m_expected)) {
      return boost::none;
    }
    auto end = std::uint32_t(0);
    for(auto& source : m_feeds) {
      if(!source.m_is_active) {
        continue;
      }
      if(source.m_position <= m_expected) {
        return boost::none;
      }
      if(end == 0 || source.m_position < end) {
        end = source.m_position;
      }
    }
    if(end == 0) {
      return boost::none;
    }
    if(!m_messages.empty() && m_messages.begin()->first < end) {
      end = m_messages.begin()->first;
    }
    auto gap = CxaPitchGap();
    gap.m_sequence = m_expected;
    gap.m_count = end - m_expected;
    return gap;
  }

  inline void CxaPitchSequencer::reset(std::uint32_t sequence) {
    m_messages.erase(m_messages.begin(), m_messages.lower_bound(sequence));
    m_expected = sequence;
    m_is_initialized = true;
  }

  inline void CxaPitchSequencer::expire(boost::posix_time::ptime timestamp) {
    for(auto& source : m_feeds) {
      if(source.m_is_active && timestamp - source.m_timestamp > m_liveness) {
        source.m_is_active = false;
      }
    }
  }
}

#endif
