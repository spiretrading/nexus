#ifndef NEXUS_RISKSERVLET_HPP
#define NEXUS_RISKSERVLET_HPP
#include <algorithm>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskServices.hpp"
#include "Nexus/RiskService/RiskSession.hpp"
#include "Nexus/RiskService/RiskStateMonitor.hpp"
#include "Nexus/RiskService/RiskTransitionController.hpp"

namespace Nexus {
namespace RiskService {

  /*! \class RiskServlet
      \brief Monitors a trader's positions and orders for risk compliance.
      \tparam ContainerType The container instantiating this servlet.
      \tparam AdministrationClientType The type of AdministrationClient used to
              access account info.
      \tparam OrderExecutionClientType The type of OrderExecutionClient used to
              manage a trader's position.
      \tparam RiskStateMonitorType The type of RiskStateMonitor used for risk
              monitoring.
   */
  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
  class RiskServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! The type of AdministrationClient used to access account info.
      using AdministrationClient =
        Beam::GetTryDereferenceType<AdministrationClientType>;

      //! The type of OrderExecutionClient used to manage a trader's position.
      using OrderExecutionClient =
        Beam::GetTryDereferenceType<OrderExecutionClientType>;

      //! The type of RiskStateMonitor used for risk monitoring.
      using ServletRiskStateMonitor =
        Beam::GetTryDereferenceType<RiskStateMonitorType>;

      //! The type of RiskTransitionController used to transition accounts.
      using RiskTransitionController = RiskService::RiskTransitionController<
        AdministrationClient*, OrderExecutionClient*>;

      //! Constructs a RiskServlet.
      /*!
        \param orderQueue The Queue publishing Orders to monitor.
        \param administrationClient Initializes the AdministrationClient.
        \param orderExecutionClient Initializes the OrderExecutionClient.
        \param riskStateMonitor Initializes the RiskStateMonitor.
        \param destinations The database of available destinations.
        \param markets The database of available markets.
      */
      template<typename AdministrationClientForward,
        typename OrderExecutionClientForward, typename RiskStateMonitorForward>
      RiskServlet(const std::shared_ptr<Beam::QueueReader<
        const OrderExecutionService::Order*>>& orderQueue,
        AdministrationClientForward&& administrationClient,
        OrderExecutionClientForward&& orderExecutionClient,
        RiskStateMonitorForward&& riskStateMonitor,
        const DestinationDatabase& destinations, const MarketDatabase& markets);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

      void Close();

    private:
      typename Beam::OptionalLocalPtr<AdministrationClientType>::type
        m_administrationClient;
      typename Beam::OptionalLocalPtr<OrderExecutionClientType>::type
        m_orderExecutionClient;
      typename Beam::OptionalLocalPtr<RiskStateMonitorType>::type
        m_riskStateMonitor;
      RiskTransitionController m_riskTransitionController;
      Beam::Threading::Sync<std::vector<ServiceProtocolClient*>,
        Beam::Threading::Mutex> m_portfolioSubscribers;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        Beam::ServiceLocator::DirectoryEntry, Beam::Threading::Mutex>
        m_accountToGroup;
      std::unordered_map<RiskPortfolioKey, Quantity> m_volume;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      Beam::ServiceLocator::DirectoryEntry LoadGroup(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void OnRiskState(const RiskStateEntry& entry);
      void OnInventoryUpdate(const RiskPortfolioInventoryEntry& entry);
      void OnSubscribeRiskPortfolioUpdatesRequest(Beam::Services::RequestToken<
        ServiceProtocolClient, SubscribeRiskPortfolioUpdatesService>& request);
  };

