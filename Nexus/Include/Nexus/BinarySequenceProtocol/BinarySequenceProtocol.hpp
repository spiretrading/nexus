#ifndef NEXUS_BINARYSEQUENCEPROTOCOL_HPP
#define NEXUS_BINARYSEQUENCEPROTOCOL_HPP

namespace Nexus {
namespace BinarySequenceProtocol {
  template<typename ChannelType, typename SequenceType>
    class BinarySequenceProtocolClient;
  struct BinarySequenceProtocolMessage;
  template<typename SequenceType> struct BinarySequenceProtocolPacket;
  class BinarySequenceProtocolParserException;
}
}

#endif
