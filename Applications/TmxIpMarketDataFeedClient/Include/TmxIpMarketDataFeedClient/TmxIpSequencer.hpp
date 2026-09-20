#ifndef TMX_IP_SEQUENCER_HPP
#define TMX_IP_SEQUENCER_HPP
#include <algorithm>
#include <deque>
#include <utility>
#include <Beam/IO/SharedBuffer.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpPacket.hpp"

namespace Nexus {

  /** A missing packet range that does not cross the sequence wrap. */
  struct TmxIpGap {

    /** The first missing sequence. */
    std::uint32_t m_sequence;

    /** The number of missing packets. */
    std::uint32_t m_count;
  };

  /**
   * Orders packets from one TMX IP transport stream.
   * Outstanding packets must span less than half the sequence range.
   * Daily sequence resets require an explicit reset.
   */
  class TmxIpSequencer {
    public:

      /**
       * Adds a parsed live or recovered packet from the selected stream.
       * The first sequenced packet establishes the initial position.
       * @param packet The received packet; its contents are copied.
       */
      void add(const TmxIpPacket& packet);

      /**
       * Returns the next packet in sequence, or none while waiting.
       * Returned views remain valid until the next read or destruction.
       */
      boost::optional<TmxIpPacket> read();

      /** Returns the sequence of the next packet to return. */
      boost::optional<std::uint32_t> get_sequence() const;

      /** Returns the missing range preceding the next buffered packet. */
      boost::optional<TmxIpGap> get_gap() const;

      /** Discards pending packets and waits for a new initial position. */
      void reset();

      /**
       * Discards pending packets and starts at a specified position.
       * @param sequence The next sequence, from 1 through 999999999.
       */
      void reset(std::uint32_t sequence);

    private:
      static constexpr auto MAXIMUM_SEQUENCE = std::uint32_t(999999999);
      struct Entry {
        TmxIpHeader m_header;
        Beam::SharedBuffer m_payload;
      };
      boost::optional<std::uint32_t> m_expected_sequence;
      std::deque<Entry> m_packets;
      Beam::SharedBuffer m_payload;

      static std::uint32_t distance(std::uint32_t first, std::uint32_t last);
  };

  inline void TmxIpSequencer::add(const TmxIpPacket& packet) {
    auto& header = packet.m_header;
    if(is_heartbeat(header)) {
      return;
    }
    auto sequence = *header.m_sequence;
    if(!m_expected_sequence) {
      m_expected_sequence = sequence;
    }
    auto offset = distance(*m_expected_sequence, sequence);
    if(offset > MAXIMUM_SEQUENCE / 2) {
      return;
    }
    auto position = [&] (const auto& entry) {
      return distance(*m_expected_sequence, *entry.m_header.m_sequence);
    };
    auto i = [&] {
      if(m_packets.empty() || position(m_packets.back()) < offset) {
        return m_packets.end();
      }
      return std::ranges::lower_bound(m_packets, offset, {}, position);
    }();
    if(i != m_packets.end() && i->m_header.m_sequence == sequence) {
      return;
    }
    auto payload =
      Beam::SharedBuffer(header.m_service.size() + packet.m_payload.size());
    payload.write(0, header.m_service.data(), header.m_service.size());
    payload.write(header.m_service.size(), packet.m_payload.data(),
      packet.m_payload.size());
    auto copy = header;
    copy.m_service =
      std::string_view(payload.get_data(), header.m_service.size());
    m_packets.insert(i, Entry(copy, std::move(payload)));
  }

  inline boost::optional<TmxIpPacket> TmxIpSequencer::read() {
    if(m_packets.empty() ||
        m_packets.front().m_header.m_sequence != m_expected_sequence) {
      return boost::none;
    }
    auto header = m_packets.front().m_header;
    m_payload = std::move(m_packets.front().m_payload);
    m_packets.pop_front();
    if(*m_expected_sequence == MAXIMUM_SEQUENCE) {
      m_expected_sequence = 1;
    } else {
      ++*m_expected_sequence;
    }
    return TmxIpPacket(header,
      std::string_view(m_payload.get_data(), m_payload.get_size()).substr(
        header.m_service.size()));
  }

  inline boost::optional<std::uint32_t> TmxIpSequencer::get_sequence() const {
    return m_expected_sequence;
  }

  inline boost::optional<TmxIpGap> TmxIpSequencer::get_gap() const {
    if(m_packets.empty()) {
      return boost::none;
    }
    auto count = distance(
      *m_expected_sequence, *m_packets.front().m_header.m_sequence);
    if(count == 0) {
      return boost::none;
    }
    count = std::min(count, MAXIMUM_SEQUENCE - *m_expected_sequence + 1);
    return TmxIpGap(*m_expected_sequence, count);
  }

  inline void TmxIpSequencer::reset() {
    m_packets.clear();
    m_expected_sequence = boost::none;
  }

  inline void TmxIpSequencer::reset(std::uint32_t sequence) {
    if(sequence == 0 || sequence > MAXIMUM_SEQUENCE) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP sequence."));
    }
    reset();
    m_expected_sequence = sequence;
  }

  inline std::uint32_t TmxIpSequencer::distance(
      std::uint32_t first, std::uint32_t last) {
    if(last >= first) {
      return last - first;
    }
    return MAXIMUM_SEQUENCE - first + last;
  }
}

#endif
