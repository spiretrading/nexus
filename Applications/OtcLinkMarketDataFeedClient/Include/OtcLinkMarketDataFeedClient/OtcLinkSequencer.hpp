#ifndef OTC_LINK_SEQUENCER_HPP
#define OTC_LINK_SEQUENCER_HPP
#include <algorithm>
#include <deque>
#include <stdexcept>
#include <utility>
#include <vector>
#include <Beam/IO/SharedBuffer.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkPacket.hpp"

namespace Nexus {

  /** A range of channel message sequences missing from every active feed. */
  struct OtcLinkGap {

    /** The first missing message sequence. */
    std::uint64_t m_sequence;

    /** The number of missing messages. */
    std::uint64_t m_count;
  };

  /** Merges redundant live feeds by channel message sequence. */
  class OtcLinkSequencer {
    public:

      /**
       * Constructs an OtcLinkSequencer.
       * @param feeds The positive number of live feeds to merge.
       * @param feed_timeout How long a feed may be silent or stalled before it
       *        is excluded.
       */
      OtcLinkSequencer(
        int feeds, boost::posix_time::time_duration feed_timeout);

      /**
       * Adds a live packet. Test and reset packets are ignored; the caller
       * coordinates channel restarts using reset. Snapshot feeds must not be
       * merged with live feeds.
       * @param feed The index of the feed receiving the packet.
       * @param packet The received packet.
       * @param timestamp The time the packet was received.
       */
      void add(int feed, const OtcLinkPacket& packet,
        boost::posix_time::ptime timestamp);

      /**
       * Adds a live packet received before the current processing time.
       * @param feed The index of the feed receiving the packet.
       * @param packet The received packet.
       * @param received The time the packet was received.
       * @param timestamp The current time used to expire inactive feeds.
       */
      void add(int feed, const OtcLinkPacket& packet,
        boost::posix_time::ptime received, boost::posix_time::ptime timestamp);

      /** Adds recovered messages without advancing live-feed positions. */
      void recover(const OtcLinkPacket& packet);

      /** Adds one complete encoded recovery message in owned storage. */
      void recover(Beam::SharedBuffer message);

      /** Excludes silent or stalled feeds at the specified time. */
      void update(boost::posix_time::ptime timestamp);

      /** Returns the next complete encoded message in owned storage. */
      boost::optional<Beam::SharedBuffer> read();

      /** Returns the next expected channel message sequence. */
      boost::optional<std::uint64_t> get_sequence() const;

      /**
       * Returns the common position of active feeds, or the furthest known
       * position when none are active.
       */
      std::uint64_t get_position() const;

      /** Returns the next range of missing message sequences. */
      boost::optional<OtcLinkGap> get_gap() const;

      /** Skips a positive number of messages within the current gap. */
      void skip(std::uint64_t count);

      /** Resumes at the first buffered live message after a failed snapshot. */
      void resume();

      /** Resumes after a snapshot, preserving later buffered live messages. */
      void resume(std::uint64_t sequence);

      /** Discards channel state and waits for the first live message. */
      void reset();

      /**
       * Discards channel state and starts at a specified message sequence.
       * @param sequence The next expected message sequence.
       */
      void reset(std::uint32_t sequence);

    private:
      struct Entry {
        std::uint32_t m_sequence;
        Beam::SharedBuffer m_payload;
      };
      struct Feed {
        bool m_is_active;
        std::uint64_t m_position;
        boost::posix_time::ptime m_timestamp;
      };
      std::vector<Feed> m_feeds;
      boost::posix_time::time_duration m_feed_timeout;
      boost::optional<std::uint64_t> m_expected_sequence;
      std::deque<Entry> m_messages;

      static boost::optional<std::pair<std::uint32_t, std::uint64_t>>
        get_range(const OtcLinkPacket& packet);
      void store(const OtcLinkPacket& packet);
  };

