#ifndef NEXUS_TO_PYTHON_SOUP_BIN_TCP_CLIENT_HPP
#define NEXUS_TO_PYTHON_SOUP_BIN_TCP_CLIENT_HPP
#include <Beam/Python/GilRelease.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Python/ToPythonSoupBinTcpPacket.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpClient.hpp"

namespace Nexus {

  /**
   * Wraps a SoupBinTcpClient for use with Python.
   * @tparam C The type of client to wrap.
   */
  template<typename C>
  class ToPythonSoupBinTcpClient {
    public:

      /** The type of client to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonSoupBinTcpClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonSoupBinTcpClient(Args&&... args);

      ~ToPythonSoupBinTcpClient();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      /** Reads the next owned packet. Supports one reader at a time. */
      ToPythonSoupBinTcpPacket read();

      /** Returns the session accepted by the server. */
      const std::string& get_session() const;

      /** Returns the next sequenced data packet's sequence number. */
      std::uint64_t get_sequence_number() const;

      /** Closes the client and interrupts a pending read. */
      void close();

    private:
      boost::optional<Client> m_client;

      ToPythonSoupBinTcpClient(const ToPythonSoupBinTcpClient&) = delete;
      ToPythonSoupBinTcpClient& operator =(
        const ToPythonSoupBinTcpClient&) = delete;
  };

  template<typename Client>
  ToPythonSoupBinTcpClient(Client&&) ->
    ToPythonSoupBinTcpClient<std::remove_cvref_t<Client>>;

  template<typename C>
  template<typename... Args>
  ToPythonSoupBinTcpClient<C>::ToPythonSoupBinTcpClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonSoupBinTcpClient<C>::~ToPythonSoupBinTcpClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  typename ToPythonSoupBinTcpClient<C>::Client&
      ToPythonSoupBinTcpClient<C>::get() {
    return *m_client;
  }

  template<typename C>
  const typename ToPythonSoupBinTcpClient<C>::Client&
      ToPythonSoupBinTcpClient<C>::get() const {
    return *m_client;
  }

  template<typename C>
  ToPythonSoupBinTcpPacket ToPythonSoupBinTcpClient<C>::read() {
    auto release = Beam::Python::GilRelease();
    return ToPythonSoupBinTcpPacket(m_client->read());
  }

  template<typename C>
  const std::string& ToPythonSoupBinTcpClient<C>::get_session() const {
    return m_client->get_session();
  }

  template<typename C>
  std::uint64_t ToPythonSoupBinTcpClient<C>::get_sequence_number() const {
    return m_client->get_sequence_number();
  }

  template<typename C>
  void ToPythonSoupBinTcpClient<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
