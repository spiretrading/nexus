#ifndef NEXUS_SOUPBINTCP_HPP
#define NEXUS_SOUPBINTCP_HPP

namespace Nexus {
namespace SoupBinTcp {
  struct LoginAcceptedPacket;
  struct LoginRejectedPacket;
  template<typename ChannelType, typename TimerType> class SoupBinTcpClient;
  struct SoupBinTcpPacket;
  class SoupBinTcpParserException;
}
}

#endif
