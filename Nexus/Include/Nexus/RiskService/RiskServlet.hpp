#ifndef NEXUS_RISK_SERVLET_HPP
#define NEXUS_RISK_SERVLET_HPP
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

namespace Nexus::RiskService {

  /**
   * Monitors a trader's positions and orders for risk compliance.
   * @param <C> The container instantiating this servlet.
   * @param <A> The type of AdministrationClient used to access account info.
   * @param <O> The type of OrderExecutionClient used to manage a trader's
   *        position.
   * @param <R> The type of RiskStateMonitor used for risk monitoring.
   */
  template<typename C, typename A, typename O, typename R>
  class RiskServlet : private boost::noncopyable {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of AdministrationClient used to access account info. */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /**
       * The type of OrderExecutionClient used to manage a trader's position.
       */
      using OrderExecutionClient = Beam::GetTryDereferenceType<O>;

      /** The type of RiskStateMonitor used for risk monitoring. */
      using ServletRiskStateMonitor = Beam::GetTryDereferenceType<R>;

      /** The type of RiskTransitionController used to transition accounts. */
      using RiskTransitionController = RiskService::RiskTransitionController<
        AdministrationClient*, OrderExecutionClient*>;

      /**
       * Constructs a RiskServlet.
       * @param orderQueue The Queue publishing Orders to monitor.
       * @param administrationClient Initializes the AdministrationClient.
       * @param orderExecutionClient Initializes the OrderExecutionClient.
       * @param riskStateMonitor Initializes the RiskStateMonitor.
       * @param destinations The database of available destinations.
       * @param markets The database of available markets.
       */
      template<typename AF, typename OF, typename RF>
      RiskServlet(const std::shared_ptr<Beam::QueueReader<
        const OrderExecutionService::Order*>>& orderQueue,
        AF&& administrationClient, OF&& orderExecutionClient,
        RF&& riskStateMonitor, const DestinationDatabase& destinations,
        const MarketDatabase& markets);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::GetOptionalLocalPtr<O> m_orderExecutionClient;
      Beam::GetOptionalLocalPtr<R> m_riskStateMonitor;
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
        ServiceProtocolClient, SubscribeRiskPortfolioUpdatesService>& request,
        int dummy);
  };

  template<typename A, typename O, typename R>
  struct MetaRiskServlet {
    using Session = RiskSession;
    template<typename C>
    struct apply {
      using type = RiskServlet<C, A, O, R>;
    };
  };

  template<typename C, typename A, typename O, typename R>
  template<typename AF, typename OF, typename RF>
  RiskServlet<C, A, O, R>::RiskServlet(const std::shared_ptr<
    Beam::QueueReader<const OrderExecutionService::Order*>>& orderQueue,
    AF&& administrationClient, OF&& orderExecutionClient, RF&& riskStateMonitor,
    const DestinationDatabase& destinations, const MarketDatabase& markets)
    : m_administrationClient(std::forward<AF>(administrationClient)),
      m_orderExecutionClient(std::forward<OF>(orderExecutionClient)),
      m_riskStateMonitor(std::forward<RF>(riskStateMonitor)),
      m_riskTransitionController(orderQueue, &*m_administrationClient,
        &*m_orderExecutionClient, destinations, markets) {}

  template<typename C, typename A, typename O, typename R>
  void RiskServlet<C, A, O, R>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterRiskServices(Store(slots));
    RegisterRiskMessages(Store(slots));
    SubscribeRiskPortfolioUpdatesService::AddRequestSlot(Store(slots),
      std::bind(&RiskServlet::OnSubscribeRiskPortfolioUpdatesRequest, this,
      std::placeholders::_1, std::placeholders::_2));
  }

  template<typename C, typename A, typename O, typename R>
  void RiskServlet<C, A, O, R>::HandleClientClosed(
      ServiceProtocolClient& client) {
    Beam::Threading::With(m_portfolioSubscribers,
      [&] (auto& subscribers) {
        subscribers.erase(std::remove(subscribers.begin(),
          subscribers.end(), &client), subscribers.end());
      });
  }

  template<typename C, typename A, typename O, typename R>
  void RiskServlet<C, A, O, R>::Open() {
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

  template<typename C, typename A, typename O, typename R>
  void RiskServlet<C, A, O, R>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C, typename A, typename O, typename R>
  void RiskServlet<C, A, O, R>::Shutdown() {
    m_riskTransitionController.Close();
    m_administrationClient->Close();
    m_orderExecutionClient->Close();
    m_tasks.Break();
    m_openState.SetClosed();
  }

  template<typename C, typename A, typename O, typename R>
  Beam::ServiceLocator::DirectoryEntry RiskServlet<C, A, O, R>::LoadGroup(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_accountToGroup.GetOrInsert(account,
      [&] {
        return m_administrationClient->LoadTradingGroupEntry(account);
      });
  }

  template<typename C, typename A, typename O, typename R>
  void RiskServlet<C, A, O, R>::OnInventoryUpdate(
      const RiskPortfolioInventoryEntry& entry) {
    auto& volume = m_volume[entry.m_key];
    if(volume == entry.m_value.m_volume) {
      return;
    }
    volume = entry.m_value.m_volume;
    auto inventories = std::vector<InventoryUpdate>();
    auto update = InventoryUpdate();
    update.account = entry.m_key.m_account;
    update.inventory = entry.m_value;
    inventories.push_back(update);
    auto group = LoadGroup(update.account);
    Beam::Threading::With(m_portfolioSubscribers,
      [&] (const auto& subscribers) {
        for(auto& client : subscribers) {
          auto& session = client->GetSession();
          if(session.HasGroupPortfolioSubscription(group)) {
            Beam::Services::SendRecordMessage<InventoryMessage>(*client,
              inventories);
          }
        }
      });
  }

  template<typename C, typename A, typename O, typename R>
  void RiskServlet<C, A, O, R>::OnRiskState(const RiskStateEntry& entry) {
    m_administrationClient->StoreRiskState(entry.m_key, entry.m_value);
  }

  template<typename C, typename A, typename O, typename R>
  void RiskServlet<C, A, O, R>::OnSubscribeRiskPortfolioUpdatesRequest(
      Beam::Services::RequestToken<
      ServiceProtocolClient, SubscribeRiskPortfolioUpdatesService>& request,
      int dummy) {
    auto& session = request.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    auto groups = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    if(!isAdministrator) {
      groups = m_administrationClient->LoadManagedTradingGroups(
        session.GetAccount());
    }
    auto entries = std::vector<RiskPortfolioInventoryEntry>();
    Beam::Threading::With(m_portfolioSubscribers,
      [&] (auto& subscribers) {
        if(isAdministrator) {
          session.AddAllPortfolioGroups();
        } else {
          for(auto& group : groups) {
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

#endif
