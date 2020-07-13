#ifndef NEXUS_BINARY_SEQUENCE_PROTOCOL_HPP
#define NEXUS_BINARY_SEQUENCE_PROTOCOL_HPP

namespace Nexus::BinarySequenceProtocol {
  template<typename C, typename S> class BinarySequenceProtocolClient;
  struct BinarySequenceProtocolMessage;
  template<typename S> struct BinarySequenceProtocolPacket;
  class BinarySequenceProtocolParserException;
}

#endif
