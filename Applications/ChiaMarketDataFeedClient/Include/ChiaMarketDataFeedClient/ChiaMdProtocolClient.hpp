#ifndef NEXUS_CHIAMDPROTOCOLCLIENT_HPP
#define NEXUS_CHIAMDPROTOCOLCLIENT_HPP
#include <string>
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "ChiaMarketDataFeedClient/ChiaMessage.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class ChiaMdProtocolClient
      \brief Parses packets from the CHIA market data feed.
      \tparam ChannelType The type of Channel receiving data.
   */
  template<typename ChannelType>
  class ChiaMdProtocolClient : private boost::noncopyable {
    public:

      //! The type of Channel receiving data.
      using Channel = Beam::GetTryDereferenceType<ChannelType>;

      //! Constructs a ChiaMdProtocolClient.
      /*!
        \param channel Initializes the Channel receiving data.
        \param username The username.
        \param password The password.
      */
      template<typename ChannelForward>
      ChiaMdProtocolClient(ChannelForward&& channel, std::string username,
        std::string password);

      //! Constructs a ChiaMdProtocolClient.
      /*!
        \param channel Initializes the Channel receiving data.
        \param username The username.
        \param password The password.
        \param session The session ID.
        \param sequence The sequence of the next expected message.
      */
      template<typename ChannelForward>
      ChiaMdProtocolClient(ChannelForward&& channel, std::string username,
        std::string password, std::string session, std::uint32_t sequence);

      ~ChiaMdProtocolClient();

      //! Reads the next message.
      ChiaMessage Read();

      //! Reads the next message.
      /*!
        \param sequenceNumber The message's sequence number.
      */
      ChiaMessage Read(Beam::Out<std::uint32_t> sequenceNumber);

      //! Reads the next message.
      /*!
        \param sequenceNumber The message's sequence number.
      */
      Beam::IO::SharedBuffer ReadBuffer(
        Beam::Out<std::uint32_t> sequenceNumber);

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<ChannelType> m_channel;
      std::string m_username;
      std::string m_password;
      std::string m_session;
      std::uint32_t m_sequence;
      std::uint32_t m_nextSequence;
      Beam::IO::SharedBuffer m_message;
      Beam::IO::SharedBuffer m_buffer;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Beam::IO::SharedBuffer ReadBuffer();
      void Append(const std::string& value, int size,
        Beam::Out<Beam::IO::SharedBuffer> buffer);
      void Append(int value, int size,
        Beam::Out<Beam::IO::SharedBuffer> buffer);
      void Append(std::uint32_t value, int size,
        Beam::Out<Beam::IO::SharedBuffer> buffer);
  };

  template<typename ChannelType>
  template<typename ChannelForward>
  ChiaMdProtocolClient<ChannelType>::ChiaMdProtocolClient(
      ChannelForward&& channel, std::string username, std::string password)
      : ChiaMdProtocolClient{std::forward<ChannelForward>(channel),
          std::move(username), std::move(password), "", 0} {}

  template<typename ChannelType>
  template<typename ChannelForward>
  ChiaMdProtocolClient<ChannelType>::ChiaMdProtocolClient(
      ChannelForward&& channel, std::string username, std::string password,
      std::string session, std::uint32_t sequence)
      : m_channel{std::forward<ChannelForward>(channel)},
        m_username{std::move(username)},
        m_password{std::move(password)},
        m_session{std::move(session)},
        m_sequence{sequence} {}

  template<typename ChannelType>
  ChiaMdProtocolClient<ChannelType>::~ChiaMdProtocolClient() {
    Close();
  }

  template<typename ChannelType>
  ChiaMessage ChiaMdProtocolClient<ChannelType>::Read() {
    std::uint32_t sequence;
    return Read(Beam::Store(sequence));
  }

  template<typename ChannelType>
  ChiaMessage ChiaMdProtocolClient<ChannelType>::Read(
      Beam::Out<std::uint32_t> sequenceNumber) {
    while(true) {
      m_message = ReadBuffer();
      if(!m_message.IsEmpty() && m_message.GetData()[0] == 'S') {
        auto message = ChiaMessage::Parse(m_message.GetData() + 1,
          m_message.GetSize() - 2);
        *sequenceNumber = m_nextSequence;
        ++m_nextSequence;
        return message;
      }
    }
  }

  template<typename ChannelType>
  Beam::IO::SharedBuffer ChiaMdProtocolClient<ChannelType>::ReadBuffer(
      Beam::Out<std::uint32_t> sequenceNumber) {
    while(true) {
      auto buffer = ReadBuffer();
      if(!buffer.IsEmpty() && buffer.GetData()[0] == 'S') {
        buffer.ShrinkFront(1);
        buffer.Shrink(2);
        *sequenceNumber = m_nextSequence;
        ++m_nextSequence;
        return buffer;
      }
    }
  }

  template<typename ChannelType>
  void ChiaMdProtocolClient<ChannelType>::Open() {
    const auto USERNAME_LENGTH = 6;
    const auto PASSWORD_LENGTH = 10;
    const auto SESSION_LENGTH = 10;
    const auto SEQUENCE_OFFSET = 11;
    const auto SEQUENCE_LENGTH = 10;
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_channel->GetConnection().Open();
      Beam::IO::SharedBuffer loginBuffer;
      Append("L", 1, Beam::Store(loginBuffer));
      Append(m_username, USERNAME_LENGTH, Beam::Store(loginBuffer));
      Append(m_password, PASSWORD_LENGTH, Beam::Store(loginBuffer));
      Append(m_session, SESSION_LENGTH, Beam::Store(loginBuffer));
      Append(m_sequence, SEQUENCE_LENGTH, Beam::Store(loginBuffer));
      loginBuffer.Append('\x0A');
      m_channel->GetWriter().Write(loginBuffer);
      auto response = ReadBuffer();
      if(response.GetSize() < SEQUENCE_OFFSET + SEQUENCE_LENGTH ||
          response.GetData()[0] != 'A') {
        BOOST_THROW_EXCEPTION(Beam::IO::ConnectException{"Invalid response."});
      }
      auto cursor = &(response.GetData()[SEQUENCE_OFFSET]);
      m_nextSequence = static_cast<std::uint32_t>(ChiaMessage::ParseNumeric(
        SEQUENCE_LENGTH, Beam::Store(cursor)));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ChannelType>
  void ChiaMdProtocolClient<ChannelType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType>
  void ChiaMdProtocolClient<ChannelType>::Shutdown() {
    m_channel->GetConnection().Close();
    m_openState.SetClosed();
  }

  template<typename ChannelType>
  Beam::IO::SharedBuffer ChiaMdProtocolClient<ChannelType>::ReadBuffer() {
    const auto READ_SIZE = 1024;
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
    }
    while(true) {
      auto delimiter = std::find(m_buffer.GetData(),
        m_buffer.GetData() + m_buffer.GetSize(), '\x0A');
      if(delimiter == m_buffer.GetData() + m_buffer.GetSize()) {
        m_channel->GetReader().Read(Beam::Store(m_buffer), READ_SIZE);
      } else if(delimiter == m_buffer.GetData() + m_buffer.GetSize()) {
        auto buffer = std::move(m_buffer);
        m_buffer.Reset();
        return buffer;
      } else {
        auto size = delimiter - m_buffer.GetData();
        Beam::IO::SharedBuffer buffer{m_buffer.GetData(),
          static_cast<std::size_t>(size)};
        m_buffer.ShrinkFront(size + 1);
        return buffer;
      }
    }
  }

  template<typename ChannelType>
  void ChiaMdProtocolClient<ChannelType>::Append(const std::string& value,
      int size, Beam::Out<Beam::IO::SharedBuffer> buffer) {
    buffer->Append(value.c_str(), value.size());
    for(int i = 0; i < size - static_cast<int>(value.size()); ++i) {
      buffer->Append(' ');
    }
  }

  template<typename ChannelType>
  void ChiaMdProtocolClient<ChannelType>::Append(int value, int size,
      Beam::Out<Beam::IO::SharedBuffer> buffer) {
    auto stringValue = std::to_string(value);
    for(int i = 0; i < size - static_cast<int>(stringValue.size()); ++i) {
      buffer->Append(' ');
    }
    buffer->Append(stringValue.c_str(), stringValue.size());
  }

  template<typename ChannelType>
  void ChiaMdProtocolClient<ChannelType>::Append(std::uint32_t value, int size,
      Beam::Out<Beam::IO::SharedBuffer> buffer) {
    auto stringValue = std::to_string(value);
    for(int i = 0; i < size - static_cast<int>(stringValue.size()); ++i) {
      buffer->Append(' ');
    }
    buffer->Append(stringValue.c_str(), stringValue.size());
  }
}
}

#endif
