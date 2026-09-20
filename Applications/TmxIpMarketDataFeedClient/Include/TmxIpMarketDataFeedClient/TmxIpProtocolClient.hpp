#ifndef TMX_IP_PROTOCOL_CLIENT_HPP
#define TMX_IP_PROTOCOL_CLIENT_HPP
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Utilities/Expect.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpPacket.hpp"

namespace Nexus {

  /** Concept satisfied by types delivering the packets of a TMX IP feed. */
  template<typename T>
  concept IsTmxIpProtocolClient = requires(T& t) {
    { t.read() } -> std::same_as<TmxIpPacket>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Reads the packets delivered by a TMX IP multicast feed.
   * @tparam C The type of Channel delivering one complete datagram per read.
   */
  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  class TmxIpProtocolClient {
    public:

      /** The type of channel receiving the feed. */
      using Channel = Beam::dereference_t<C>;

      /**
       * Constructs a TmxIpProtocolClient.
       * @param channel The channel receiving the feed.
       */
      template<Beam::Initializes<C> CF>
      explicit TmxIpProtocolClient(CF&& channel);

      ~TmxIpProtocolClient();

      /**
       * Reads the next packet from the feed.
       * The returned views are valid until the next read or destruction.
       */
      TmxIpPacket read();

      void close();

    private:
      Beam::local_ptr_t<C> m_channel;
      Beam::SharedBuffer m_buffer;
      Beam::OpenState m_open_state;

      TmxIpProtocolClient(const TmxIpProtocolClient&) = delete;
      TmxIpProtocolClient& operator =(
        const TmxIpProtocolClient&) = delete;
  };

  template<typename CF>
  TmxIpProtocolClient(CF&&) -> TmxIpProtocolClient<std::remove_cvref_t<CF>>;

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  TmxIpProtocolClient<C>::TmxIpProtocolClient(CF&& channel)
    try : m_channel(std::forward<CF>(channel)) {
    } catch(const std::exception&) {
      Beam::throw_nested_with_location(Beam::ConnectException(
        "Failed to initialize the TMX IP protocol client."));
    }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  TmxIpProtocolClient<C>::~TmxIpProtocolClient() {
    close();
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  TmxIpPacket TmxIpProtocolClient<C>::read() {
    try {
      reset(m_buffer);
      m_channel->get_reader().read(Beam::out(m_buffer));
    } catch(const std::exception&) {
      Beam::throw_nested_with_location(
        Beam::IOException("Failed to read TMX IP packet."));
    }
    return TmxIpPacket::parse(
      std::string_view(m_buffer.get_data(), m_buffer.get_size()));
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void TmxIpProtocolClient<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_channel->get_connection().close();
    m_open_state.close();
  }
}

#endif
