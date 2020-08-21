#ifndef NEXUS_RISK_SERVLET_HPP
#define NEXUS_RISK_SERVLET_HPP
#include <unordered_map>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/QueueReaderPublisher.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/SnapshotPublisher.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/RiskService/ConsolidatedRiskController.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskServices.hpp"
#include "Nexus/RiskService/RiskSession.hpp"

namespace Nexus::RiskService {

  /**
   * Monitors a trader's positions and orders for risk compliance.
   * @param <C> The container instantiating this servlet.
   * @param <A> The type of AdministrationClient to use.
   * @param <M> The type of MarketDataClient to use.
   * @param <O> The type of OrderExecutionClient to use.
   * @param <R> The type of Timer to use to transition from CLOSED_ORDERS to
   *        DISABLED.
   * @param <T> The type of TimeClient to use.
   * @param <D> The type of RiskDataStore to use.
   */
  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  class RiskServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of AdministrationClient to use used. */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::GetTryDereferenceType<M>;

      /** The type of OrderExecutionClient to use. */
      using OrderExecutionClient = Beam::GetTryDereferenceType<O>;

      /** The type of TransitionTimer to use. */
      using TransitionTimer = R;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /** The type of RiskDataStore to use. */
      using RiskDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a RiskServlet.
       * @param accounts Publishes the accounts whose risk is to be managed.
       * @param administrationClient Initializes the AdministrationClient.
       * @param marketDataClient Initializes the MarketDataClient.
       * @param orderExecutionClient Initializes the OrderExecutionClient.
       * @param transitionTimer Initializes the transition Timer.
       * @param timeClient Initializes the TimeClient.
       * @param dataStore Initializes the RiskDataStore.
       * @param exchangeRates The list of exchange rates.
       * @param markets The market database used by the portfolio.
       * @param destinations The destination database used to flatten positions.
       */
      template<typename AF, typename MF, typename OF, typename TF, typename DF>
      RiskServlet(
        Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
        AF&& administrationClient, MF&& marketDataClient,
        OF&& orderExecutionClient, std::function<
        std::unique_ptr<TransitionTimer> ()> transitionTimerFactory,
        TF&& timeClient, DF&& dataStore,
        std::vector<ExchangeRate> exchangeRates, MarketDatabase markets,
        DestinationDatabase destinations);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

      void Close();

