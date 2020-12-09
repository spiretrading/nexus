#ifndef NEXUS_BINARY_SEQUENCE_PROTOCOL_CLIENT_HPP
#define NEXUS_BINARY_SEQUENCE_PROTOCOL_CLIENT_HPP
#include <cstdint>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Expect.hpp>
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocol.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolMessage.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolPacket.hpp"

namespace Nexus::BinarySequenceProtocol {

  /**
   * Implements a client using the BinarySequenceProtocol.
   * @param <C> The type of Channel connected to the server.
   * @param <S> The type used to represent sequence numbers.
   */
  template<typename C, typename S>
  class BinarySequenceProtocolClient {
    public:

      /** The type of Channel connected to the server. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /** The type used to represent sequence numbers. */
      using Sequence = S;

      /**
       * Constructs a BinarySequenceProtocolClient.
       * @param channel The Channel to connect to the server
       */
      template<typename CF>
      explicit BinarySequenceProtocolClient(CF&& channel);

      ~BinarySequenceProtocolClient();

      /** Reads the next message from the feed. */
      BinarySequenceProtocolMessage Read();

      /**
       * Reads the next message from the feed.
       * @param sequenceNumber The message's sequence number.
       */
      BinarySequenceProtocolMessage Read(Beam::Out<Sequence> sequenceNumber);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<C> m_channel;
      Beam::IO::SharedBuffer m_buffer;
      BinarySequenceProtocolPacket<Sequence> m_packet;
      const char* m_source;
      std::size_t m_remainingSize;
      Sequence m_sequenceNumber;
      Beam::IO::OpenState m_openState;

      BinarySequenceProtocolClient(
        const BinarySequenceProtocolClient&) = delete;
      BinarySequenceProtocolClient& operator =(
        const BinarySequenceProtocolClient&) = delete;
  };

  template<typename C, typename S>
  template<typename CF>
  BinarySequenceProtocolClient<C, S>::BinarySequenceProtocolClient(CF&& channel)
    try : m_channel(std::forward<CF>(channel)),
          m_sequenceNumber(-1) {
    } catch(const std::exception&) {
      std::throw_with_nested(Beam::IO::ConnectException(
        "Binary sequence protocol failed to connect."));
    }

  template<typename C, typename S>
  BinarySequenceProtocolClient<C, S>::~BinarySequenceProtocolClient() {
    Close();
  }

  template<typename C, typename S>
  BinarySequenceProtocolMessage BinarySequenceProtocolClient<C, S>::Read() {
    auto sequenceNumber = Sequence();
    return Read(Beam::Store(sequenceNumber));
  }

  template<typename C, typename S>
  BinarySequenceProtocolMessage BinarySequenceProtocolClient<C, S>::Read(
      Beam::Out<Sequence> sequenceNumber) {
    if(m_sequenceNumber == -1 ||
        m_sequenceNumber == m_packet.m_sequenceNumber + m_packet.m_count) {
      while(true) {
        m_buffer.Reset();
        Beam::TryOrNest([&] {
          m_channel->GetReader().Read(Beam::Store(m_buffer));
          m_packet = BinarySequenceProtocolPacket<Sequence>::Parse(
            m_buffer.GetData(), m_buffer.GetSize());
        }, Beam::IO::IOException("Failed to read binary sequence packet."));
        if(m_packet.m_count != 0) {
          m_sequenceNumber = m_packet.m_sequenceNumber;
          m_source = m_packet.m_payload;
          m_remainingSize = m_buffer.GetSize() -
            BinarySequenceProtocolPacket<Sequence>::PACKET_LENGTH;
          break;
        }
      }
    }
    auto message = Beam::TryOrNest([&] {
      return BinarySequenceProtocolMessage::Parse(m_source, m_remainingSize);
    }, Beam::IO::IOException("Failed to read binary sequence packet."));
    auto messageSize = message.m_length + sizeof(message.m_length);
    m_remainingSize -= messageSize;
    m_source += messageSize;
    *sequenceNumber = m_sequenceNumber;
    ++m_sequenceNumber;
    return message;
  }

  template<typename C, typename S>
  void BinarySequenceProtocolClient<C, S>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_channel->GetConnection().Close();
    m_openState.Close();
  }
}

#endif
