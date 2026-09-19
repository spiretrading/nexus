#ifndef ASX_TRADE_ITCH_SEQUENCER_HPP
#define ASX_TRADE_ITCH_SEQUENCER_HPP
#include <algorithm>
#include <deque>
#include <limits>
#include <utility>
#include <vector>
#include <Beam/IO/SharedBuffer.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchParserException.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Packet.hpp"

namespace Nexus {

  /** Stores a range of sequences missing from every feed. */
  struct AsxTradeItchGap {

    /** The first missing sequence. */
    std::uint64_t m_sequence;

    /** The number of missing sequences. */
    std::uint64_t m_count;
  };

  /** Merges redundant feeds for one partition into an ordered stream. */
  class AsxTradeItchSequencer {
    public:

      /** Identifies the MoldUDP64 session being sequenced. */
      using Session = Beam::FixedString<MoldUdp64Packet::SESSION_FIELD_LENGTH>;

      /**
       * Constructs an AsxTradeItchSequencer.
       * @param feeds The number of feeds to merge.
       * @param feed_timeout How long a feed may be silent or stalled before
       *        it is excluded.
       */
      AsxTradeItchSequencer(
        int feeds, boost::posix_time::time_duration feed_timeout);

      /**
       * Adds a packet from a live feed. The first packet selects the session;
       * packets from other sessions are ignored until an explicit reset.
       * @param feed The index of the feed receiving the packet.
       * @param packet The received packet.
       * @param timestamp The local time the packet was received.
       */
      void add(int feed, const MoldUdp64Packet& packet,
        boost::posix_time::ptime timestamp);

      /**
       * Adds recovered messages belonging to the selected session.
       * @param packet The retransmitted packet.
       */
      void recover(const MoldUdp64Packet& packet);

      /**
       * Advances the time used to exclude silent or stalled feeds.
       * @param timestamp The current local time.
       */
      void update(boost::posix_time::ptime timestamp);

      /** Returns the next owned message payload without its length prefix. */
      boost::optional<Beam::SharedBuffer> read();

      /** Returns the selected session, if one has been established. */
      const boost::optional<Session>& get_session() const;

      /** Returns the sequence of the next message to return. */
      boost::optional<std::uint64_t> get_sequence() const;

      /**
       * Returns the common position of active feeds, or the furthest known
       * position when none are active.
       */
      std::uint64_t get_position() const;

      /** Returns the range of sequences missing from every feed. */
      boost::optional<AsxTradeItchGap> get_gap() const;

      /** Returns whether the advertised end of the session was reached. */
      bool is_end_of_session() const;

      /**
       * Sets the next sequence within the selected session.
       * @param sequence The sequence to expect.
       */
      void reset(std::uint64_t sequence);

      /**
       * Starts sequencing a session, discarding the previous session's state.
       * @param session The session to select.
       * @param sequence The sequence to expect.
       */
      void reset(const Session& session, std::uint64_t sequence);

    private:
      struct Entry {
        std::uint64_t m_sequence;
        Beam::SharedBuffer m_payload;
      };
      struct Feed {
        bool m_is_active;
        std::uint64_t m_position;
        boost::posix_time::ptime m_timestamp;
      };
      std::vector<Feed> m_feeds;
      boost::posix_time::time_duration m_feed_timeout;
      boost::optional<Session> m_session;
      boost::optional<std::uint64_t> m_expected_sequence;
      boost::optional<std::uint64_t> m_end_sequence;
      std::deque<Entry> m_messages;

      static std::uint64_t get_position(const MoldUdp64Packet& packet);
      void store(const MoldUdp64Packet& packet, std::uint64_t position);
  };

  inline AsxTradeItchSequencer::AsxTradeItchSequencer(
    int feeds, boost::posix_time::time_duration feed_timeout)
    : m_feeds(feeds),
      m_feed_timeout(feed_timeout) {}

