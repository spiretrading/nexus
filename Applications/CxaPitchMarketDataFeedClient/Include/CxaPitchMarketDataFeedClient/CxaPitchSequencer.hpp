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
#include "CxaPitchMarketDataFeedClient/CxaPitchMessages.hpp"
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

      /** The number of consecutive rewound blocks that signal a restart. */
      static constexpr auto REWIND_LIMIT = 2;

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
       * @return Whether the unit's sequence restarted, discarding all state.
       */
      bool add(int feed, const CxaPitchBlock& block,
        boost::posix_time::ptime timestamp);

      /**
       * Adds a block received from a gap response feed.
       * @param block The block that was received.
       */
      void recover(const CxaPitchBlock& block);

      /**
       * Advances the time used to determine whether a feed is silent.
       * @param timestamp The current time.
       */
      void update(boost::posix_time::ptime timestamp);

      /** Returns the payload of the next message in sequence. */
      boost::optional<Beam::SharedBuffer> read();

      /** Returns the sequence of the next message to return. */
      boost::optional<std::uint32_t> get_sequence() const;

      /** Returns the sequence that every feed has delivered through. */
      std::uint32_t get_position() const;

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
      std::uint32_t m_expected;
      int m_rewinds;
      bool m_is_initialized;

      void restart(std::uint32_t sequence);
      void expire(boost::posix_time::ptime timestamp);
      void store(const CxaPitchBlock& block);
  };

  inline CxaPitchSequencer::CxaPitchSequencer(
    int feeds, boost::posix_time::time_duration liveness)
    : m_liveness(liveness),
      m_feeds(feeds),
      m_expected(0),
      m_rewinds(0),
      m_is_initialized(false) {}

  inline bool CxaPitchSequencer::add(int feed, const CxaPitchBlock& block,
      boost::posix_time::ptime timestamp) {
    if(feed < 0 || feed >= static_cast<int>(m_feeds.size())) {
      boost::throw_with_location(
        CxaPitchParserException("Feed index out of range."));
    }
    validate(block);
    auto& source = m_feeds[feed];
    source.m_is_active = true;
    source.m_timestamp = timestamp;
    expire(timestamp);
    auto& header = block.get_header();
    if(header.m_sequence == 0) {
      return false;
    }
    if(!m_is_initialized) {
      m_expected = header.m_sequence;
      m_is_initialized = true;
    }
    auto is_restart = false;
    if(header.m_sequence < source.m_position) {
      ++m_rewinds;
      if(m_rewinds >= REWIND_LIMIT) {
        restart(header.m_sequence);
        is_restart = true;
      }
    } else {
      m_rewinds = 0;
    }
    auto position = header.m_sequence + header.m_count;
    if(position > source.m_position) {
      source.m_position = position;
    }
    store(block);
    return is_restart;
  }

  inline void CxaPitchSequencer::recover(const CxaPitchBlock& block) {
    if(!m_is_initialized || block.get_header().m_sequence == 0) {
      return;
    }
    validate(block);
    store(block);
  }

  inline void CxaPitchSequencer::update(boost::posix_time::ptime timestamp) {
    expire(timestamp);
  }

  inline boost::optional<Beam::SharedBuffer> CxaPitchSequencer::read() {
    if(!m_is_initialized) {
      return boost::none;
    }
    auto entry = m_messages.find(m_expected);
    if(entry == m_messages.end()) {
      return boost::none;
    }
    auto payload = std::move(entry->second);
    m_messages.erase(entry);
    ++m_expected;
    return payload;
  }

  inline boost::optional<std::uint32_t>
      CxaPitchSequencer::get_sequence() const {
    if(!m_is_initialized) {
      return boost::none;
    }
    return m_expected;
  }

  inline std::uint32_t CxaPitchSequencer::get_position() const {
    auto position = std::uint32_t(0);
    for(auto& source : m_feeds) {
      if(!source.m_is_active) {
        continue;
      }
      if(position == 0 || source.m_position < position) {
        position = source.m_position;
      }
    }
    return position;
  }

  inline boost::optional<CxaPitchGap> CxaPitchSequencer::get_gap() const {
    if(!m_is_initialized || m_messages.contains(m_expected)) {
      return boost::none;
    }
    auto end = get_position();
    if(end <= m_expected) {
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

  inline void CxaPitchSequencer::restart(std::uint32_t sequence) {
    m_messages.clear();
    for(auto& source : m_feeds) {
      source.m_position = 0;
    }
    m_expected = sequence;
    m_rewinds = 0;
    m_is_initialized = true;
  }

  inline void CxaPitchSequencer::expire(boost::posix_time::ptime timestamp) {
    for(auto& source : m_feeds) {
      if(source.m_is_active && timestamp - source.m_timestamp > m_liveness) {
        source.m_is_active = false;
      }
    }
  }

  inline void CxaPitchSequencer::store(const CxaPitchBlock& block) {
    auto sequence = block.get_header().m_sequence;
    for(auto& message : block) {
      if(sequence >= m_expected && !m_messages.contains(sequence)) {
        m_messages.emplace(sequence, Beam::SharedBuffer(
          message.m_payload - CxaPitchMessage::HEADER_LENGTH,
          message.m_length));
      }
      ++sequence;
    }
  }
}

#endif
