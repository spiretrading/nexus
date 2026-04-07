#ifndef NEXUS_STAMP_PACKET_HPP
#define NEXUS_STAMP_PACKET_HPP
#include <cstdint>
#include <Beam/Pointers/Out.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Stamp/StampHeader.hpp"
#include "Nexus/Stamp/StampParserException.hpp"

namespace Nexus {

  /** Stores a STAMP packet. */
  struct StampPacket {

    /** Used to mark the beginning of a STAMP packet. */
    static const auto START_TOKEN = (unsigned char)('\x02');

    /** Used to mark the end of a STAMP packet. */
    static const auto END_TOKEN = (unsigned char)('\x03');

    /** The packet's header. */
    StampHeader m_header;

    /** Pointer to the first byte in the packet's message. */
    const char* m_message;

    /** The size of the message. */
    std::size_t m_message_size;

    /**
     * Parses a StampPacket.
     * @param token A pointer to the first byte in the packet.
     * @param size The size of the buffer to parse.
     * @return The parsed packet.
     */
    static StampPacket parse(const char* token, std::size_t size);

    /** Constructs a StampPacket. */
    StampPacket() noexcept;
  };

  inline StampPacket StampPacket::parse(const char* token, std::size_t size) {
    auto packet = StampPacket();
    if(size < 1) {
      boost::throw_with_location(
        StampParserException("STAMP packet too short."));
    }
    if(*token != START_TOKEN) {
      boost::throw_with_location(
        StampParserException("Start token not found."));
    }
    ++token;
    const char* header_token = token;
    packet.m_header = StampHeader::parse(Beam::out(header_token), size - 1);
    if(size < packet.m_header.m_length - 2U) {
      boost::throw_with_location(
        StampParserException("STAMP packet too short."));
    }
    token = header_token;
    packet.m_message = token;
    packet.m_message_size = packet.m_header.m_length - StampHeader::HEADER_SIZE;
    token += packet.m_message_size;
    if(*token != END_TOKEN) {
      boost::throw_with_location(StampParserException("End token not found."));
    }
    ++token;
    return packet;
  }

  inline StampPacket::StampPacket() noexcept
    : m_message(nullptr),
      m_message_size(0) {}
}

#endif
