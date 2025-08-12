#ifndef NEXUS_RISK_CLIENT_HPP
#define NEXUS_RISK_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus::RiskService {

  /** Provides a generic interface over an arbitrary RiskClient. */
  class RiskClient {
    public:

      /**
       * Constructs a RiskClient of a specified type using emplacement.
       * @param <T> The type of risk client to emplace.
       * @param args The arguments to pass to the emplaced risk client.
       */
      template<typename T, typename... Args>
      explicit RiskClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a RiskClient by copying an existing risk client.
       * @param client The client to copy.
       */
      template<typename C>
      explicit RiskClient(C client);

      explicit RiskClient(RiskClient* client);

      explicit RiskClient(const std::shared_ptr<RiskClient>& client);

      explicit RiskClient(const std::unique_ptr<RiskClient>& client);

      /**
       * Loads the InventorySnapshot of a given account.
       * @param account The account to load.
       * @return The <i>account</i>'s InventorySnapshot.
       */
      InventorySnapshot load_inventory_snapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Resets the inventories belonging to a region for all accounts.
       * @param region The region to reset.
       */
      void reset(const Region& region);

      /** Returns the object publishing a RiskPortfolioUpdates. */
      const RiskPortfolioUpdatePublisher& get_risk_portfolio_update_publisher();

      void close();

    private:
      struct VirtualRiskClient {
        virtual ~VirtualRiskClient() = default;
        virtual InventorySnapshot load_inventory_snapshot(
          const Beam::ServiceLocator::DirectoryEntry&) = 0;
        virtual void reset(const Region&) = 0;
        virtual const RiskPortfolioUpdatePublisher&
          get_risk_portfolio_update_publisher() = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedRiskClient final : VirtualRiskClient {
        using RiskClient = C;
        Beam::GetOptionalLocalPtr<RiskClient> m_client;

        template<typename... Args>
        WrappedRiskClient(Args&&... args);
        InventorySnapshot load_inventory_snapshot(
          const Beam::ServiceLocator::DirectoryEntry& account) override;
        void reset(const Region& region) override;
        const RiskPortfolioUpdatePublisher&
          get_risk_portfolio_update_publisher() override;
        void close() override;
      };
      std::shared_ptr<VirtualRiskClient> m_client;
  };

  /** Checks if a type implements a RiskClient. */
  template<typename T>
  concept IsRiskClient = std::constructible_from<
    RiskClient, std::remove_pointer_t<std::remove_cvref_t<T>>*>;

  template<typename T, typename... Args>
  RiskClient::RiskClient(std::in_place_type_t<T>, Args&&... args)
    : m_client(std::make_shared<WrappedRiskClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename C>
  RiskClient::RiskClient(C client)
    : RiskClient(std::in_place_type<C>, std::move(client)) {}

  inline RiskClient::RiskClient(RiskClient* client)
    : RiskClient(*client) {}

  inline RiskClient::RiskClient(const std::shared_ptr<RiskClient>& client)
    : RiskClient(*client) {}

  inline RiskClient::RiskClient(const std::unique_ptr<RiskClient>& client)
    : RiskClient(*client) {}

  inline InventorySnapshot RiskClient::load_inventory_snapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_inventory_snapshot(account);
  }

  inline void RiskClient::reset(const Region& region) {
    return m_client->reset(region);
  }

  inline const RiskPortfolioUpdatePublisher&
      RiskClient::get_risk_portfolio_update_publisher() {
    return m_client->get_risk_portfolio_update_publisher();
  }

  inline void RiskClient::close() {
    return m_client->close();
  }

  template<typename C>
  template<typename... Args>
  RiskClient::WrappedRiskClient<C>::WrappedRiskClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  InventorySnapshot RiskClient::WrappedRiskClient<C>::load_inventory_snapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_inventory_snapshot(account);
  }

  template<typename C>
  void RiskClient::WrappedRiskClient<C>::reset(const Region& region) {
    return m_client->reset(region);
  }

  template<typename C>
  const RiskPortfolioUpdatePublisher&
      RiskClient::WrappedRiskClient<C>::get_risk_portfolio_update_publisher() {
    return m_client->get_risk_portfolio_update_publisher();
  }

  template<typename C>
  void RiskClient::WrappedRiskClient<C>::close() {
    return m_client->close();
  }
}

#endif
