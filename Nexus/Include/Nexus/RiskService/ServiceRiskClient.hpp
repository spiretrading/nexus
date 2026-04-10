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
      using ServiceProtocolClientBuilder = Beam::dereference_t<B>;

      /**
       * Constructs an RiskClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit ServiceRiskClient(BF&& client_builder);

      ~ServiceRiskClient();
      InventorySnapshot load_inventory_snapshot(
        const Beam::DirectoryEntry& account);
      void reset(const Scope& scope);
      const RiskPortfolioUpdatePublisher& get_risk_portfolio_update_publisher();
      void close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Remote<Beam::TablePublisher<RiskPortfolioKey, Inventory>>
        m_publisher;
      Beam::ServiceProtocolClientHandler<B> m_client_handler;
      Beam::OpenState m_open_state;
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
    register_risk_services(out(m_client_handler.get_slots()));
    register_risk_messages(out(m_client_handler.get_slots()));
    Beam::add_message_slot<InventoryMessage>(out(m_client_handler.get_slots()),
      std::bind_front(&ServiceRiskClient::on_inventory_update, this));
    m_publisher.set_initializer([this] (auto& publisher) {
      publisher.emplace();
      m_tasks.push([this] {
        auto entries = std::vector<RiskInventoryEntry>();
        try {
          auto client = m_client_handler.get_client();
          entries = client->template send_request<
            SubscribeRiskPortfolioUpdatesService>();
        } catch(const std::exception&) {
          m_publisher->close(Beam::make_nested_service_exception(
            "Failed to subscribe to portfolio updates."));
          return;
        }
        for(auto& entry : entries) {
          if(entry.m_value.m_transaction_count == 0 &&
              entry.m_value.m_position.m_quantity == 0) {
            m_publisher->erase(entry.m_key);
          } else {
            m_publisher->push(entry);
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
      const Beam::DirectoryEntry& account) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadInventorySnapshotService>(
        account);
    }, "Failed to load inventory snapshot: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  void ServiceRiskClient<B>::reset(const Scope& scope) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<ResetScopeService>(scope);
    }, "Failed to reset scope.");
  }

  template<typename B>
  const RiskPortfolioUpdatePublisher&
      ServiceRiskClient<B>::get_risk_portfolio_update_publisher() {
    return *m_publisher;
  }

  template<typename B>
  void ServiceRiskClient<B>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_tasks.close();
    m_tasks.wait();
    m_client_handler.close();
    if(m_publisher.is_available()) {
      m_publisher->close();
    }
    m_open_state.close();
  }

  template<typename B>
  void ServiceRiskClient<B>::on_reconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_tasks.push([=, this] {
      if(!m_publisher.is_available()) {
        return;
      }
      auto entries = std::vector<RiskInventoryEntry>();
      try {
        entries = client->template send_request<
          SubscribeRiskPortfolioUpdatesService>();
      } catch(const std::exception&) {
        m_publisher->close(Beam::make_nested_service_exception(
          "Unable to recover risk portfolio updates."));
        return;
      }
      if(auto snapshot = m_publisher->get_snapshot()) {
        for(auto& snapshot_entry : *snapshot) {
          auto entry_iterator = std::find_if(entries.begin(), entries.end(),
            [&] (const auto& entry) {
              return entry.m_key == snapshot_entry.first;
            });
          if(entry_iterator == entries.end()) {
            m_publisher->erase(snapshot_entry.first,
              Inventory(get_key(snapshot_entry.second.m_position)));
          }
        }
      }
      for(auto& entry : entries) {
        m_publisher->push(entry);
      }
    });
  }

  template<typename B>
  void ServiceRiskClient<B>::on_inventory_update(ServiceProtocolClient& client,
      const std::vector<InventoryUpdate>& inventories) {
    m_tasks.push([=, this] {
      for(auto& update : inventories) {
        if(update.inventory.m_transaction_count == 0) {
          m_publisher->erase(RiskPortfolioKey(update.account,
            update.inventory.m_position.m_ticker), update.inventory);
        } else {
          m_publisher->push(RiskPortfolioKey(update.account,
            update.inventory.m_position.m_ticker), update.inventory);
        }
      }
    });
  }
}

#endif
