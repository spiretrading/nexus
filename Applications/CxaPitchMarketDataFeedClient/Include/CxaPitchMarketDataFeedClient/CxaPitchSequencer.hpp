#ifndef CXA_PITCH_SEQUENCER_HPP
#define CXA_PITCH_SEQUENCER_HPP
#include <algorithm>
#include <deque>
#include <utility>
#include <vector>
#include <Beam/IO/SharedBuffer.hpp>
#include <boost/optional/optional.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchMessages.hpp"

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
       * @return Whether the feed's sequence restarted, discarding all state.
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
      struct Entry {
        std::uint32_t m_sequence;
        Beam::SharedBuffer m_payload;
      };
      struct Feed {
        bool m_is_active;
        std::uint32_t m_position;
        boost::posix_time::ptime m_timestamp;
      };
      struct Rewind {
        std::uint32_t m_sequence;
        std::deque<Entry> m_messages;
      };
      std::vector<Feed> m_feeds;
      boost::posix_time::time_duration m_feed_timeout;
      boost::optional<std::uint32_t> m_expected;
      std::deque<Entry> m_messages;
      boost::posix_time::ptime m_message_timestamp;
      boost::optional<Rewind> m_rewind;

      boost::optional<boost::posix_time::ptime> get_timestamp(
        const CxaPitchBlock& block) const;
      void restart(std::uint32_t sequence);
      void expire(boost::posix_time::ptime timestamp);
      void store(const CxaPitchBlock& block);
  };

  inline CxaPitchSequencer::CxaPitchSequencer(
    int feeds, boost::posix_time::time_duration feed_timeout)
    : m_feeds(feeds),
      m_feed_timeout(feed_timeout) {}

  inline bool CxaPitchSequencer::add(int feed, const CxaPitchBlock& block,
      boost::posix_time::ptime timestamp) {
    expire(timestamp);
    auto& header = block.get_header();
    if(header.m_sequence == 0) {
      return false;
    }
    if(!m_expected) {
      m_expected = header.m_sequence;
    }
    auto message_timestamp = get_timestamp(block);
    auto is_stale =
      message_timestamp && !m_message_timestamp.is_not_a_date_time() &&
      *message_timestamp <= m_message_timestamp;
    auto& source = m_feeds[feed];
    auto position = header.m_sequence + header.m_count;
    auto is_restart = false;
    if(header.m_sequence < source.m_position && !is_stale) {
      auto is_clear = false;
      for(auto& message : block) {
        if(message.m_type == CxaPitchUnitClear::TYPE) {
          is_clear = true;
          break;
        }
      }
      if(header.m_count != 0 && (message_timestamp || is_clear ||
          (m_rewind && header.m_sequence > m_rewind->m_sequence))) {
        if(m_rewind) {
          if(message_timestamp ||
              header.m_sequence > m_rewind->m_sequence) {
            restart(std::min(m_rewind->m_sequence, header.m_sequence));
            is_restart = true;
          }
        } else {
          m_rewind.emplace(header.m_sequence, std::deque<Entry>());
          auto sequence = header.m_sequence;
          for(auto& message : block) {
            m_rewind->m_messages.emplace_back(sequence, Beam::SharedBuffer(
              message.m_payload - CxaPitchMessage::HEADER_LENGTH,
              message.m_length));
            ++sequence;
          }
        }
      }
    } else {
      m_rewind = boost::none;
      if(position > source.m_position ||
          (header.m_count == 0 && position == source.m_position)) {
        source.m_is_active = true;
        source.m_timestamp = timestamp;
      }
      if(message_timestamp && (m_message_timestamp.is_not_a_date_time() ||
          *message_timestamp > m_message_timestamp)) {
        m_message_timestamp = *message_timestamp;
      }
    }
    source.m_position = std::max(source.m_position, position);
    store(block);
    return is_restart;
  }

  inline void CxaPitchSequencer::recover(const CxaPitchBlock& block) {
    if(!m_expected || block.get_header().m_sequence == 0) {
      return;
    }
    store(block);
  }

  inline void CxaPitchSequencer::update(boost::posix_time::ptime timestamp) {
    expire(timestamp);
  }

  inline boost::optional<Beam::SharedBuffer> CxaPitchSequencer::read() {
    if(m_messages.empty() || m_messages.front().m_sequence != m_expected) {
      return boost::none;
    }
    auto payload = std::move(m_messages.front().m_payload);
    m_messages.pop_front();
    ++*m_expected;
    return payload;
  }

  inline boost::optional<std::uint32_t>
      CxaPitchSequencer::get_sequence() const {
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
    if(!m_expected ||
        !m_messages.empty() && m_messages.front().m_sequence == m_expected) {
      return boost::none;
    }
    auto end = get_position();
    if(end <= *m_expected) {
      return boost::none;
    }
    if(!m_messages.empty() && m_messages.front().m_sequence < end) {
      end = m_messages.front().m_sequence;
    }
    return CxaPitchGap(*m_expected, end - *m_expected);
  }

  inline void CxaPitchSequencer::reset(std::uint32_t sequence) {
    while(!m_messages.empty() && m_messages.front().m_sequence < sequence) {
      m_messages.pop_front();
    }
    m_expected = sequence;
  }

  inline boost::optional<boost::posix_time::ptime>
      CxaPitchSequencer::get_timestamp(const CxaPitchBlock& block) const {
    for(auto& message : block) {
      auto timestamp = visit(message, [] (const auto& message) {
        auto timestamp = boost::optional<boost::posix_time::ptime>();
        if constexpr(requires { message.m_timestamp; }) {
          timestamp = message.m_timestamp;
        }
        return timestamp;
      });
      if(timestamp) {
        return timestamp;
      }
    }
    return boost::none;
  }

  inline void CxaPitchSequencer::restart(std::uint32_t sequence) {
    m_messages = std::move(m_rewind->m_messages);
    for(auto& source : m_feeds) {
      source.m_position = 0;
    }
    m_expected = sequence;
    m_rewind = boost::none;
  }

  inline void CxaPitchSequencer::expire(boost::posix_time::ptime timestamp) {
    for(auto& source : m_feeds) {
      if(source.m_is_active &&
          timestamp - source.m_timestamp > m_feed_timeout) {
        source.m_is_active = false;
      }
    }
  }

  inline void CxaPitchSequencer::store(const CxaPitchBlock& block) {
    auto sequence = block.get_header().m_sequence;
    for(auto& message : block) {
      if(sequence >= *m_expected) {
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
