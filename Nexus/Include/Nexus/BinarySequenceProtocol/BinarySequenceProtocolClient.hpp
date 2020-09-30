#ifndef NEXUS_BINARY_SEQUENCE_PROTOCOL_CLIENT_HPP
#define NEXUS_BINARY_SEQUENCE_PROTOCOL_CLIENT_HPP
#include <cstdint>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
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
  class BinarySequenceProtocolClient : private boost::noncopyable {
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
      BinarySequenceProtocolClient(CF&& channel);

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
      using Buffer = typename Channel::Reader::Buffer;
      Beam::GetOptionalLocalPtr<C> m_channel;
      Buffer m_buffer;
      BinarySequenceProtocolPacket<Sequence> m_packet;
      const char* m_source;
      std::size_t m_remainingSize;
      Sequence m_sequenceNumber;
      Beam::IO::OpenState m_openState;
  };

  template<typename C, typename S>
  template<typename CF>
  BinarySequenceProtocolClient<C, S>::BinarySequenceProtocolClient(CF&& channel)
    : m_channel(std::forward<CF>(channel)),
      m_sequenceNumber(-1) {}

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
  BinarySequenceProtocolMessage BinarySequenceProtocolClient<C, S>::
      Read(Beam::Out<Sequence> sequenceNumber) {
    m_openState.EnsureOpen();
    if(m_sequenceNumber == -1 ||
        m_sequenceNumber == m_packet.m_sequenceNumber + m_packet.m_count) {
      while(true) {
        m_buffer.Reset();
        m_channel->GetReader().Read(Beam::Store(m_buffer));
        m_packet = BinarySequenceProtocolPacket<Sequence>::Parse(
          m_buffer.GetData(), m_buffer.GetSize());
        if(m_packet.m_count != 0) {
          m_sequenceNumber = m_packet.m_sequenceNumber;
          m_source = m_packet.m_payload;
          m_remainingSize = m_buffer.GetSize() -
            BinarySequenceProtocolPacket<Sequence>::PACKET_LENGTH;
          break;
        }
      }
    }
    auto message = BinarySequenceProtocolMessage::Parse(m_source,
      m_remainingSize);
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