  inline OtcLinkSequencer::OtcLinkSequencer(int feeds,
      boost::posix_time::time_duration feed_timeout)
      : m_feed_timeout(feed_timeout) {
    if(feeds <= 0 || feed_timeout.is_special() ||
        feed_timeout <= boost::posix_time::seconds(0)) {
      boost::throw_with_location(
        std::invalid_argument("Invalid OTC Link sequencer configuration."));
    }
    m_feeds.resize(feeds);
  }

  inline void OtcLinkSequencer::add(int feed, const OtcLinkPacket& packet,
      boost::posix_time::ptime timestamp) {
    add(feed, packet, timestamp, timestamp);
  }

  inline void OtcLinkSequencer::add(int feed, const OtcLinkPacket& packet,
      boost::posix_time::ptime received, boost::posix_time::ptime timestamp) {
    auto& header = packet.get_header();
    if(header.has_flag(OtcLinkHeader::Flag::TEST) ||
        header.has_flag(OtcLinkHeader::Flag::SEQUENCE_RESET)) {
      return;
    }
    if(header.has_flag(OtcLinkHeader::Flag::REPLAY)) {
      recover(packet);
      return;
    }
    auto range = get_range(packet);
    auto& source = m_feeds[feed];
    if(!range) {
      auto is_current_heartbeat = [&] {
        if(!header.has_flag(OtcLinkHeader::Flag::HEARTBEAT) ||
            source.m_position == 0) {
          return false;
        }
        return std::ranges::none_of(m_feeds, [&] (const auto& feed) {
          return feed.m_position > source.m_position;
        });
      };
      if(is_current_heartbeat()) {
        source.m_is_active = true;
        source.m_timestamp = received;
      }
      update(timestamp);
      return;
    }
    if(!m_expected_sequence) {
      m_expected_sequence = range->first;
    }
    if(range->second > source.m_position) {
      source.m_is_active = true;
      source.m_position = range->second;
      source.m_timestamp = received;
    }
    if(range->second > *m_expected_sequence) {
      store(packet);
    }
    update(timestamp);
  }

  inline void OtcLinkSequencer::recover(const OtcLinkPacket& packet) {
    auto& header = packet.get_header();
    if(!m_expected_sequence || header.has_flag(OtcLinkHeader::Flag::TEST) ||
        header.has_flag(OtcLinkHeader::Flag::SEQUENCE_RESET)) {
      return;
    }
    auto range = get_range(packet);
    if(range && range->second > *m_expected_sequence) {
      store(packet);
    }
  }

  inline void OtcLinkSequencer::recover(Beam::SharedBuffer message) {
    auto source = std::string_view(message.get_data(), message.get_size());
    auto parsed = OtcLinkMessage::parse(source);
    if(parsed.m_length != source.size()) {
      boost::throw_with_location(
        OtcLinkParserException("OTC Link recovery message length mismatch."));
    }
    auto sequence = parsed.get_cursor().read_uint32();
    if(!m_expected_sequence || sequence < *m_expected_sequence) {
      return;
    }
    auto i = std::ranges::lower_bound(
      m_messages, sequence, {}, &Entry::m_sequence);
    if(i == m_messages.end() || i->m_sequence != sequence) {
      m_messages.insert(i, Entry(sequence, std::move(message)));
    }
  }

  inline void OtcLinkSequencer::update(boost::posix_time::ptime timestamp) {
    for(auto& source : m_feeds) {
      if(source.m_is_active && (timestamp < source.m_timestamp ||
          timestamp - source.m_timestamp > m_feed_timeout)) {
        source.m_is_active = false;
      }
    }
  }

  inline boost::optional<Beam::SharedBuffer> OtcLinkSequencer::read() {
    if(m_messages.empty() ||
        m_messages.front().m_sequence != *m_expected_sequence) {
      return boost::none;
    }
    auto payload = std::move(m_messages.front().m_payload);
    m_messages.pop_front();
    ++*m_expected_sequence;
    return payload;
  }

