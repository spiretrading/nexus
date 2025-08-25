#ifndef NEXUS_PYTHON_RISK_CLIENT_HPP
#define NEXUS_PYTHON_RISK_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
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
       * Constructs a ToPythonRiskClient.
       * @param args The arguments to forward to the Client's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonRiskClient, Args...>>
      ToPythonRiskClient(Args&&... args);

      ~ToPythonRiskClient();

      /** Returns the wrapped client. */
      const Client& get_client() const;

      /** Returns the wrapped client. */
      Client& get_client();

      InventorySnapshot load_inventory_snapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);
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
    ToPythonRiskClient<std::remove_reference_t<Client>>;

  template<IsRiskClient C>
  template<typename... Args, typename>
  ToPythonRiskClient<C>::ToPythonRiskClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsRiskClient C>
  ToPythonRiskClient<C>::~ToPythonRiskClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<IsRiskClient C>
  const typename ToPythonRiskClient<C>::Client&
      ToPythonRiskClient<C>::get_client() const {
    return *m_client;
  }

  template<IsRiskClient C>
  typename ToPythonRiskClient<C>::Client& ToPythonRiskClient<C>::get_client() {
    return *m_client;
  }

  template<IsRiskClient C>
  InventorySnapshot ToPythonRiskClient<C>::load_inventory_snapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
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
