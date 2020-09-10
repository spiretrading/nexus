#ifndef NEXUS_CHIA_MD_PROTOCOL_CLIENT_HPP
#define NEXUS_CHIA_MD_PROTOCOL_CLIENT_HPP
#include <string>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "ChiaMarketDataFeedClient/ChiaMessage.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses packets from the CHIA market data feed.
   * @param <C> The type of Channel receiving data.
   */
  template<typename C>
  class ChiaMdProtocolClient : private boost::noncopyable {
    public:

      /** The type of Channel receiving data. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a ChiaMdProtocolClient.
       * @param channel Initializes the Channel receiving data.
       * @param username The username.
       * @param password The password.
       */
      template<typename CF>
      ChiaMdProtocolClient(CF&& channel, std::string username,
        std::string password);

      /**
       * Constructs a ChiaMdProtocolClient.
       * @param channel Initializes the Channel receiving data.
       * @param username The username.
       * @param password The password.
       * @param session The session ID.
       * @param sequence The sequence of the next expected message.
       */
      template<typename CF>
      ChiaMdProtocolClient(CF&& channel, std::string username,
        std::string password, std::string session, std::uint32_t sequence);

      ~ChiaMdProtocolClient();

      /** Reads the next message. */
      ChiaMessage Read();

      /**
       * Reads the next message.
       * @param sequenceNumber The message's sequence number.
       */
      ChiaMessage Read(Beam::Out<std::uint32_t> sequenceNumber);

      /**
       * Reads the next message.
       * @param sequenceNumber The message's sequence number.
       */
      Beam::IO::SharedBuffer ReadBuffer(
        Beam::Out<std::uint32_t> sequenceNumber);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<C> m_channel;
      std::string m_username;
      std::string m_password;
      std::string m_session;
      std::uint32_t m_sequence;
      std::uint32_t m_nextSequence;
      Beam::IO::SharedBuffer m_message;
      Beam::IO::SharedBuffer m_buffer;
      Beam::IO::OpenState m_openState;

      Beam::IO::SharedBuffer ReadBuffer();
      void Append(const std::string& value, int size,
        Beam::Out<Beam::IO::SharedBuffer> buffer);
      void Append(int value, int size,
        Beam::Out<Beam::IO::SharedBuffer> buffer);
      void Append(std::uint32_t value, int size,
        Beam::Out<Beam::IO::SharedBuffer> buffer);
  };

  template<typename C>
  template<typename CF>
  ChiaMdProtocolClient<C>::ChiaMdProtocolClient(CF&& channel,
    std::string username, std::string password)
    : ChiaMdProtocolClient(std::forward<CF>(channel), std::move(username),
        std::move(password), "", 0) {}

  template<typename C>
  template<typename CF>
  ChiaMdProtocolClient<C>::ChiaMdProtocolClient(CF&& channel,
      std::string username, std::string password, std::string session,
      std::uint32_t sequence)
      : m_channel(std::forward<CF>(channel)),
        m_username(std::move(username)),
        m_password(std::move(password)),
        m_session(std::move(session)),
        m_sequence(sequence) {
    constexpr auto USERNAME_LENGTH = 6;
    constexpr auto PASSWORD_LENGTH = 10;
    constexpr auto SESSION_LENGTH = 10;
    constexpr auto SEQUENCE_OFFSET = 11;
    constexpr auto SEQUENCE_LENGTH = 10;
    try {
      auto loginBuffer = Beam::IO::SharedBuffer();
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
        BOOST_THROW_EXCEPTION(Beam::IO::ConnectException("Invalid response."));
      }
      auto cursor = &(response.GetData()[SEQUENCE_OFFSET]);
      m_nextSequence = static_cast<std::uint32_t>(ChiaMessage::ParseNumeric(
        SEQUENCE_LENGTH, Beam::Store(cursor)));
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C>
  ChiaMdProtocolClient<C>::~ChiaMdProtocolClient() {
    Close();
  }

  template<typename C>
  ChiaMessage ChiaMdProtocolClient<C>::Read() {
    auto sequence = std::uint32_t();
    return Read(Beam::Store(sequence));
  }

  template<typename C>
  ChiaMessage ChiaMdProtocolClient<C>::Read(
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

  template<typename C>
  Beam::IO::SharedBuffer ChiaMdProtocolClient<C>::ReadBuffer(
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

  template<typename C>
  void ChiaMdProtocolClient<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_channel->GetConnection().Close();
    m_openState.Close();
  }

  template<typename C>
  Beam::IO::SharedBuffer ChiaMdProtocolClient<C>::ReadBuffer() {
    constexpr auto READ_SIZE = 1024;
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
        auto buffer = Beam::IO::SharedBuffer(m_buffer.GetData(),
          static_cast<std::size_t>(size));
        m_buffer.ShrinkFront(size + 1);
        return buffer;
      }
    }
  }

  template<typename C>
  void ChiaMdProtocolClient<C>::Append(const std::string& value, int size,
      Beam::Out<Beam::IO::SharedBuffer> buffer) {
    buffer->Append(value.c_str(), value.size());
    for(auto i = 0; i < size - static_cast<int>(value.size()); ++i) {
      buffer->Append(' ');
    }
  }

  template<typename C>
  void ChiaMdProtocolClient<C>::Append(int value, int size,
      Beam::Out<Beam::IO::SharedBuffer> buffer) {
    auto stringValue = std::to_string(value);
    for(auto i = 0; i < size - static_cast<int>(stringValue.size()); ++i) {
      buffer->Append(' ');
    }
    buffer->Append(stringValue.c_str(), stringValue.size());
  }

  template<typename C>
  void ChiaMdProtocolClient<C>::Append(std::uint32_t value, int size,
      Beam::Out<Beam::IO::SharedBuffer> buffer) {
    auto stringValue = std::to_string(value);
    for(auto i = 0; i < size - static_cast<int>(stringValue.size()); ++i) {
      buffer->Append(' ');
    }
    buffer->Append(stringValue.c_str(), stringValue.size());
  }
}

#endif
