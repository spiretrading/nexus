#ifndef OTC_LINK_PACKET_HPP
#define OTC_LINK_PACKET_HPP
#include <iterator>
#include "OtcLinkMarketDataFeedClient/OtcLinkHeader.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkMessage.hpp"

namespace Nexus {

  /** An OTC Link packet view, valid while its source is unchanged. */
  class OtcLinkPacket {
    public:

      /** Iterates over the messages within a packet. */
      class Iterator {
        public:

          /** The message type produced by this iterator. */
          using value_type = OtcLinkMessage;

          /** The type representing a distance between iterator positions. */
          using difference_type = std::ptrdiff_t;

          /** The traversal supported by this iterator. */
          using iterator_concept = std::input_iterator_tag;

          const OtcLinkMessage& operator *() const;
          const OtcLinkMessage* operator ->() const;
          Iterator& operator ++();
          Iterator operator ++(int);
          bool operator ==(std::default_sentinel_t) const;

        private:
          friend class OtcLinkPacket;
          std::string_view m_source;
          std::uint8_t m_remaining;
          OtcLinkMessage m_message;

          Iterator(std::string_view source, std::uint8_t remaining);
      };

      /**
       * Parses a complete packet and validates all message boundaries.
       * @param source The complete packet.
       */
      static OtcLinkPacket parse(std::string_view source);

      /** Returns this packet's header. */
      const OtcLinkHeader& get_header() const;

      /** Returns the message bytes following the packet header. */
      std::string_view get_payload() const;

      Iterator begin() const;
      std::default_sentinel_t end() const;

    private:
      OtcLinkHeader m_header;
      std::string_view m_payload;
  };

  inline const OtcLinkMessage& OtcLinkPacket::Iterator::operator *() const {
    return m_message;
  }

  inline const OtcLinkMessage* OtcLinkPacket::Iterator::operator ->() const {
    return &m_message;
  }

  inline OtcLinkPacket::Iterator& OtcLinkPacket::Iterator::operator ++() {
    m_source.remove_prefix(m_message.m_length);
    --m_remaining;
    if(m_remaining != 0) {
      m_message = OtcLinkMessage::parse(m_source);
    }
    return *this;
  }

  inline OtcLinkPacket::Iterator OtcLinkPacket::Iterator::operator ++(int) {
    auto previous = *this;
    ++*this;
    return previous;
  }

  inline bool OtcLinkPacket::Iterator::operator ==(
      std::default_sentinel_t) const {
    return m_remaining == 0;
  }

  inline OtcLinkPacket::Iterator::Iterator(std::string_view source,
      std::uint8_t remaining)
      : m_source(source),
        m_remaining(remaining),
        m_message() {
    if(m_remaining != 0) {
      m_message = OtcLinkMessage::parse(m_source);
    }
  }

  inline OtcLinkPacket OtcLinkPacket::parse(std::string_view source) {
    auto packet = OtcLinkPacket();
    packet.m_header = OtcLinkHeader::parse(source);
    if(packet.m_header.m_length != source.size()) {
      boost::throw_with_location(
        OtcLinkParserException("OTC Link packet length mismatch."));
    }
    packet.m_payload = source.substr(OtcLinkHeader::LENGTH);
    auto is_control =
      packet.m_header.has_flag(OtcLinkHeader::Flag::HEARTBEAT) ||
      packet.m_header.has_flag(OtcLinkHeader::Flag::SEQUENCE_RESET);
    if(is_control && packet.m_header.m_count != 0) {
      boost::throw_with_location(
        OtcLinkParserException("OTC Link control packet contains messages."));
    }
    auto remaining = packet.m_payload;
    for(auto i = 0; i < packet.m_header.m_count; ++i) {
      auto message = OtcLinkMessage::parse(remaining);
      remaining.remove_prefix(message.m_length);
    }
    if(!remaining.empty()) {
      boost::throw_with_location(
        OtcLinkParserException("OTC Link packet message count mismatch."));
    }
    return packet;
  }

  inline const OtcLinkHeader& OtcLinkPacket::get_header() const {
    return m_header;
  }

  inline std::string_view OtcLinkPacket::get_payload() const {
    return m_payload;
  }

  inline OtcLinkPacket::Iterator OtcLinkPacket::begin() const {
    return Iterator(m_payload, m_header.m_count);
  }

  inline std::default_sentinel_t OtcLinkPacket::end() const {
    return std::default_sentinel;
  }
}

#endif