  inline boost::optional<std::uint64_t> OtcLinkSequencer::get_sequence() const {
    return m_expected_sequence;
  }

  inline std::uint64_t OtcLinkSequencer::get_position() const {
    auto position = std::uint64_t(0);
    for(auto& source : m_feeds) {
      if(source.m_is_active &&
          (position == 0 || source.m_position < position)) {
        position = source.m_position;
      }
    }
    if(position == 0) {
      for(auto& source : m_feeds) {
        position = std::max(position, source.m_position);
      }
    }
    return position;
  }

  inline boost::optional<OtcLinkGap> OtcLinkSequencer::get_gap() const {
    if(!m_expected_sequence || (!m_messages.empty() &&
        m_messages.front().m_sequence == *m_expected_sequence)) {
      return boost::none;
    }
    auto end = get_position();
    if(!m_messages.empty()) {
      end = std::min(
        end, static_cast<std::uint64_t>(m_messages.front().m_sequence));
    }
    if(end <= *m_expected_sequence) {
      return boost::none;
    }
    return OtcLinkGap(*m_expected_sequence, end - *m_expected_sequence);
  }

  inline void OtcLinkSequencer::skip(std::uint64_t count) {
    auto gap = get_gap();
    if(!gap || count == 0 || count > gap->m_count) {
      boost::throw_with_location(
        std::invalid_argument("Invalid OTC Link gap skip."));
    }
    *m_expected_sequence += count;
  }

  inline void OtcLinkSequencer::resume() {
    if(m_messages.empty()) {
      m_expected_sequence = boost::none;
    } else {
      m_expected_sequence = m_messages.front().m_sequence;
    }
  }

  inline void OtcLinkSequencer::resume(std::uint64_t sequence) {
    if(sequence == 0 || sequence > std::uint64_t(UINT32_MAX) + 1) {
      boost::throw_with_location(
        std::invalid_argument("Invalid OTC Link snapshot sequence."));
    }
    m_expected_sequence = sequence;
    while(!m_messages.empty() && m_messages.front().m_sequence < sequence) {
      m_messages.pop_front();
    }
  }

  inline void OtcLinkSequencer::reset() {
    for(auto& source : m_feeds) {
      source = {};
    }
    m_messages.clear();
    m_expected_sequence = boost::none;
  }

  inline void OtcLinkSequencer::reset(std::uint32_t sequence) {
    reset();
    m_expected_sequence = sequence;
  }

  inline boost::optional<std::pair<std::uint32_t, std::uint64_t>>
      OtcLinkSequencer::get_range(const OtcLinkPacket& packet) {
    auto range = boost::optional<std::pair<std::uint32_t, std::uint64_t>>();
    for(auto& message : packet) {
      auto sequence = message.get_cursor().read_uint32();
      auto end = static_cast<std::uint64_t>(sequence) + 1;
      if(!range) {
        range.emplace(sequence, end);
      } else {
        range->first = std::min(range->first, sequence);
        range->second = std::max(range->second, end);
      }
    }
    return range;
  }

  inline void OtcLinkSequencer::store(const OtcLinkPacket& packet) {
    auto payload = Beam::SharedBuffer();
    auto offset = std::size_t(0);
    for(auto& message : packet) {
      auto sequence = message.get_cursor().read_uint32();
      if(sequence >= *m_expected_sequence) {
        auto i = [&] {
          if(m_messages.empty() || sequence > m_messages.back().m_sequence) {
            return m_messages.end();
          }
          return std::ranges::lower_bound(
            m_messages, sequence, {}, &Entry::m_sequence);
        }();
        if(i == m_messages.end() || i->m_sequence != sequence) {
          if(payload.get_size() == 0) {
            auto source = packet.get_payload();
            payload = Beam::SharedBuffer(source.data(), source.size());
          }
          m_messages.insert(
            i, Entry(sequence, payload.slice(offset, message.m_length)));
        }
      }
      offset += message.m_length;
    }
  }
}

#endif
