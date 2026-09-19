#ifndef NEXUS_MOLD_UDP_64_CLIENT_HPP
#define NEXUS_MOLD_UDP_64_CLIENT_HPP
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Utilities/Expect.hpp>
#include "Nexus/MoldUdp64/MoldUdp64Packet.hpp"

namespace Nexus {

  /**
   * Reads packets from a MoldUDP64 feed.
   * @tparam C The type of Channel delivering one complete datagram per read.
   */
  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  class MoldUdp64Client {
    public:

      /** The type of channel receiving the feed. */
      using Channel = Beam::dereference_t<C>;

      /**
       * Constructs a MoldUdp64Client.
       * @param channel The channel receiving the feed.
       */
      template<Beam::Initializes<C> CF>
      explicit MoldUdp64Client(CF&& channel);

      ~MoldUdp64Client();

      /**
       * Reads the next packet, including heartbeats and end-of-session markers.
       * The returned payload is valid until the next read or destruction.
       */
      MoldUdp64Packet read();

      /** Closes the connection to the feed. */
      void close();

    private:
      Beam::local_ptr_t<C> m_channel;
      Beam::SharedBuffer m_buffer;
      Beam::OpenState m_open_state;

      MoldUdp64Client(const MoldUdp64Client&) = delete;
      MoldUdp64Client& operator =(const MoldUdp64Client&) = delete;
  };

  template<typename CF>
  MoldUdp64Client(CF&&) -> MoldUdp64Client<std::remove_cvref_t<CF>>;

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  MoldUdp64Client<C>::MoldUdp64Client(CF&& channel)
    try : m_channel(std::forward<CF>(channel)) {
    } catch(const std::exception&) {
      Beam::throw_nested_with_location(
        Beam::ConnectException("MoldUDP64 client failed to connect."));
    }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  MoldUdp64Client<C>::~MoldUdp64Client() {
    close();
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  MoldUdp64Packet MoldUdp64Client<C>::read() {
    reset(m_buffer);
    try {
      m_channel->get_reader().read(Beam::out(m_buffer));
    } catch(const std::exception&) {
      Beam::throw_nested_with_location(
        Beam::IOException("Failed to read MoldUDP64 packet."));
    }
    return MoldUdp64Packet::parse(
      std::string_view(m_buffer.get_data(), m_buffer.get_size()));
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void MoldUdp64Client<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_channel->get_connection().close();
    m_open_state.close();
  }
}

#endif