  template<typename AdministrationClientType, typename OrderExecutionClientType,
    typename RiskStateMonitorType>
  struct MetaRiskServlet {
    using Session = RiskSession;
    template<typename ContainerType>
    struct apply {
      using type = RiskServlet<ContainerType, AdministrationClientType,
        OrderExecutionClientType, RiskStateMonitorType>;
    };
  };

  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
  template<typename AdministrationClientForward,
    typename OrderExecutionClientForward, typename RiskStateMonitorForward>
  RiskServlet<ContainerType, AdministrationClientType, OrderExecutionClientType,
      RiskStateMonitorType>::RiskServlet(const std::shared_ptr<
      Beam::QueueReader<const OrderExecutionService::Order*>>& orderQueue,
      AdministrationClientForward&& administrationClient,
      OrderExecutionClientForward&& orderExecutionClient,
      RiskStateMonitorForward&& riskStateMonitor,
      const DestinationDatabase& destinations, const MarketDatabase& markets)
      : m_administrationClient(std::forward<AdministrationClientForward>(
          administrationClient)),
        m_orderExecutionClient(std::forward<OrderExecutionClientForward>(
          orderExecutionClient)),
        m_riskStateMonitor(std::forward<RiskStateMonitorForward>(
          riskStateMonitor)),
        m_riskTransitionController(orderQueue, &*m_administrationClient,
          &*m_orderExecutionClient, destinations, markets) {}

  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
  void RiskServlet<ContainerType, AdministrationClientType,
      OrderExecutionClientType, RiskStateMonitorType>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterRiskServices(Store(slots));
    RegisterRiskMessages(Store(slots));
    SubscribeRiskPortfolioUpdatesService::AddRequestSlot(Store(slots),
      std::bind(&RiskServlet::OnSubscribeRiskPortfolioUpdatesRequest, this,
      std::placeholders::_1));
  }

  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
  void RiskServlet<ContainerType, AdministrationClientType,
      OrderExecutionClientType, RiskStateMonitorType>::HandleClientClosed(
      ServiceProtocolClient& client) {
    Beam::Threading::With(m_portfolioSubscribers,
      [&] (std::vector<ServiceProtocolClient*>& subscribers) {
        subscribers.erase(std::remove(subscribers.begin(),
          subscribers.end(), &client), subscribers.end());
      });
  }

  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
  void RiskServlet<ContainerType, AdministrationClientType,
      OrderExecutionClientType, RiskStateMonitorType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_administrationClient->Open();
      m_orderExecutionClient->Open();
      m_riskTransitionController.Open();
      m_riskStateMonitor->GetRiskStatePublisher().Monitor(
        m_tasks.GetSlot<RiskStateEntry>(std::bind(&RiskServlet::OnRiskState,
        this, std::placeholders::_1)));
      m_riskStateMonitor->GetInventoryPublisher().Monitor(
        m_tasks.GetSlot<RiskPortfolioInventoryEntry>(std::bind(
        &RiskServlet::OnInventoryUpdate, this, std::placeholders::_1)));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
  void RiskServlet<ContainerType, AdministrationClientType,
      OrderExecutionClientType, RiskStateMonitorType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
  void RiskServlet<ContainerType, AdministrationClientType,
      OrderExecutionClientType, RiskStateMonitorType>::Shutdown() {
    m_riskTransitionController.Close();
    m_administrationClient->Close();
    m_orderExecutionClient->Close();
    m_tasks.Break();
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
  Beam::ServiceLocator::DirectoryEntry RiskServlet<ContainerType,
      AdministrationClientType, OrderExecutionClientType,
      RiskStateMonitorType>::LoadGroup(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_accountToGroup.GetOrInsert(account,
      [&] {
        return m_administrationClient->LoadParentTradingGroup(account);
      });
  }

  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
  void RiskServlet<ContainerType, AdministrationClientType,
      OrderExecutionClientType, RiskStateMonitorType>::OnInventoryUpdate(
      const RiskPortfolioInventoryEntry& entry) {
    auto& volume = m_volume[entry.m_key];
    if(volume == entry.m_value.m_volume) {
      return;
    }
    volume = entry.m_value.m_volume;
    std::vector<InventoryUpdate> inventories;
    InventoryUpdate update;
    update.account = entry.m_key.m_account;
    update.inventory = entry.m_value;
    inventories.push_back(update);
    auto group = LoadGroup(update.account);
    Beam::Threading::With(m_portfolioSubscribers,
      [&] (const std::vector<ServiceProtocolClient*>& subscribers) {
        for(const auto& client : subscribers) {
          auto& session = client->GetSession();
          if(session.HasGroupPortfolioSubscription(group)) {
            Beam::Services::SendRecordMessage<InventoryMessage>(*client,
              inventories);
          }
        }
      });
  }

  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
  void RiskServlet<ContainerType, AdministrationClientType,
      OrderExecutionClientType, RiskStateMonitorType>::OnRiskState(
      const RiskStateEntry& entry) {
    m_administrationClient->StoreRiskState(entry.m_key, entry.m_value);
  }

  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
  void RiskServlet<ContainerType, AdministrationClientType,
      OrderExecutionClientType, RiskStateMonitorType>::
      OnSubscribeRiskPortfolioUpdatesRequest(Beam::Services::RequestToken<
      ServiceProtocolClient, SubscribeRiskPortfolioUpdatesService>& request) {
    auto& session = request.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    std::vector<Beam::ServiceLocator::DirectoryEntry> groups;
    if(!isAdministrator) {
      groups = m_administrationClient->LoadManagedTradingGroups(
        session.GetAccount());
    }
    std::vector<RiskPortfolioInventoryEntry> entries;
    Beam::Threading::With(m_portfolioSubscribers,
      [&] (std::vector<ServiceProtocolClient*>& subscribers) {
        if(isAdministrator) {
          session.AddAllPortfolioGroups();
        } else {
          for(const auto& group : groups) {
            session.AddPortfolioGroup(group);
          }
        }
        subscribers.push_back(&request.GetClient());
        auto queue =
          std::make_shared<Beam::Queue<RiskPortfolioInventoryEntry>>();
        m_riskStateMonitor->GetInventoryPublisher().With(
          [&] {
            m_riskStateMonitor->GetInventoryPublisher().Monitor(queue);
            queue->Break();
          });
        while(!queue->IsEmpty()) {
          auto entry = queue->Top();
          queue->Pop();
          auto group = LoadGroup(entry.m_key.m_account);
          if(session.HasGroupPortfolioSubscription(group)) {
            entries.push_back(entry);
          }
        }
        request.SetResult(entries);
      });
  }
}
}

#endif
