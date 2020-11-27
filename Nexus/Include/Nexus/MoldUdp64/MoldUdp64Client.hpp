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
#include "Nexus/MoldUdp64/MoldUdp64Message.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Packet.hpp"

namespace Nexus::MoldUdp64 {

  /**
   * Implements a client using the MoldUdp64 protocol.
   * @param <C> The type of Channel connected to the MoldUdp64 server.
   */
  template<typename C>
  class MoldUdp64Client {
    public:

      /** The type of Channel connected to the MoldUdp64 server. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a MoldUdp64Client.
       * @param channel The Channel to connect to the MoldUdp64 server
       */
      template<typename CF>
      explicit MoldUdp64Client(CF&& channel);

      ~MoldUdp64Client();

      /** Reads the next message from the feed. */
      MoldUdp64Message Read();

      /**
       * Reads the next message from the feed.
       * @param sequenceNumber The message's sequence number.
       */
      MoldUdp64Message Read(Beam::Out<std::uint64_t> sequenceNumber);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<C> m_channel;
      Beam::IO::SharedBuffer m_buffer;
      MoldUdp64Packet m_packet;
      const char* m_source;
      std::size_t m_remainingSize;
      std::uint64_t m_sequenceNumber;
      Beam::IO::OpenState m_openState;

      MoldUdp64Client(const MoldUdp64Client&) = delete;
      MoldUdp64Client& operator =(const MoldUdp64Client&) = delete;
  };

  template<typename C>
  template<typename CF>
  MoldUdp64Client<C>::MoldUdp64Client(CF&& channel)
    try : m_channel(std::forward<C>(channel)),
          m_sequenceNumber(-1) {
    } catch(const std::exception&) {
      std::throw_with_nested(Beam::IO::ConnectException(
        "MoldUDP64 client failed to connect."));
    }

  template<typename C>
  MoldUdp64Client<C>::~MoldUdp64Client() {
    Close();
  }

  template<typename C>
  MoldUdp64Message MoldUdp64Client<C>::Read() {
    auto sequenceNumber = std::uint64_t();
    return Read(Beam::Store(sequenceNumber));
  }

  template<typename C>
  MoldUdp64Message MoldUdp64Client<C>::Read(
      Beam::Out<std::uint64_t> sequenceNumber) {
    if(m_sequenceNumber == -1 ||
        m_sequenceNumber == m_packet.m_sequenceNumber + m_packet.m_count) {
      while(true) {
        m_buffer.Reset();
        Beam::TryOrNest([&] {
          m_channel->GetReader().Read(Beam::Store(m_buffer));
          m_packet = MoldUdp64Packet::Parse(
            m_buffer.GetData(), m_buffer.GetSize());
        }, Beam::IO::IOException("Failed to read MoldUDP64 packet."));
        if(m_packet.m_count != 0) {
          m_sequenceNumber = m_packet.m_sequenceNumber;
          m_source = m_packet.m_payload;
          m_remainingSize = m_buffer.GetSize() - MoldUdp64Packet::PACKET_LENGTH;
          break;
        }
      }
    }
    auto message = Beam::TryOrNest([&] {
      return MoldUdp64Message::Parse(m_source, m_remainingSize);
    }, Beam::IO::IOException("Failed to read MoldUDP64 packet."));
    auto messageSize = message.m_length + sizeof(message.m_length);
    m_remainingSize -= messageSize;
    m_source += messageSize;
    *sequenceNumber = m_sequenceNumber;
    ++m_sequenceNumber;
    return message;
  }

  template<typename C>
  void MoldUdp64Client<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_channel->GetConnection().Close();
    m_openState.Close();
  }
}

#endif
