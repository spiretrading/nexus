#ifndef NEXUS_MOLDUDP64MESSAGE_HPP
#define NEXUS_MOLDUDP64MESSAGE_HPP
#include <cstdint>
#include <Beam/Utilities/Endian.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/MoldUdp64/MoldUdp64.hpp"
#include "Nexus/MoldUdp64/MoldUdp64ParserException.hpp"

namespace Nexus {
namespace MoldUdp64 {

  /*! \struct MoldUdp64Message
      \brief Stores a MoldUdp64 message.
   */
  struct MoldUdp64Message {

    //! The length of the message, ignoring this field.
    std::uint16_t m_length;

    //! The code identifying this message type.
    std::uint8_t m_messageType;

    //! The data contents of this message.
    const char* m_data;

    //! Parses a MoldUdp64Message.
    /*!
      \param source The first byte in the message to parse.
      \param size The remaining number of bytes in the packet.
      \return The MoldUdp64Message represented by the <i>source</i>.
    */
    static MoldUdp64Message Parse(const char* source, std::size_t size);
  };

  inline MoldUdp64Message MoldUdp64Message::Parse(const char* source,
      std::size_t size) {
    static const auto LENGTH_LENGTH = 2;
    static const auto TYPE_LENGTH = 1;
    static const auto HEADER_LENGTH = LENGTH_LENGTH + TYPE_LENGTH;
    if(size < HEADER_LENGTH) {
      BOOST_THROW_EXCEPTION(MoldUdp64ParserException("Packet too short."));
    }
    MoldUdp64Message message;
    message.m_length = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint16_t*>(source));
    if(size < message.m_length + sizeof(message.m_length)) {
      BOOST_THROW_EXCEPTION(MoldUdp64ParserException("Packet too short."));
    }
    source += LENGTH_LENGTH;
    message.m_messageType = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint8_t*>(source));
    source += TYPE_LENGTH;
    message.m_data = source;
    return message;
  }
}
}

#endif
