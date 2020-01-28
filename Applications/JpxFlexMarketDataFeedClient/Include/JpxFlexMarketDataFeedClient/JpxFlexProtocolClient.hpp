#ifndef NEXUS_JPX_FLEX_PROTOCOL_CLIENT_HPP
#define NEXUS_JPX_FLEX_PROTOCOL_CLIENT_HPP
#include <utility>
#include <Beam/IO/Channel.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>

namespace Nexus::MarketDataService {

  /**
   * Parses packets from the JPX Flex market data feed.
   * @param <C> The type of channel receiving the packets to parse.
   */
  template<typename C>
  class JpxFlexProtocolClient {
    public:

      /** The type of channel receiving messages. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a JpxFlexProtocolClient.
       * @param channel The channel receiving the packets to parse.
       */
      template<typename ChannelForward>
      JpxFlexProtocolClient(ChannelForward&& channel);

      ~JpxFlexProtocolClient();

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<C> m_channel;
  };

  template<typename C>
  template<typename ChannelForward>
  JpxFlexProtocolClient<C>::JpxFlexProtocolClient(ChannelForward&& channel)
    : m_channel(std::forward<ChannelForward>(channel)) {}

  template<typename C>
  JpxFlexProtocolClient<C>::~JpxFlexProtocolClient() {
    Close();
  }

  template<typename C>
  void JpxFlexProtocolClient<C>::Open() {
    m_channel->GetConnection().Open();
  }

  template<typename C>
  void JpxFlexProtocolClient<C>::Close() {
    m_channel->GetConnection().Close();
  }
}

#endif
