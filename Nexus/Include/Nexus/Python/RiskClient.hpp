#ifndef NEXUS_PYTHON_RISK_CLIENT_HPP
#define NEXUS_PYTHON_RISK_CLIENT_HPP
#include <memory>
#include <Beam/Python/GilRelease.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/RiskService/VirtualRiskClient.hpp"

namespace Nexus::RiskService {

  /**
   * Wraps a RiskClient for use with Python.
   * @param <C> The type of RiskClient to wrap.
   */
  template<typename C>
  class ToPythonRiskClient final : public VirtualRiskClient {
    public:

      /** The type of OrderExecutionClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonRiskClient.
       * @param client The RiskClient to wrap.
       */
      ToPythonRiskClient(std::unique_ptr<Client> client);

      ~ToPythonRiskClient() override;

      InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Reset(const Region& region) override;

      const RiskPortfolioUpdatePublisher&
        GetRiskPortfolioUpdatePublisher() override;

      void Close() override;

    private:
      std::unique_ptr<Client> m_client;
  };

  /**
   * Makes a ToPythonRiskClient.
   * @param client The RiskClient to wrap.
   */
  template<typename Client>
  auto MakeToPythonRiskClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonRiskClient<Client>>(std::move(client));
  }

  template<typename C>
  ToPythonRiskClient<C>::ToPythonRiskClient(std::unique_ptr<Client> client)
    : m_client(std::move(client)) {}

  template<typename C>
  ToPythonRiskClient<C>::~ToPythonRiskClient() {
    Close();
    auto release = Beam::Python::GilRelease();
    m_client.reset();
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
