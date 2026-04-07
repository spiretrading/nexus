#ifndef NEXUS_RISK_CLIENT_HPP
#define NEXUS_RISK_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus {

  /** Checks if a type implements a RiskClient. */
  template<typename T>
  concept IsRiskClient = requires(T& client,
      const Beam::DirectoryEntry& account, const Region& region) {
    { client.load_inventory_snapshot(account) } ->
      std::same_as<InventorySnapshot>;
    client.reset(region);
    { client.get_risk_portfolio_update_publisher() } ->
      std::same_as<const RiskPortfolioUpdatePublisher&>;
    client.close();
  };

  /** Provides a generic interface over an arbitrary RiskClient. */
  class RiskClient {
    public:

      /**
       * Constructs a RiskClient of a specified type using emplacement.
       * @tparam T The type of risk client to emplace.
       * @param args The arguments to pass to the emplaced risk client.
       */
      template<IsRiskClient T, typename... Args>
      explicit RiskClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a RiskClient by referencing an existing risk client.
       * @param client The client to reference.
       */
      template<Beam::DisableCopy<RiskClient> T> requires
        IsRiskClient<Beam::dereference_t<T>>
      RiskClient(T&& client);

      RiskClient(const RiskClient&) = default;
      RiskClient(RiskClient&&) = default;

      /**
       * Loads the InventorySnapshot of a given account.
       * @param account The account to load.
       * @return The <i>account</i>'s InventorySnapshot.
       */
      InventorySnapshot load_inventory_snapshot(
        const Beam::DirectoryEntry& account);

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
          const Beam::DirectoryEntry& account) = 0;
        virtual void reset(const Region& region) = 0;
        virtual const RiskPortfolioUpdatePublisher&
          get_risk_portfolio_update_publisher() = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedRiskClient final : VirtualRiskClient {
        using RiskClient = C;
        Beam::local_ptr_t<RiskClient> m_client;

        template<typename... Args>
        WrappedRiskClient(Args&&... args);

        InventorySnapshot load_inventory_snapshot(
          const Beam::DirectoryEntry& account) override;
        void reset(const Region& region) override;
        const RiskPortfolioUpdatePublisher&
          get_risk_portfolio_update_publisher() override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualRiskClient> m_client;
  };

  template<IsRiskClient T, typename... Args>
  RiskClient::RiskClient(std::in_place_type_t<T>, Args&&... args)
    : m_client(Beam::make_virtual_ptr<WrappedRiskClient<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<RiskClient> T> requires
    IsRiskClient<Beam::dereference_t<T>>
  RiskClient::RiskClient(T&& client)
    : m_client(Beam::make_virtual_ptr<
        WrappedRiskClient<std::remove_cvref_t<T>>>(std::forward<T>(client))) {}

  inline InventorySnapshot RiskClient::load_inventory_snapshot(
      const Beam::DirectoryEntry& account) {
    return m_client->load_inventory_snapshot(account);
  }

  inline void RiskClient::reset(const Region& region) {
    m_client->reset(region);
  }

  inline const RiskPortfolioUpdatePublisher&
      RiskClient::get_risk_portfolio_update_publisher() {
    return m_client->get_risk_portfolio_update_publisher();
  }

  inline void RiskClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  RiskClient::WrappedRiskClient<C>::WrappedRiskClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  InventorySnapshot RiskClient::WrappedRiskClient<C>::load_inventory_snapshot(
      const Beam::DirectoryEntry& account) {
    return m_client->load_inventory_snapshot(account);
  }

  template<typename C>
  void RiskClient::WrappedRiskClient<C>::reset(const Region& region) {
    m_client->reset(region);
  }

  template<typename C>
  const RiskPortfolioUpdatePublisher&
      RiskClient::WrappedRiskClient<C>::get_risk_portfolio_update_publisher() {
    return m_client->get_risk_portfolio_update_publisher();
  }

  template<typename C>
  void RiskClient::WrappedRiskClient<C>::close() {
    m_client->close();
  }
}

#endif