    private:
      using ConsolidatedRiskController =
        RiskService::ConsolidatedRiskController<AdministrationClient*,
        MarketDataClient*, OrderExecutionClient*, TransitionTimer, TimeClient*,
        RiskDataStore*>;
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::GetOptionalLocalPtr<M> m_marketDataClient;
      Beam::GetOptionalLocalPtr<O> m_orderExecutionClient;
      std::function<std::unique_ptr<TransitionTimer> ()>
        m_transitionTimerFactory;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      std::vector<ExchangeRate> m_exchangeRates;
      MarketDatabase m_markets;
      DestinationDatabase m_destinations;
      std::shared_ptr<
        Beam::SnapshotPublisher<Beam::ServiceLocator::DirectoryEntry,
        std::vector<Beam::ServiceLocator::DirectoryEntry>>> m_accountPublisher;
      std::unordered_map<RiskPortfolioKey, Quantity> m_volumes;
      boost::optional<ConsolidatedRiskController> m_controller;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        Beam::ServiceLocator::DirectoryEntry, Beam::Threading::Mutex>
        m_accountToGroup;
      Beam::SynchronizedVector<ServiceProtocolClient*, Beam::Threading::Mutex>
        m_portfolioSubscribers;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      void BuildController();
      void ResetAccount(const Beam::ServiceLocator::DirectoryEntry& account);
      Beam::ServiceLocator::DirectoryEntry LoadGroup(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void OnRiskState(const RiskStateEntry& entry);
      void OnPortfolio(const RiskInventoryEntry& entry);
      void OnSubscribeRiskPortfolioUpdatesRequest(Beam::Services::RequestToken<
        ServiceProtocolClient, SubscribeRiskPortfolioUpdatesService>& request);
      void OnResetRegion(ServiceProtocolClient& client, const Region& region);
  };

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  struct MetaRiskServlet {
    using Session = RiskSession;
    template<typename C>
    struct apply {
      using type = RiskServlet<C, A, M, O, R, T, D>;
    };
  };

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  template<typename AF, typename MF, typename OF, typename TF, typename DF>
  RiskServlet<C, A, M, O, R, T, D>::RiskServlet(
    Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
    AF&& administrationClient, MF&& marketDataClient,
    OF&& orderExecutionClient, std::function<
    std::unique_ptr<TransitionTimer> ()> transitionTimerFactory,
    TF&& timeClient, DF&& dataStore, std::vector<ExchangeRate> exchangeRates,
    MarketDatabase markets, DestinationDatabase destinations)
    : m_administrationClient(std::forward<AF>(administrationClient)),
      m_marketDataClient(std::forward<MF>(marketDataClient)),
      m_orderExecutionClient(std::forward<OF>(orderExecutionClient)),
      m_transitionTimerFactory(std::move(transitionTimerFactory)),
      m_timeClient(std::forward<TF>(timeClient)),
      m_dataStore(std::forward<DF>(dataStore)),
      m_exchangeRates(std::move(exchangeRates)),
      m_markets(std::move(markets)),
      m_destinations(std::move(destinations)),
      m_accountPublisher(Beam::MakeSequencePublisherAdaptor(std::make_shared<
        Beam::QueueReaderPublisher<Beam::ServiceLocator::DirectoryEntry>>(
        std::move(accounts)))) {}

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterRiskServices(Store(slots));
    RegisterRiskMessages(Store(slots));
    SubscribeRiskPortfolioUpdatesService::AddRequestSlot(Store(slots),
      std::bind(&RiskServlet::OnSubscribeRiskPortfolioUpdatesRequest, this,
      std::placeholders::_1));
    ResetRegionService::AddSlot(Beam::Store(slots), std::bind(
      &RiskServlet::OnResetRegion, this, std::placeholders::_1,
      std::placeholders::_2));
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_portfolioSubscribers.Remove(&client);
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_administrationClient->Open();
      m_marketDataClient->Open();
      m_orderExecutionClient->Open();
      m_timeClient->Open();
      m_dataStore->Open();
      BuildController();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::Shutdown() {
    m_controller = boost::none;
    m_openState.SetClosed();
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::BuildController() {
    auto accounts = std::make_shared<Beam::Queue<
      Beam::ServiceLocator::DirectoryEntry>>();
    m_accountPublisher->Monitor(accounts);
    m_controller.emplace(std::move(accounts), &*m_administrationClient,
      &*m_marketDataClient, &*m_orderExecutionClient, m_transitionTimerFactory,
      &*m_timeClient, &*m_dataStore, m_exchangeRates, m_markets,
      m_destinations);
    m_controller->GetRiskStatePublisher().Monitor(
      m_tasks.GetSlot<RiskStateEntry>(std::bind(&RiskServlet::OnRiskState,
      this, std::placeholders::_1)));
    m_controller->GetPortfolioPublisher().Monitor(
      m_tasks.GetSlot<RiskInventoryEntry>(std::bind(&RiskServlet::OnPortfolio,
      this, std::placeholders::_1)));
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::ResetAccount(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto snapshot = m_dataStore->LoadInventorySnapshot(account);
    auto excludedOrders = OrderExecutionService::LoadOrderSubmissions(account,
      snapshot.m_excludedOrders, *m_orderExecutionClient);
    auto trailingOrderQuery = OrderExecutionService::AccountQuery();
    trailingOrderQuery.SetIndex(account);
    trailingOrderQuery.SetRange(Beam::Queries::Increment(snapshot.m_sequence),
      Beam::Queries::Sequence::Present());
    trailingOrderQuery.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Unlimited());
    auto trailingOrdersQueue = std::make_shared<
      Beam::Queue<Nexus::OrderExecutionService::SequencedOrder>>();
    m_orderExecutionClient->QueryOrderSubmissions(trailingOrderQuery,
      trailingOrdersQueue);
    auto lastSequence = snapshot.m_sequence;
    Beam::ForEach(trailingOrdersQueue, [&] (const auto& order) {
      excludedOrders.push_back(order);
      lastSequence = std::max(lastSequence, order.GetSequence());
    });
    auto portfolio = RiskPortfolio(m_markets,
      RiskPortfolio::Bookkeeper(snapshot.m_inventories));
    auto reports = std::vector<OrderExecutionService::ExecutionReportEntry>();
    excludedOrders.erase(std::remove_if(excludedOrders.begin(),
      excludedOrders.end(), [&] (const auto& order) {
        if(auto snapshot = order->GetPublisher().GetSnapshot()) {
          if(IsTerminal(snapshot->back().m_status)) {
            for(auto& report : *snapshot) {
              portfolio.Update(order->GetInfo().m_fields, report);
            }
            return true;
          } else {
            std::transform(snapshot->begin(), snapshot->end(),
              std::back_inserter(reports),
              [&] (auto& report) {
                return OrderExecutionService::ExecutionReportEntry(order,
                  std::move(report));
              });
          }
        }
        return false;
      }), excludedOrders.end());
    std::sort(reports.begin(), reports.end(),
      [] (auto& left, auto& right) {
        return std::tie(left.m_executionReport.m_timestamp,
          left.m_executionReport.m_id, left.m_executionReport.m_sequence) <
          std::tie(right.m_executionReport.m_timestamp,
          right.m_executionReport.m_id, right.m_executionReport.m_sequence);
      });
    for(auto& report : reports) {
      auto inverseReport = report.m_executionReport;
      inverseReport.m_lastQuantity = -inverseReport.m_lastQuantity;
      inverseReport.m_executionFee = -inverseReport.m_executionFee;
      inverseReport.m_processingFee = -inverseReport.m_processingFee;
      inverseReport.m_commission = -inverseReport.m_commission;
      portfolio.Update(report.m_order->GetInfo().m_fields, inverseReport);
    }
    auto updatedSnapshot = InventorySnapshot();
    for(auto& inventory : portfolio.GetBookkeeper().GetInventoryRange()) {
      updatedSnapshot.m_inventories.push_back(inventory.second);
    }
    updatedSnapshot.m_sequence = lastSequence;
    std::transform(excludedOrders.begin(), excludedOrders.end(),
      std::back_inserter(updatedSnapshot.m_excludedOrders),
      [] (const auto& order) {
        return order->GetInfo().m_orderId;
      });
    m_dataStore->Store(account, updatedSnapshot);
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  Beam::ServiceLocator::DirectoryEntry RiskServlet<C, A, M, O, R, T, D>::
      LoadGroup(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_accountToGroup.GetOrInsert(account,
      [&] {
        return m_administrationClient->LoadParentTradingGroup(account);
      });
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::OnRiskState(
      const RiskStateEntry& entry) {
    m_administrationClient->StoreRiskState(entry.m_key, entry.m_value);
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::OnPortfolio(
      const RiskInventoryEntry& entry) {
    auto& volume = m_volumes[entry.m_key];
    if(volume == entry.m_value.m_volume) {
      return;
    }
    volume = entry.m_value.m_volume;
    auto inventories = std::vector<InventoryUpdate>();
    auto update = InventoryUpdate();
    update.account = entry.m_key.m_account;
    update.inventory = entry.m_value;
    inventories.push_back(update);
    auto group = LoadGroup(entry.m_key.m_account);
    m_portfolioSubscribers.With([&] (auto& subscribers) {
      for(auto& subscriber : subscribers) {
        auto& session = subscriber->GetSession();
        if(session.HasGroupPortfolioSubscription(group)) {
          Beam::Services::SendRecordMessage<InventoryMessage>(*subscriber,
            inventories);
        }
      }
    });
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::OnSubscribeRiskPortfolioUpdatesRequest(
      Beam::Services::RequestToken<ServiceProtocolClient,
      SubscribeRiskPortfolioUpdatesService>& request) {
    auto& session = request.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    auto groups = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    if(!isAdministrator) {
      groups = m_administrationClient->LoadManagedTradingGroups(
        session.GetAccount());
    }
    auto entries = std::vector<RiskInventoryEntry>();
    m_portfolioSubscribers.With([&] (auto& subscribers) {
      if(isAdministrator) {
        session.AddAllPortfolioGroups();
      } else {
        for(auto& group : groups) {
          session.AddPortfolioGroup(group);
        }
      }
      subscribers.push_back(&request.GetClient());
      auto queue = std::make_shared<Beam::Queue<RiskInventoryEntry>>();
      m_controller->GetPortfolioPublisher().Monitor(queue);
      while(auto entry = queue->TryPop()) {
        auto group = LoadGroup(entry->m_key.m_account);
        if(session.HasGroupPortfolioSubscription(group)) {
          entries.push_back(std::move(*entry));
        }
      }
      request.SetResult(entries);
    });
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::OnResetRegion(
      ServiceProtocolClient& client, const Region& region) {
    auto& session = client.GetSession();
    if(!m_administrationClient->CheckAdministrator(session.GetAccount())) {
      return;
    }
    m_controller = boost::none;
    if(auto accounts = m_accountPublisher->GetSnapshot()) {
      for(auto& account : *accounts) {
        try {
          ResetAccount(account);
        } catch(const std::exception&) {
          std::cerr << "Region reset failed for account:\n\t" <<
            "Account: " << account << "\n\t" <<
            BEAM_REPORT_CURRENT_EXCEPTION() << std::endl;
        }
      }
    }
    BuildController();
  }
}

#endif
