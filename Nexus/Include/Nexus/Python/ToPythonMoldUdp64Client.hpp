#ifndef NEXUS_TO_PYTHON_MOLD_UDP_64_CLIENT_HPP
#define NEXUS_TO_PYTHON_MOLD_UDP_64_CLIENT_HPP
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/MoldUdp64/MoldUdp64Client.hpp"
#include "Nexus/Python/ToPythonMoldUdp64Packet.hpp"

namespace Nexus {

  /**
   * Wraps a MoldUdp64Client for use with Python.
   * @tparam C The type of client to wrap.
   */
  template<typename C>
  class ToPythonMoldUdp64Client {
    public:

      /** The type of client to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonMoldUdp64Client in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonMoldUdp64Client(Args&&... args);

      ~ToPythonMoldUdp64Client();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      /** Reads the next owned packet. Supports one reader at a time. */
      ToPythonMoldUdp64Packet read();

      /** Closes the client and interrupts a pending read. */
      void close();

    private:
      boost::optional<Client> m_client;

      ToPythonMoldUdp64Client(const ToPythonMoldUdp64Client&) = delete;
      ToPythonMoldUdp64Client& operator =(
        const ToPythonMoldUdp64Client&) = delete;
  };

  template<typename Client>
  ToPythonMoldUdp64Client(Client&&) ->
    ToPythonMoldUdp64Client<std::remove_cvref_t<Client>>;

  template<typename C>
  template<typename... Args>
  ToPythonMoldUdp64Client<C>::ToPythonMoldUdp64Client(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonMoldUdp64Client<C>::~ToPythonMoldUdp64Client() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  typename ToPythonMoldUdp64Client<C>::Client&
      ToPythonMoldUdp64Client<C>::get() {
    return *m_client;
  }

  template<typename C>
  const typename ToPythonMoldUdp64Client<C>::Client&
      ToPythonMoldUdp64Client<C>::get() const {
    return *m_client;
  }

  template<typename C>
  ToPythonMoldUdp64Packet ToPythonMoldUdp64Client<C>::read() {
    auto release = Beam::Python::GilRelease();
    return ToPythonMoldUdp64Packet(m_client->read());
  }

  template<typename C>
  void ToPythonMoldUdp64Client<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
