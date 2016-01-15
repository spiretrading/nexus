#ifndef NEXUS_STAMPPACKET_HPP
#define NEXUS_STAMPPACKET_HPP
#include <cstdint>
#include <Beam/Pointers/Out.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/StampProtocol/StampHeader.hpp"
#include "Nexus/StampProtocol/StampParserException.hpp"
#include "Nexus/StampProtocol/StampProtocol.hpp"

namespace Nexus {
namespace StampProtocol {

  /*! \struct StampPacket
      \brief Stores a STAMP packet.
   */
  struct StampPacket {

    //! Used to mark the beginning of a STAMP packet.
    static const unsigned char START_TOKEN = '\x02';

    //! Used to mark the end of a STAMP packet.
    static const unsigned char END_TOKEN = '\x03';

    //! The packet's header.
    StampHeader m_header;

    //! Pointer to the first byte in the packet's message.
    const char* m_message;

    //! The size of the message.
    std::size_t m_messageSize;

    //! Parses a StampPacket.
    /*!
      \param token A pointer to the first byte in the packet.
      \param size The size of the buffer to parse.
      \return The parsed packet.
    */
    static StampPacket Parse(const char* token, std::size_t size);

    //! Constructs a StampPacket.
    StampPacket();
  };

  inline StampPacket StampPacket::Parse(const char* token, std::size_t size) {
    StampPacket packet;
    if(size < 1) {
      BOOST_THROW_EXCEPTION(StampParserException("STAMP packet too short."));
    }
    if(*token != START_TOKEN) {
      BOOST_THROW_EXCEPTION(StampParserException("Start token not found."));
    }
    ++token;
    const char* headerToken = token;
    packet.m_header = StampHeader::Parse(Beam::Store(headerToken), size - 1);
    if(size < packet.m_header.m_length - 2U) {
      BOOST_THROW_EXCEPTION(StampParserException("STAMP packet too short."));
    }
    token = headerToken;
    packet.m_message = token;
    packet.m_messageSize = packet.m_header.m_length - StampHeader::HEADER_SIZE;
    token += packet.m_messageSize;
    if(*token != END_TOKEN) {
      BOOST_THROW_EXCEPTION(StampParserException("End token not found."));
    }
    ++token;
    return packet;
  }

  inline StampPacket::StampPacket()
      : m_message(nullptr),
        m_messageSize(0) {}
}
}

#endif
