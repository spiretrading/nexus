#ifndef NEXUS_MOLD_UDP_64_PACKET_HPP
#define NEXUS_MOLD_UDP_64_PACKET_HPP
#include <cstring>
#include <iterator>
#include <Beam/Utilities/FixedString.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/MoldUdp64/MoldUdp64Message.hpp"

namespace Nexus {

  /** A downstream packet view, valid while its source buffer is unchanged. */
  struct MoldUdp64Packet {

    /** The length of the packet header. */
    static constexpr auto PACKET_LENGTH = std::size_t(20);

    /** The length of the session field. */
    static constexpr auto SESSION_FIELD_LENGTH = std::size_t(10);

    /** The message count indicating the end of a session. */
    static constexpr auto END_OF_SESSION = std::uint16_t(0xFFFF);

    /** Iterates over the messages within a packet. */
    class Iterator {
      public:

        /** The message type produced by this iterator. */
        using value_type = MoldUdp64Message;

        /** The type representing a distance between iterator positions. */
        using difference_type = std::ptrdiff_t;

        /** The traversal supported by this iterator. */
        using iterator_concept = std::input_iterator_tag;

        const MoldUdp64Message& operator *() const;
        const MoldUdp64Message* operator ->() const;
        Iterator& operator ++();
        Iterator operator ++(int);
        bool operator ==(std::default_sentinel_t) const;

      private:
        friend struct MoldUdp64Packet;
        std::string_view m_source;
        std::uint16_t m_remaining;
        MoldUdp64Message m_message;

        Iterator(std::string_view source, std::uint16_t remaining);
    };

    /** Identity of the session the payload relates to. */
    Beam::FixedString<SESSION_FIELD_LENGTH> m_session;

    /** Sequence number of the first message, or the next expected message. */
    std::uint64_t m_sequence_number;

    /** The message count, or END_OF_SESSION. */
    std::uint16_t m_count;

    /** The length-prefixed messages following the header. */
    std::string_view m_payload;

    /**
     * Parses and validates a complete downstream datagram.
     * @param source The datagram to parse.
     * @return A view of the packet.
     */
    static MoldUdp64Packet parse(std::string_view source);

    /** Returns whether this packet is a heartbeat. */
    bool is_heartbeat() const;

    /** Returns whether this packet marks the end of the session. */
    bool is_end_of_session() const;

    Iterator begin() const;
    std::default_sentinel_t end() const;
  };

  inline const MoldUdp64Message&
      MoldUdp64Packet::Iterator::operator *() const {
    return m_message;
  }

  inline const MoldUdp64Message*
      MoldUdp64Packet::Iterator::operator ->() const {
    return &m_message;
  }

  inline MoldUdp64Packet::Iterator& MoldUdp64Packet::Iterator::operator ++() {
    m_source.remove_prefix(
      MoldUdp64Message::HEADER_LENGTH + m_message.m_length);
    --m_remaining;
    if(m_remaining != 0) {
      m_message = MoldUdp64Message::parse(m_source);
    }
    return *this;
  }

  inline MoldUdp64Packet::Iterator MoldUdp64Packet::Iterator::operator ++(int) {
    auto previous = *this;
    ++*this;
    return previous;
  }

  inline bool MoldUdp64Packet::Iterator::operator ==(
      std::default_sentinel_t) const {
    return m_remaining == 0;
  }

  inline MoldUdp64Packet::Iterator::Iterator(
      std::string_view source, std::uint16_t remaining)
      : m_source(source),
        m_remaining(remaining),
        m_message() {
    if(m_remaining != 0) {
      m_message = MoldUdp64Message::parse(m_source);
    }
  }

  inline MoldUdp64Packet MoldUdp64Packet::parse(std::string_view source) {
    if(source.size() < PACKET_LENGTH) {
      boost::throw_with_location(MoldUdp64ParserException("Packet too short."));
    }
    auto packet = MoldUdp64Packet();
    packet.m_session = source.substr(0, SESSION_FIELD_LENGTH);
    std::memcpy(&packet.m_sequence_number,
      source.data() + SESSION_FIELD_LENGTH, sizeof(packet.m_sequence_number));
    packet.m_sequence_number =
      boost::endian::big_to_native(packet.m_sequence_number);
    std::memcpy(&packet.m_count,
      source.data() + SESSION_FIELD_LENGTH + sizeof(packet.m_sequence_number),
      sizeof(packet.m_count));
    packet.m_count = boost::endian::big_to_native(packet.m_count);
    packet.m_payload = source.substr(PACKET_LENGTH);
    auto remaining = packet.m_payload;
    if(!packet.is_end_of_session()) {
      for(auto i = 0; i != packet.m_count; ++i) {
        auto message = MoldUdp64Message::parse(remaining);
        remaining.remove_prefix(
          MoldUdp64Message::HEADER_LENGTH + message.m_length);
      }
    }
    if(!remaining.empty()) {
      boost::throw_with_location(
        MoldUdp64ParserException("Packet message count mismatch."));
    }
    return packet;
  }

  inline bool MoldUdp64Packet::is_heartbeat() const {
    return m_count == 0;
  }

  inline bool MoldUdp64Packet::is_end_of_session() const {
    return m_count == END_OF_SESSION;
  }

  inline MoldUdp64Packet::Iterator MoldUdp64Packet::begin() const {
    if(is_end_of_session()) {
      return Iterator(m_payload, 0);
    }
    return Iterator(m_payload, m_count);
  }

  inline std::default_sentinel_t MoldUdp64Packet::end() const {
    return std::default_sentinel;
  }
}

#endif
