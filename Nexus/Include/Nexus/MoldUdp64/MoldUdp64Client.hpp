#ifndef NEXUS_MOLDUDP64CLIENT_HPP
#define NEXUS_MOLDUDP64CLIENT_HPP
#include <cstdint>
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/MoldUdp64/MoldUdp64Message.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Packet.hpp"

namespace Nexus {
namespace MoldUdp64 {

  /*! \class MoldUdp64Client
      \brief Implements a client using the MoldUdp64 protocol.
      \tparam ChannelType The type of Channel connected to the MoldUdp64 server.
   */
  template<typename ChannelType>
  class MoldUdp64Client : private boost::noncopyable {
    public:

      //! The type of Channel connected to the MoldUdp64 server.
      using Channel = Beam::GetTryDereferenceType<ChannelType>;

      //! Constructs a MoldUdp64Client.
      /*!
        \param channel The Channel to connect to the MoldUdp64 server
      */
      template<typename ChannelForward>
      MoldUdp64Client(ChannelForward&& channel);

      ~MoldUdp64Client();

      //! Reads the next message from the feed.
      MoldUdp64Message Read();

      //! Reads the next message from the feed.
      /*!
        \param sequenceNumber The message's sequence number.
      */
      MoldUdp64Message Read(Beam::Out<std::uint64_t> sequenceNumber);

      void Close();

    private:
      using Buffer = typename Channel::Reader::Buffer;
      Beam::GetOptionalLocalPtr<ChannelType> m_channel;
      Buffer m_buffer;
      MoldUdp64Packet m_packet;
      const char* m_source;
      std::size_t m_remainingSize;
      std::uint64_t m_sequenceNumber;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ChannelType>
  template<typename ChannelForward>
  MoldUdp64Client<ChannelType>::MoldUdp64Client(ChannelForward&& channel)
      : m_channel(std::forward<ChannelType>(channel)),
        m_sequenceNumber(-1) {
    m_openState.SetOpen();
  }

  template<typename ChannelType>
  MoldUdp64Client<ChannelType>::~MoldUdp64Client() {
    Close();
  }

  template<typename ChannelType>
  MoldUdp64Message MoldUdp64Client<ChannelType>::Read() {
    std::uint64_t sequenceNumber;
    return Read(Beam::Store(sequenceNumber));
  }

  template<typename ChannelType>
  MoldUdp64Message MoldUdp64Client<ChannelType>::Read(
      Beam::Out<std::uint64_t> sequenceNumber) {
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
    }
    if(m_sequenceNumber == -1 ||
        m_sequenceNumber == m_packet.m_sequenceNumber + m_packet.m_count) {
      while(true) {
        m_buffer.Reset();
        m_channel->GetReader().Read(Beam::Store(m_buffer));
        m_packet = MoldUdp64Packet::Parse(
          m_buffer.GetData(), m_buffer.GetSize());
        if(m_packet.m_count != 0) {
          m_sequenceNumber = m_packet.m_sequenceNumber;
          m_source = m_packet.m_payload;
          m_remainingSize = m_buffer.GetSize() - MoldUdp64Packet::PACKET_LENGTH;
          break;
        }
      }
    }
    auto message = MoldUdp64Message::Parse(m_source, m_remainingSize);
    auto messageSize = message.m_length + sizeof(message.m_length);
    m_remainingSize -= messageSize;
    m_source += messageSize;
    *sequenceNumber = m_sequenceNumber;
    ++m_sequenceNumber;
    return message;
  }

  template<typename ChannelType>
  void MoldUdp64Client<ChannelType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType>
  void MoldUdp64Client<ChannelType>::Shutdown() {
    m_channel->GetConnection().Close();
    m_openState.SetClosed();
  }
}
}

#endif
