#ifndef NEXUS_RISK_CLIENT_HPP
#define NEXUS_RISK_CLIENT_HPP
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/TablePublisher.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Utilities/Remote.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskServices.hpp"

namespace Nexus::RiskService {

  /**
   * Client used to access the Risk service.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class RiskClient : private boost::noncopyable {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder =
        Beam::GetTryDereferenceType<B>;

      /**
       * Constructs an RiskClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      RiskClient(BF&& clientBuilder);

      ~RiskClient();

      /** Returns the object publishing a RiskPortfolioUpdates. */
      const RiskPortfolioUpdatePublisher& GetRiskPortfolioUpdatePublisher();

      void Open();

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Remote<Beam::TablePublisher<RiskPortfolioKey,
        RiskPortfolioInventory>> m_publisher;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void OnInventoryUpdate(ServiceProtocolClient& client,
        const std::vector<InventoryUpdate>& inventories);
  };

  template<typename B>
  template<typename BF>
  RiskClient<B>::RiskClient(BF&& clientBuilder)
      : m_clientHandler(std::forward<BF>(clientBuilder)) {
    m_clientHandler.SetReconnectHandler(
      std::bind(&RiskClient::OnReconnect, this, std::placeholders::_1));
    RegisterRiskServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterRiskMessages(Beam::Store(m_clientHandler.GetSlots()));
    Beam::Services::AddMessageSlot<InventoryMessage>(
      Beam::Store(m_clientHandler.GetSlots()),
      std::bind(&RiskClient::OnInventoryUpdate, this, std::placeholders::_1,
      std::placeholders::_2));
    m_publisher.SetInitializationFunction(
      [=] (auto& publisher) {
        publisher.emplace();
        m_tasks.Push(
          [=] {
            auto entries = std::vector<RiskPortfolioInventoryEntry>();
            try {
              auto client = m_clientHandler.GetClient();
              entries = client->template SendRequest<
                SubscribeRiskPortfolioUpdatesService>(0);
            } catch(const std::exception&) {
              m_publisher->Break(std::current_exception());
              return;
            }
            for(auto& entry : entries) {
              if(entry.m_value.m_transactionCount == 0) {
                m_publisher->Delete(entry);
              } else {
                m_publisher->Push(entry);
              }
            }
          });
      });
  }

  template<typename B>
  RiskClient<B>::~RiskClient() {
    Close();
  }

  template<typename B>
  const RiskPortfolioUpdatePublisher& RiskClient<B>::
      GetRiskPortfolioUpdatePublisher() {
    return *m_publisher;
  }

  template<typename B>
  void RiskClient<B>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_clientHandler.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename B>
  void RiskClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename B>
  void RiskClient<B>::Shutdown() {
    m_clientHandler.Close();
    m_tasks.Break();
    m_openState.SetClosed();
  }

  template<typename B>
  void RiskClient<B>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_tasks.Push(
      [=] {
        if(!m_publisher.IsAvailable()) {
          return;
        }
        auto entries = std::vector<RiskPortfolioInventoryEntry>();
        try {
          entries = client->template SendRequest<
            SubscribeRiskPortfolioUpdatesService>(0);
        } catch(const std::exception&) {
          m_publisher->Break(std::current_exception());
          return;
        }
        auto snapshot = Beam::TablePublisher<
          RiskPortfolioKey, RiskPortfolioInventory>::Snapshot();
        m_publisher->WithSnapshot(
          [&] (auto publisherSnapshot) {
            if(publisherSnapshot.is_initialized()) {
              snapshot = *publisherSnapshot;
            }
          });
        for(auto& snapshotEntry : snapshot) {
          auto entryIterator = std::find_if(entries.begin(), entries.end(),
            [&] (auto& entry) {
              return entry.m_key == snapshotEntry.first;
            });
          if(entryIterator == entries.end()) {
            m_publisher->Delete(snapshotEntry.first,
              RiskPortfolioInventory(snapshotEntry.second.m_position.m_key));
          }
        }
        for(auto& entry : entries) {
          m_publisher->Push(entry);
        }
      });
  }

  template<typename B>
  void RiskClient<B>::OnInventoryUpdate(ServiceProtocolClient& client,
      const std::vector<InventoryUpdate>& inventories) {
    m_tasks.Push(
      [=] {
        for(auto& update : inventories) {
          if(update.inventory.m_transactionCount == 0) {
            m_publisher->Delete(RiskPortfolioKey(update.account,
              update.inventory.m_position.m_key.m_index), update.inventory);
          } else {
            m_publisher->Push(RiskPortfolioKey(update.account,
              update.inventory.m_position.m_key.m_index), update.inventory);
          }
        }
      });
  }
}

#endif
