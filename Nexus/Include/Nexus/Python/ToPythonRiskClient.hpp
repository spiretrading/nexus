#ifndef NEXUS_PYTHON_RISK_CLIENT_HPP
#define NEXUS_PYTHON_RISK_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/RiskService/RiskClientBox.hpp"

namespace Nexus::RiskService {

  /**
   * Wraps a RiskClient for use with Python.
   * @param <C> The type of RiskClient to wrap.
   */
  template<typename C>
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
      const Client& GetClient() const;

      /** Returns the wrapped client. */
      Client& GetClient();

      InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void Reset(const Region& region);

      const RiskPortfolioUpdatePublisher& GetRiskPortfolioUpdatePublisher();

      void Close();

    private:
      boost::optional<Client> m_client;

      ToPythonRiskClient(const ToPythonRiskClient&) = delete;
      ToPythonRiskClient& operator =(const ToPythonRiskClient&) = delete;
  };

  template<typename Client>
  ToPythonRiskClient(Client&&) -> ToPythonRiskClient<std::decay_t<Client>>;

  template<typename C>
  template<typename... Args, typename>
  ToPythonRiskClient<C>::ToPythonRiskClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonRiskClient<C>::~ToPythonRiskClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonRiskClient<C>::Client&
      ToPythonRiskClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonRiskClient<C>::Client& ToPythonRiskClient<C>::GetClient() {
    return *m_client;
  }

  template<typename C>
  InventorySnapshot ToPythonRiskClient<C>::LoadInventorySnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadInventorySnapshot(account);
  }

  template<typename C>
  void ToPythonRiskClient<C>::Reset(const Region& region) {
    auto release = Beam::Python::GilRelease();
    return m_client->Reset(region);
  }

  template<typename C>
  const RiskPortfolioUpdatePublisher&
      ToPythonRiskClient<C>::GetRiskPortfolioUpdatePublisher() {
    auto release = Beam::Python::GilRelease();
    return m_client->GetRiskPortfolioUpdatePublisher();
  }

  template<typename C>
  void ToPythonRiskClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
