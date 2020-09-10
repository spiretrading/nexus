#ifndef NEXUS_UTP_PROTOCOL_CLIENT_HPP
#define NEXUS_UTP_PROTOCOL_CLIENT_HPP
#include <cstdint>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MoldUdp64/MoldUdp64Client.hpp"
#include "UtpMarketDataFeedClient/UtpMessage.hpp"

namespace Nexus::MarketDataService {

  /**
   * Implements a client using the UTP protocol.
   * @param <C> The type of Channel connected to the server.
   */
  template<typename C>
  class UtpProtocolClient : private boost::noncopyable {
    public:

      /** The type of Channel connected to the server. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a UtpProtocolClient.
       * @param channel The Channel to connect to the server
       */
      template<typename CF>
      UtpProtocolClient(CF&& channel);

      ~UtpProtocolClient();

      /**
       * Reads the next message from the feed.
       * @return The next UtpMessage in the data feed.
       */
      UtpMessage Read();

      void Close();

    private:
      MoldUdp64::MoldUdp64Client<C> m_moldClient;
      std::uint64_t m_sequenceNumber;
      Beam::IO::OpenState m_openState;
  };

  template<typename C>
  template<typename CF>
  UtpProtocolClient<C>::UtpProtocolClient(CF&& channel)
    : m_moldClient(std::forward<CF>(channel)),
      m_sequenceNumber(-1) {}

  template<typename C>
  UtpProtocolClient<C>::~UtpProtocolClient() {
    Close();
  }

  template<typename C>
  UtpMessage UtpProtocolClient<C>::Read() {
    m_openState.EnsureOpen();
    auto sequenceNumber = std::uint64_t();
    auto moldMessage = m_moldClient.Read(Beam::Store(sequenceNumber));
    if(m_sequenceNumber != -1 && sequenceNumber > m_sequenceNumber + 1) {
      std::cout << "Packets dropped: " << (m_sequenceNumber + 1) << " - " <<
        (sequenceNumber - 1) << std::endl;
    }
    m_sequenceNumber = sequenceNumber;
    auto token = moldMessage.m_data;
    auto message = UtpMessage::Parse(Beam::Store(token), moldMessage.m_length);
    return message;
  }

  template<typename C>
  void UtpProtocolClient<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_moldClient.Close();
    m_openState.Close();
  }
}

#endif
