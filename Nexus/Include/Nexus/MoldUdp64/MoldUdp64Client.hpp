#ifndef NEXUS_MOLD_UDP_64_CLIENT_HPP
#define NEXUS_MOLD_UDP_64_CLIENT_HPP
#include <cstdint>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/MoldUdp64/MoldUdp64Message.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Packet.hpp"

namespace Nexus {

  /**
   * Implements a client using the MoldUdp64 protocol.
   * @param <C> The type of Channel connected to the MoldUdp64 server.
   */
  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  class MoldUdp64Client {
    public:

      /** The type of Channel connected to the MoldUdp64 server. */
      using Channel = Beam::dereference_t<C>;

      /**
       * Constructs a MoldUdp64Client.
       * @param channel The Channel to connect to the MoldUdp64 server
       */
      template<Beam::Initializes<C> CF>
      explicit MoldUdp64Client(CF&& channel);

      ~MoldUdp64Client();

      /** Reads the next message from the feed. */
      MoldUdp64Message read();

      /**
       * Reads the next message from the feed.
       * @param sequence_number The message's sequence number.
       */
      MoldUdp64Message read(Beam::Out<std::uint64_t> sequence_number);

      void close();

    private:
      Beam::local_ptr_t<C> m_channel;
      Beam::SharedBuffer m_buffer;
      MoldUdp64Packet m_packet;
      const char* m_source;
      std::size_t m_remaining_size;
      std::uint64_t m_sequence_number;
      Beam::OpenState m_open_state;

      MoldUdp64Client(const MoldUdp64Client&) = delete;
      MoldUdp64Client& operator =(const MoldUdp64Client&) = delete;
  };

  template<typename C>
  MoldUdp64Client(C&&) -> MoldUdp64Client<std::remove_cvref_t<C>>;

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  MoldUdp64Client<C>::MoldUdp64Client(CF&& channel)
    try : m_channel(std::forward<CF>(channel)),
          m_sequence_number(-1) {
    } catch(const std::exception&) {
      std::throw_with_nested(
        Beam::ConnectException("MoldUDP64 client failed to connect."));
    }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  MoldUdp64Client<C>::~MoldUdp64Client() {
    close();
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  MoldUdp64Message MoldUdp64Client<C>::read() {
    auto sequence_number = std::uint64_t();
    return read(Beam::out(sequence_number));
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  MoldUdp64Message MoldUdp64Client<C>::read(
      Beam::Out<std::uint64_t> sequence_number) {
    if(m_sequence_number == -1 ||
        m_sequence_number == m_packet.m_sequence_number + m_packet.m_count) {
      while(true) {
        reset(m_buffer);
        Beam::try_or_nest([&] {
          m_channel->get_reader().read(Beam::out(m_buffer));
          m_packet = MoldUdp64Packet::parse(
            std::string_view(m_buffer.get_data(), m_buffer.get_size()));
        }, Beam::IOException("Failed to read MoldUDP64 packet."));
        if(m_packet.m_count != 0) {
          m_sequence_number = m_packet.m_sequence_number;
          m_source = m_packet.m_payload;
          m_remaining_size =
            m_buffer.get_size() - MoldUdp64Packet::PACKET_LENGTH;
          break;
        }
      }
    }
    auto message = Beam::try_or_nest([&] {
      return MoldUdp64Message::parse(
        std::string_view(m_source, m_remaining_size));
    }, Beam::IOException("Failed to read MoldUDP64 packet."));
    auto message_size = message.m_length + sizeof(message.m_length);
    m_remaining_size -= message_size;
    m_source += message_size;
    *sequence_number = m_sequence_number;
    ++m_sequence_number;
    return message;
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
