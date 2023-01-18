#ifndef NEXUS_RISK_CLIENT_HPP
#define NEXUS_RISK_CLIENT_HPP
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/TablePublisher.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <Beam/Utilities/Remote.hpp>
#include <boost/lexical_cast.hpp>
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
  class RiskClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs an RiskClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit RiskClient(BF&& clientBuilder);

      ~RiskClient();

      /**
       * Loads the InventorySnapshot of a given account.
       * @param account The account to load.
       * @return The <i>account</i>'s InventorySnapshot.
       */
      InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Resets the inventories belonging to a region for all accounts.
       * @param region The region to reset.
       */
      void Reset(const Region& region);

      /** Returns the object publishing a RiskPortfolioUpdates. */
      const RiskPortfolioUpdatePublisher& GetRiskPortfolioUpdatePublisher();

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Remote<Beam::TablePublisher<RiskPortfolioKey, RiskInventory>>
        m_publisher;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      RiskClient(const RiskClient&) = delete;
      RiskClient& operator =(const RiskClient&) = delete;
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void OnInventoryUpdate(ServiceProtocolClient& client,
        const std::vector<InventoryUpdate>& inventories);
  };

  template<typename B>
  template<typename BF>
  RiskClient<B>::RiskClient(BF&& clientBuilder)
    BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_clientHandler(std::forward<BF>(clientBuilder),
              std::bind_front(&RiskClient::OnReconnect, this)) {
    BEAM_UNSUPPRESS_THIS_INITIALIZER()
    RegisterRiskServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterRiskMessages(Beam::Store(m_clientHandler.GetSlots()));
    Beam::Services::AddMessageSlot<InventoryMessage>(
      Beam::Store(m_clientHandler.GetSlots()),
      std::bind_front(&RiskClient::OnInventoryUpdate, this));
    m_publisher.SetInitializationFunction([this] (auto& publisher) {
      publisher.emplace();
      m_tasks.Push([this] {
        auto entries = std::vector<RiskInventoryEntry>();
        try {
          auto client = m_clientHandler.GetClient();
          entries = client->template SendRequest<
            SubscribeRiskPortfolioUpdatesService>();
        } catch(const std::exception&) {
          m_publisher->Break(Beam::Services::MakeNestedServiceException(
            "Failed to subscribe to portfolio updates."));
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
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the risk server."));
  }

  template<typename B>
  RiskClient<B>::~RiskClient() {
    Close();
  }

  template<typename B>
  InventorySnapshot RiskClient<B>::LoadInventorySnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadInventorySnapshotService>(
        account);
    }, "Failed to load inventory snapshot: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  void RiskClient<B>::Reset(const Region& region) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<ResetRegionService>(region);
    }, "Failed to reset region.");
  }

  template<typename B>
  const RiskPortfolioUpdatePublisher&
      RiskClient<B>::GetRiskPortfolioUpdatePublisher() {
    return *m_publisher;
  }

  template<typename B>
  void RiskClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_tasks.Break();
    m_tasks.Wait();
    m_clientHandler.Close();
    if(m_publisher.IsAvailable()) {
      m_publisher->Break();
    }
    m_openState.Close();
  }

  template<typename B>
  void RiskClient<B>::OnReconnect(
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
        for(auto& snapshotEntry : *snapshot) {
          auto entryIterator = std::find_if(entries.begin(), entries.end(),
            [&] (auto& entry) {
              return entry.m_key == snapshotEntry.first;
            });
          if(entryIterator == entries.end()) {
            m_publisher->Delete(snapshotEntry.first,
              RiskInventory(snapshotEntry.second.m_position.m_key));
          }
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
    m_tasks.Push([=, this] {
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