  inline void AsxTradeItchSequencer::add(int feed,
      const MoldUdp64Packet& packet, boost::posix_time::ptime timestamp) {
    update(timestamp);
    if(m_session && *m_session != packet.m_session) {
      return;
    }
    auto position = get_position(packet);
    if(!m_session) {
      m_session = packet.m_session;
    }
    if(!m_expected_sequence) {
      m_expected_sequence = packet.m_sequence_number;
    }
    auto& source = m_feeds[feed];
    auto is_current_heartbeat = [&] {
      if(!(packet.is_heartbeat() || packet.is_end_of_session()) ||
          position != source.m_position) {
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
    if(packet.is_end_of_session()) {
      m_end_sequence = position;
    }
    store(packet, position);
  }

  inline void AsxTradeItchSequencer::recover(const MoldUdp64Packet& packet) {
    if(!m_session || *m_session != packet.m_session) {
      return;
    }
    store(packet, get_position(packet));
  }

  inline void AsxTradeItchSequencer::update(
      boost::posix_time::ptime timestamp) {
    for(auto& source : m_feeds) {
      if(source.m_is_active &&
          (timestamp < source.m_timestamp ||
            timestamp - source.m_timestamp > m_feed_timeout)) {
        source.m_is_active = false;
      }
    }
  }

  inline boost::optional<Beam::SharedBuffer> AsxTradeItchSequencer::read() {
    if(m_messages.empty() ||
        m_messages.front().m_sequence != m_expected_sequence) {
      return boost::none;
    }
    auto payload = std::move(m_messages.front().m_payload);
    m_messages.pop_front();
    ++*m_expected_sequence;
    return payload;
  }

  inline const boost::optional<AsxTradeItchSequencer::Session>&
      AsxTradeItchSequencer::get_session() const {
    return m_session;
  }

  inline boost::optional<std::uint64_t>
      AsxTradeItchSequencer::get_sequence() const {
    return m_expected_sequence;
  }

  inline std::uint64_t AsxTradeItchSequencer::get_position() const {
    auto position = boost::optional<std::uint64_t>();
    for(auto& source : m_feeds) {
      if(source.m_is_active && (!position || source.m_position < *position)) {
        position = source.m_position;
      }
    }
    if(position) {
      return *position;
    }
    auto furthest = std::uint64_t(0);
    for(auto& source : m_feeds) {
      furthest = std::max(furthest, source.m_position);
    }
    return furthest;
  }

  inline boost::optional<AsxTradeItchGap>
      AsxTradeItchSequencer::get_gap() const {
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
    return AsxTradeItchGap(*m_expected_sequence, end - *m_expected_sequence);
  }

  inline bool AsxTradeItchSequencer::is_end_of_session() const {
    return m_end_sequence && m_expected_sequence &&
      *m_expected_sequence >= *m_end_sequence;
  }

  inline void AsxTradeItchSequencer::reset(std::uint64_t sequence) {
    while(!m_messages.empty() && m_messages.front().m_sequence < sequence) {
      m_messages.pop_front();
    }
    m_expected_sequence = sequence;
  }

  inline void AsxTradeItchSequencer::reset(
      const Session& session, std::uint64_t sequence) {
    std::ranges::fill(m_feeds, Feed());
    m_session = session;
    m_expected_sequence = sequence;
    m_end_sequence = boost::none;
    m_messages.clear();
  }

  inline std::uint64_t AsxTradeItchSequencer::get_position(
      const MoldUdp64Packet& packet) {
    if(packet.is_end_of_session()) {
      return packet.m_sequence_number;
    }
    if(packet.m_count > std::numeric_limits<std::uint64_t>::max() -
        packet.m_sequence_number) {
      boost::throw_with_location(
        AsxTradeItchParserException("MoldUDP64 sequence range overflow."));
    }
    return packet.m_sequence_number + packet.m_count;
  }

  inline void AsxTradeItchSequencer::store(
      const MoldUdp64Packet& packet, std::uint64_t position) {
    auto sequence = packet.m_sequence_number;
    if(packet.is_heartbeat() || packet.is_end_of_session() ||
        position <= *m_expected_sequence) {
      return;
    }
    auto payload = Beam::SharedBuffer();
    auto offset = std::size_t(0);
    for(auto& message : packet) {
      if(sequence >= *m_expected_sequence) {
        auto entry = [&] {
          if(m_messages.empty() || sequence > m_messages.back().m_sequence) {
            return m_messages.end();
          }
          return std::ranges::lower_bound(
            m_messages, sequence, {}, &Entry::m_sequence);
        }();
        if(entry == m_messages.end() || entry->m_sequence != sequence) {
          if(payload.get_size() == 0) {
            payload = Beam::SharedBuffer(
              packet.m_payload.data(), packet.m_payload.size());
          }
          m_messages.insert(entry, Entry(sequence, payload.slice(
            offset + MoldUdp64Message::HEADER_LENGTH, message.m_length)));
        }
      }
      offset += MoldUdp64Message::HEADER_LENGTH + message.m_length;
      ++sequence;
    }
  }
}

#endif
