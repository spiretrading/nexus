#ifndef NEXUS_BINARYSEQUENCEPROTOCOLCLIENT_HPP
#define NEXUS_BINARYSEQUENCEPROTOCOLCLIENT_HPP
#include <cstdint>
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocol.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolMessage.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolPacket.hpp"

namespace Nexus {
namespace BinarySequenceProtocol {

  /*! \class BinarySequenceProtocolClient
      \brief Implements a client using the BinarySequenceProtocol.
      \tparam ChannelType The type of Channel connected to the server.
      \tparam SequenceType The type used to represent sequence numbers.
   */
  template<typename ChannelType, typename SequenceType>
  class BinarySequenceProtocolClient : private boost::noncopyable {
    public:

      //! The type of Channel connected to the server.
      using Channel = Beam::GetTryDereferenceType<ChannelType>;

      //! The type used to represent sequence numbers.
      using Sequence = SequenceType;

      //! Constructs a BinarySequenceProtocolClient.
      /*!
        \param channel The Channel to connect to the server
      */
      template<typename ChannelForward>
      BinarySequenceProtocolClient(ChannelForward&& channel);

      ~BinarySequenceProtocolClient();

      //! Reads the next message from the feed.
      BinarySequenceProtocolMessage Read();

      //! Reads the next message from the feed.
      /*!
        \param sequenceNumber The message's sequence number.
      */
      BinarySequenceProtocolMessage Read(Beam::Out<Sequence> sequenceNumber);

      void Open();

      void Close();

    private:
      using Buffer = typename Channel::Reader::Buffer;
      Beam::GetOptionalLocalPtr<ChannelType> m_channel;
      Buffer m_buffer;
      BinarySequenceProtocolPacket<Sequence> m_packet;
      const char* m_source;
      std::size_t m_remainingSize;
      Sequence m_sequenceNumber;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ChannelType, typename SequenceType>
  template<typename ChannelForward>
  BinarySequenceProtocolClient<ChannelType, SequenceType>::
      BinarySequenceProtocolClient(ChannelForward&& channel)
      : m_channel(std::forward<ChannelType>(channel)) {}

  template<typename ChannelType, typename SequenceType>
  BinarySequenceProtocolClient<ChannelType, SequenceType>::
      ~BinarySequenceProtocolClient() {
    Close();
  }

  template<typename ChannelType, typename SequenceType>
  BinarySequenceProtocolMessage
      BinarySequenceProtocolClient<ChannelType, SequenceType>::Read() {
    Sequence sequenceNumber;
    return Read(Beam::Store(sequenceNumber));
  }

  template<typename ChannelType, typename SequenceType>
  BinarySequenceProtocolMessage
      BinarySequenceProtocolClient<ChannelType, SequenceType>::Read(
      Beam::Out<Sequence> sequenceNumber) {
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
    }
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

  template<typename ChannelType, typename SequenceType>
  void BinarySequenceProtocolClient<ChannelType, SequenceType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_channel->GetConnection().Open();
      m_sequenceNumber = -1;
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ChannelType, typename SequenceType>
  void BinarySequenceProtocolClient<ChannelType, SequenceType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType, typename SequenceType>
  void BinarySequenceProtocolClient<ChannelType, SequenceType>::Shutdown() {
    m_channel->GetConnection().Close();
    m_openState.SetClosed();
  }
}
}

#endif
