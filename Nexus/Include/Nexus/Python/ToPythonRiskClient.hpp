#ifndef NEXUS_PYTHON_RISK_CLIENT_HPP
#define NEXUS_PYTHON_RISK_CLIENT_HPP
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/RiskService/RiskClient.hpp"

namespace Nexus {

  /**
   * Wraps a RiskClient for use with Python.
   * @param <C> The type of RiskClient to wrap.
   */
  template<IsRiskClient C>
  class ToPythonRiskClient {
    public:

      /** The type of client to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonRiskClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonRiskClient(Args&&... args);

      ~ToPythonRiskClient();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      /** Returns a reference to the underlying client. */
      Client& operator *();

      /** Returns a reference to the underlying client. */
      const Client& operator *() const;

      /** Returns a pointer to the underlying client. */
      Client* operator ->();

      /** Returns a pointer to the underlying client. */
      const Client* operator ->() const;

      InventorySnapshot load_inventory_snapshot(
        const Beam::DirectoryEntry& account);
      void reset(const Region& region);
      const RiskPortfolioUpdatePublisher& get_risk_portfolio_update_publisher();
      void close();

    private:
      boost::optional<Client> m_client;

      ToPythonRiskClient(const ToPythonRiskClient&) = delete;
      ToPythonRiskClient& operator =(const ToPythonRiskClient&) = delete;
  };

  template<typename Client>
  ToPythonRiskClient(Client&&) ->
    ToPythonRiskClient<std::remove_cvref_t<Client>>;

  template<IsRiskClient C>
  template<typename... Args>
  ToPythonRiskClient<C>::ToPythonRiskClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsRiskClient C>
  ToPythonRiskClient<C>::~ToPythonRiskClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<IsRiskClient C>
  typename ToPythonRiskClient<C>::Client& ToPythonRiskClient<C>::get() {
    return *m_client;
  }

  template<IsRiskClient C>
  const typename ToPythonRiskClient<C>::Client&
      ToPythonRiskClient<C>::get() const {
    return *m_client;
  }

  template<IsRiskClient C>
  typename ToPythonRiskClient<C>::Client& ToPythonRiskClient<C>::operator *() {
    return *m_client;
  }

  template<IsRiskClient C>
  const typename ToPythonRiskClient<C>::Client&
      ToPythonRiskClient<C>::operator *() const {
    return *m_client;
  }

  template<IsRiskClient C>
  typename ToPythonRiskClient<C>::Client* ToPythonRiskClient<C>::operator ->() {
    return m_client.get_ptr();
  }

  template<IsRiskClient C>
  const typename ToPythonRiskClient<C>::Client*
      ToPythonRiskClient<C>::operator ->() const {
    return m_client.get_ptr();
  }

  template<IsRiskClient C>
  InventorySnapshot ToPythonRiskClient<C>::load_inventory_snapshot(
      const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_inventory_snapshot(account);
  }

  template<IsRiskClient C>
  void ToPythonRiskClient<C>::reset(const Region& region) {
    auto release = Beam::Python::GilRelease();
    m_client->reset(region);
  }

  template<IsRiskClient C>
  const RiskPortfolioUpdatePublisher&
      ToPythonRiskClient<C>::get_risk_portfolio_update_publisher() {
    auto release = Beam::Python::GilRelease();
    return m_client->get_risk_portfolio_update_publisher();
  }

  template<IsRiskClient C>
  void ToPythonRiskClient<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
