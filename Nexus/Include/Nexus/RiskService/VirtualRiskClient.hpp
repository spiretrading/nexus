#ifndef NEXUS_VIRTUAL_RISK_CLIENT_HPP
#define NEXUS_VIRTUAL_RISK_CLIENT_HPP
#include <memory>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus::RiskService {

  /** Provides a pure virtual interface to an RiskClient. */
  class VirtualRiskClient : private boost::noncopyable {
    public:
      virtual ~VirtualRiskClient() = default;

      virtual InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      virtual void Reset(const Region& region) = 0;

      virtual const RiskPortfolioUpdatePublisher&
        GetRiskPortfolioUpdatePublisher() = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      /** Constructs a VirtualRiskClient. */
      VirtualRiskClient() = default;
  };

  /**
   * Wraps a RiskClient providing it with a virtual interface.
   * @param <C> The type of RiskClient to wrap.
   */
  template<typename C>
  class WrapperRiskClient : public VirtualRiskClient {
    public:

      /** The RiskClient to wrap. */
      using Client = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a WrapperRiskClient.
       * @param client The RiskClient to wrap.
       */
      template<typename CF>
      explicit WrapperRiskClient(CF&& client);

      InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Reset(const Region& region) override;

      const RiskPortfolioUpdatePublisher& GetRiskPortfolioUpdatePublisher()
        override;

      void Open() override;

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<C> m_client;
  };

  /**
   * Wraps a RiskClient into a VirtualRiskClient.
   * @param client The client to wrap.
   */
  template<typename RiskClient>
  std::unique_ptr<VirtualRiskClient> MakeVirtualRiskClient(
      RiskClient&& client) {
    return std::make_unique<WrapperRiskClient<RiskClient>>(
      std::forward<RiskClient>(client));
  }

  template<typename C>
  template<typename CF>
  WrapperRiskClient<C>::WrapperRiskClient(CF&& client)
    : m_client(std::forward<CF>(client)) {}

  template<typename C>
  InventorySnapshot WrapperRiskClient<C>::LoadInventorySnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadInventorySnapshot(account);
  }

  template<typename C>
  void WrapperRiskClient<C>::Reset(const Region& region) {
    return m_client->Reset(region);
  }

  template<typename C>
  const RiskPortfolioUpdatePublisher& WrapperRiskClient<C>::
      GetRiskPortfolioUpdatePublisher() {
    return m_client->GetRiskPortfolioUpdatePublisher();
  }

  template<typename C>
  void WrapperRiskClient<C>::Open() {
    return m_client->Open();
  }

  template<typename C>
  void WrapperRiskClient<C>::Close() {
    return m_client->Close();
  }
}

#endif
