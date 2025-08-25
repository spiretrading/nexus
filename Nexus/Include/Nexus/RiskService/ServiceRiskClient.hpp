#ifndef NEXUS_SERVICE_RISK_CLIENT_HPP
#define NEXUS_SERVICE_RISK_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/TablePublisher.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <Beam/Utilities/Remote.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/RiskService/RiskClient.hpp"
#include "Nexus/RiskService/RiskServices.hpp"

namespace Nexus {

  /**
   * Implements a RiskClient using Beam services.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ServiceRiskClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs an RiskClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit ServiceRiskClient(BF&& client_builder);

      ~ServiceRiskClient();
      InventorySnapshot load_inventory_snapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void reset(const Region& region);
      const RiskPortfolioUpdatePublisher& get_risk_portfolio_update_publisher();
      void close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Remote<Beam::TablePublisher<RiskPortfolioKey, Inventory>>
        m_publisher;
      Beam::Services::ServiceProtocolClientHandler<B> m_client_handler;
      Beam::IO::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      ServiceRiskClient(const ServiceRiskClient&) = delete;
      ServiceRiskClient& operator =(const ServiceRiskClient&) = delete;
      void on_reconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void on_inventory_update(ServiceProtocolClient& client,
        const std::vector<InventoryUpdate>& inventories);
  };

  template<typename B>
  template<typename BF>
  ServiceRiskClient<B>::ServiceRiskClient(BF&& client_builder)
    BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_client_handler(std::forward<BF>(client_builder),
          std::bind_front(&ServiceRiskClient::on_reconnect, this)) {
    BEAM_UNSUPPRESS_THIS_INITIALIZER()
    RegisterRiskServices(Beam::Store(m_client_handler.GetSlots()));
    RegisterRiskMessages(Beam::Store(m_client_handler.GetSlots()));
    Beam::Services::AddMessageSlot<InventoryMessage>(
      Beam::Store(m_client_handler.GetSlots()),
      std::bind_front(&ServiceRiskClient::on_inventory_update, this));
    m_publisher.SetInitializationFunction([this] (auto& publisher) {
      publisher.emplace();
      m_tasks.Push([this] {
        auto entries = std::vector<RiskInventoryEntry>();
        try {
          auto client = m_client_handler.GetClient();
          entries = client->template SendRequest<
            SubscribeRiskPortfolioUpdatesService>();
        } catch(const std::exception&) {
          m_publisher->Break(Beam::Services::MakeNestedServiceException(
            "Failed to subscribe to portfolio updates."));
          return;
        }
        for(auto& entry : entries) {
          if(entry.m_value.m_transaction_count == 0) {
            m_publisher->Delete(entry);
          } else {
            m_publisher->Push(entry);
          }
        }
      });
    });
  }

  template<typename B>
  ServiceRiskClient<B>::~ServiceRiskClient() {
    close();
  }

  template<typename B>
  InventorySnapshot ServiceRiskClient<B>::load_inventory_snapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadInventorySnapshotService>(
        account);
    }, "Failed to load inventory snapshot: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  void ServiceRiskClient<B>::reset(const Region& region) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      client->template SendRequest<ResetRegionService>(region);
    }, "Failed to reset region.");
  }

  template<typename B>
  const RiskPortfolioUpdatePublisher&
      ServiceRiskClient<B>::get_risk_portfolio_update_publisher() {
    return *m_publisher;
  }

  template<typename B>
  void ServiceRiskClient<B>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_tasks.Break();
    m_tasks.Wait();
    m_client_handler.Close();
    if(m_publisher.IsAvailable()) {
      m_publisher->Break();
    }
    m_open_state.Close();
  }

  template<typename B>
  void ServiceRiskClient<B>::on_reconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_tasks.Push([=, this] {
      if(!m_publisher.IsAvailable()) {
        return;
      }
      auto entries = std::vector<RiskInventoryEntry>();
      try {
        entries = client->template SendRequest<
          SubscribeRiskPortfolioUpdatesService>();
      } catch(const std::exception&) {
        m_publisher->Break(Beam::Services::MakeNestedServiceException(
          "Unable to recover risk portfolio updates."));
        return;
      }
      if(auto snapshot = m_publisher->GetSnapshot()) {
        for(auto& snapshot_entry : *snapshot) {
          auto entry_iterator = std::find_if(entries.begin(), entries.end(),
            [&] (const auto& entry) {
              return entry.m_key == snapshot_entry.first;
            });
          if(entry_iterator == entries.end()) {
            m_publisher->Delete(snapshot_entry.first,
              Inventory(get_key(snapshot_entry.second.m_position)));
          }
        }
      }
      for(auto& entry : entries) {
        m_publisher->Push(entry);
      }
    });
  }

  template<typename B>
  void ServiceRiskClient<B>::on_inventory_update(ServiceProtocolClient& client,
      const std::vector<InventoryUpdate>& inventories) {
    m_tasks.Push([=, this] {
      for(auto& update : inventories) {
        if(update.inventory.m_transaction_count == 0) {
          m_publisher->Delete(RiskPortfolioKey(update.account,
            update.inventory.m_position.m_security), update.inventory);
        } else {
          m_publisher->Push(RiskPortfolioKey(update.account,
            update.inventory.m_position.m_security), update.inventory);
        }
      }
    });
  }
}

#endif
