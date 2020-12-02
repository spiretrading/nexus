#ifndef NEXUS_RISK_CLIENT_BOX_HPP
#define NEXUS_RISK_CLIENT_BOX_HPP
#include <memory>
#include <type_traits>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus::RiskService {

  /** Provides a generic interface over an arbitrary RiskClient. */
  class RiskClientBox {
    public:

      /**
       * Constructs a RiskClientBox of a specified type using emplacement.
       * @param <T> The type of risk client to emplace.
       * @param args The arguments to pass to the emplaced risk client.
       */
      template<typename T, typename... Args>
      explicit RiskClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a RiskClientBox by copying an existing risk client.
       * @param client The client to copy.
       */
      template<typename RiskClient>
      explicit RiskClientBox(RiskClient client);

      explicit RiskClientBox(RiskClientBox* client);

      explicit RiskClientBox(const std::shared_ptr<RiskClientBox>& client);

      explicit RiskClientBox(const std::unique_ptr<RiskClientBox>& client);

      InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void Reset(const Region& region);

      const RiskPortfolioUpdatePublisher& GetRiskPortfolioUpdatePublisher();

      void Close();

    private:
      struct VirtualRiskClient {
        virtual ~VirtualRiskClient() = default;
        virtual InventorySnapshot LoadInventorySnapshot(
          const Beam::ServiceLocator::DirectoryEntry& account) = 0;
        virtual void Reset(const Region& region) = 0;
        virtual const RiskPortfolioUpdatePublisher&
          GetRiskPortfolioUpdatePublisher() = 0;
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedRiskClient final : VirtualRiskClient {
        using RiskClient = C;
        Beam::GetOptionalLocalPtr<RiskClient> m_client;

        template<typename... Args>
        WrappedRiskClient(Args&&... args);
        InventorySnapshot LoadInventorySnapshot(
          const Beam::ServiceLocator::DirectoryEntry& account) override;
        void Reset(const Region& region) override;
        const RiskPortfolioUpdatePublisher&
          GetRiskPortfolioUpdatePublisher() override;
        void Close() override;
      };
      std::shared_ptr<VirtualRiskClient> m_client;
  };

  template<typename T, typename... Args>
  RiskClientBox::RiskClientBox(std::in_place_type_t<T>, Args&&... args)
    : m_client(std::make_shared<WrappedRiskClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename RiskClient>
  RiskClientBox::RiskClientBox(RiskClient client)
    : RiskClientBox(std::in_place_type<RiskClient>, std::move(client)) {}

  inline RiskClientBox::RiskClientBox(RiskClientBox* client)
    : RiskClientBox(*client) {}

  inline RiskClientBox::RiskClientBox(
    const std::shared_ptr<RiskClientBox>& client)
    : RiskClientBox(*client) {}

  inline RiskClientBox::RiskClientBox(
    const std::unique_ptr<RiskClientBox>& client)
    : RiskClientBox(*client) {}

  inline InventorySnapshot RiskClientBox::LoadInventorySnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadInventorySnapshot(account);
  }

  inline void RiskClientBox::Reset(const Region& region) {
    return m_client->Reset(region);
  }

  inline const RiskPortfolioUpdatePublisher&
      RiskClientBox::GetRiskPortfolioUpdatePublisher() {
    return m_client->GetRiskPortfolioUpdatePublisher();
  }

  inline void RiskClientBox::Close() {
    return m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  RiskClientBox::WrappedRiskClient<C>::WrappedRiskClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  InventorySnapshot RiskClientBox::WrappedRiskClient<C>::LoadInventorySnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadInventorySnapshot(account);
  }

  template<typename C>
  void RiskClientBox::WrappedRiskClient<C>::Reset(const Region& region) {
    return m_client->Reset(region);
  }

  template<typename C>
  const RiskPortfolioUpdatePublisher&
      RiskClientBox::WrappedRiskClient<C>::GetRiskPortfolioUpdatePublisher() {
    return m_client->GetRiskPortfolioUpdatePublisher();
  }

  template<typename C>
  void RiskClientBox::WrappedRiskClient<C>::Close() {
    return m_client->Close();
  }
}

#endif
