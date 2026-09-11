#ifndef CXA_PITCH_SEQUENCER_HPP
#define CXA_PITCH_SEQUENCER_HPP
#include <algorithm>
#include <deque>
#include <utility>
#include <vector>
#include <Beam/IO/SharedBuffer.hpp>
#include <boost/optional/optional.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"

namespace Nexus {

  /** Stores a range of sequences that are missing from every feed. */
  struct CxaPitchGap {

    /** The first sequence that is missing. */
    std::uint32_t m_sequence;

    /** The number of sequences that are missing. */
    std::uint32_t m_count;
  };

  /** Merges the blocks received from a feed into one ordered stream. */
  class CxaPitchSequencer {
    public:

      /**
       * Constructs a CxaPitchSequencer.
       * @param feeds The number of feeds to merge.
       * @param feed_timeout How long a feed may be silent before it is
       *        excluded.
       */
      CxaPitchSequencer(
        int feeds, boost::posix_time::time_duration feed_timeout);

      /**
       * Adds a block received from a feed.
       * @param feed The index of the feed that received the block.
       * @param block The block that was received.
       * @param timestamp The time that the block was received.
       */
      void add(int feed, const CxaPitchBlock& block,
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
      struct Entry {
        std::uint32_t m_sequence;
        Beam::SharedBuffer m_payload;
      };
      struct Feed {
        bool m_is_active;
        std::uint32_t m_position;
        boost::posix_time::ptime m_timestamp;
      };
      std::vector<Feed> m_feeds;
      boost::posix_time::time_duration m_feed_timeout;
      boost::optional<std::uint32_t> m_expected_sequence;
      std::deque<Entry> m_messages;

      void store(const CxaPitchBlock& block);
  };

  inline CxaPitchSequencer::CxaPitchSequencer(
    int feeds, boost::posix_time::time_duration feed_timeout)
    : m_feeds(feeds),
      m_feed_timeout(feed_timeout) {}

  inline void CxaPitchSequencer::add(int feed, const CxaPitchBlock& block,
      boost::posix_time::ptime timestamp) {
    update(timestamp);
    auto& header = block.get_header();
    if(header.m_sequence == 0) {
      return;
    }
    if(!m_expected_sequence) {
      m_expected_sequence = header.m_sequence;
    }
    auto& source = m_feeds[feed];
    auto position = header.m_sequence + header.m_count;
    auto is_current_heartbeat = [&] {
      if(header.m_count != 0 || position != source.m_position) {
        return false;
      }
      return std::ranges::none_of(m_feeds, [&] (const auto& feed) {
        return feed.m_position > position;
      });
    };
    if(position > source.m_position || is_current_heartbeat()) {
      source.m_is_active = true;
      source.m_timestamp = timestamp;
    }
    source.m_position = std::max(source.m_position, position);
    store(block);
  }

  inline void CxaPitchSequencer::recover(const CxaPitchBlock& block) {
    if(!m_expected_sequence || block.get_header().m_sequence == 0) {
      return;
    }
    store(block);
  }

  inline void CxaPitchSequencer::update(boost::posix_time::ptime timestamp) {
    for(auto& source : m_feeds) {
      if(source.m_is_active &&
          timestamp - source.m_timestamp > m_feed_timeout) {
        source.m_is_active = false;
      }
    }
  }

  inline boost::optional<Beam::SharedBuffer> CxaPitchSequencer::read() {
    if(m_messages.empty() ||
        m_messages.front().m_sequence != m_expected_sequence) {
      return boost::none;
    }
    auto payload = std::move(m_messages.front().m_payload);
    m_messages.pop_front();
    ++*m_expected_sequence;
    return payload;
  }

  inline boost::optional<std::uint32_t>
      CxaPitchSequencer::get_sequence() const {
    return m_expected_sequence;
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
    if(!m_expected_sequence || !m_messages.empty() &&
        m_messages.front().m_sequence == m_expected_sequence) {
      return boost::none;
    }
    auto end = get_position();
    if(end <= *m_expected_sequence) {
      return boost::none;
    }
    if(!m_messages.empty() && m_messages.front().m_sequence < end) {
      end = m_messages.front().m_sequence;
    }
    return CxaPitchGap(*m_expected_sequence, end - *m_expected_sequence);
  }

  inline void CxaPitchSequencer::reset(std::uint32_t sequence) {
    while(!m_messages.empty() && m_messages.front().m_sequence < sequence) {
      m_messages.pop_front();
    }
    m_expected_sequence = sequence;
  }

  inline void CxaPitchSequencer::store(const CxaPitchBlock& block) {
    auto sequence = block.get_header().m_sequence;
    for(auto& message : block) {
      if(sequence >= *m_expected_sequence) {
        auto entry = [&] {
          if(m_messages.empty() || sequence > m_messages.back().m_sequence) {
            return m_messages.end();
          }
          return std::ranges::lower_bound(
            m_messages, sequence, {}, &Entry::m_sequence);
        }();
        if(entry == m_messages.end() || entry->m_sequence != sequence) {
          m_messages.insert(entry, Entry(sequence, Beam::SharedBuffer(
            message.m_payload - CxaPitchMessage::HEADER_LENGTH,
            message.m_length)));
        }
      }
      ++sequence;
    }
  }
}

#endif
