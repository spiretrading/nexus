#ifndef NEXUS_RISK_CONTROLLER_HPP
#define NEXUS_RISK_CONTROLLER_HPP
#include <cstdint>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/StatePublisher.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Timer.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Accounting/PortfolioController.hpp"
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/RiskService/RiskDataStore.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskStateProcessor.hpp"
#include "Nexus/RiskService/RiskTransitionProcessor.hpp"

namespace Nexus::RiskService {

  /**
   * Implements a controller for a single account's RiskStateProcessor and
   * RiskTransitionProcessor updating both processors based on Orders
   * submitted and market data. Portfolio's are valued using the BboQuote
   * published by a MarketDataClient and Orders published by an
   * OrderExecutionPublisher.
   * @param <A> The type of AdministrationClient used to load an account's
   *        RiskParameters.
   * @param <M> The type of MarketDataClient to use.
   * @param <O> The type of OrderExecutionClient to use.
   * @param <R> The type of Timer to use to transition from CLOSED_ORDERS to
   *        DISABLED.
   * @param <T> The type of TimeClient to use.
   * @param <D> The type of RiskDataStore to use.
   */
  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  class RiskController {
    public:

      /**
       * The type of AdministrationClient used to load an account's
       * RiskParameters.
       */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::GetTryDereferenceType<M>;

      /** The type of OrderExecutionClient to use. */
      using OrderExecutionClient = Beam::GetTryDereferenceType<O>;

      /** The type of TransitionTimer to use. */
      using TransitionTimer = Beam::GetTryDereferenceType<R>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /** The type of RiskDataStore to use. */
      using RiskDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a RiskController.
       * @param account The account whose risk is being controlled.
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
      template<typename AF, typename MF, typename OF, typename RF, typename TF,
        typename DF>
      RiskController(Beam::ServiceLocator::DirectoryEntry account,
        AF&& administrationClient, MF&& marketDataClient,
        OF&& orderExecutionClient, RF&& transitionTimer, TF&& timeClient,
        DF&& dataStore, std::vector<ExchangeRate> exchangeRates,
        MarketDatabase markets, DestinationDatabase destinations);

      /** Resets all inventories for a Region. */
      void Reset(const Region& region);

      /** Resets all inventories for a Market. */
      void Reset(MarketCode market);

      /** Returns a Publisher for the account's RiskState. */
      const Beam::Publisher<RiskState>& GetRiskStatePublisher() const;

      /** Returns a Publisher for the account's Portfolio. */
      const Beam::SnapshotPublisher<RiskPortfolio::UpdateEntry, RiskPortfolio*>&
        GetPortfolioPublisher() const;

    private:
      mutable Beam::Threading::Mutex m_mutex;
      Beam::ServiceLocator::DirectoryEntry m_account;
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::GetOptionalLocalPtr<M> m_marketDataClient;
      Beam::GetOptionalLocalPtr<O> m_orderExecutionClient;
      Beam::GetOptionalLocalPtr<R> m_transitionTimer;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      std::vector<ExchangeRate> m_exchangeRates;
      MarketDatabase m_markets;
      DestinationDatabase m_destinations;
      boost::optional<RiskStateProcessor<TimeClient*>> m_stateProcessor;
      boost::optional<RiskTransitionProcessor<OrderExecutionClient*>>
        m_transitionProcessor;
      Beam::StatePublisher<RiskState> m_statePublisher;
      Beam::ValueSnapshotPublisher<RiskPortfolio::UpdateEntry, RiskPortfolio*>
        m_portfolioPublisher;
      boost::optional<Accounting::PortfolioController<RiskPortfolio*,
        MarketDataClient*>> m_portfolioController;
      RiskPortfolio m_snapshotPortfolio;
      Beam::Queries::Sequence m_snapshotSequence;
      std::unordered_set<const OrderExecutionService::Order*> m_excludedOrders;
      boost::optional<Beam::RoutineTaskQueue> m_tasks;

      RiskController(const RiskController&) = delete;
      RiskController& operator =(const RiskController&) = delete;
      void UpdateSnapshot(const OrderExecutionService::Order& order);
      void UpdatePortfolio(RiskPortfolio& portfolio);
      std::tuple<RiskPortfolio, Beam::Queries::Sequence> BuildPortfolio();
      void BuildProcessors();
      template<typename F>
      void Update(F&& f);
      void OnTransitionTimer(Beam::Threading::Timer::Result result);
      void OnRiskParametersUpdate(const RiskParameters& parameters);
      void OnPortfolioUpdate(const RiskPortfolio::UpdateEntry& update);
      void OnOrderSubmission(
        const OrderExecutionService::SequencedOrder& order);
      void OnExecutionReport(const OrderExecutionService::Order& order,
        const OrderExecutionService::ExecutionReport& report);
  };

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  RiskController(const Beam::ServiceLocator::DirectoryEntry&, A&&, M&&, O&&,
    R&&, T&&, D&&, const std::vector<ExchangeRate>&, MarketDatabase,
    DestinationDatabase) -> RiskController<std::remove_reference_t<A>,
    std::remove_reference_t<M>, std::remove_reference_t<O>,
    std::remove_reference_t<R>, std::remove_reference_t<T>,
    std::remove_reference_t<D>>;

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  template<typename AF, typename MF, typename OF, typename RF, typename TF,
    typename DF>
  RiskController<A, M, O, R, T, D>::RiskController(
      Beam::ServiceLocator::DirectoryEntry account,
      AF&& administrationClient, MF&& marketDataClient,
      OF&& orderExecutionClient, RF&& transitionTimer, TF&& timeClient,
      DF&& dataStore, std::vector<ExchangeRate> exchangeRates,
      MarketDatabase markets, DestinationDatabase destinations)
      : m_account(std::move(account)),
        m_administrationClient(std::forward<AF>(administrationClient)),
        m_marketDataClient(std::forward<MF>(marketDataClient)),
        m_orderExecutionClient(std::forward<OF>(orderExecutionClient)),
        m_transitionTimer(std::forward<RF>(transitionTimer)),
        m_timeClient(std::forward<TF>(timeClient)),
        m_dataStore(std::forward<DF>(dataStore)),
        m_exchangeRates(std::move(exchangeRates)),
        m_markets(std::move(markets)),
        m_destinations(std::move(destinations)),
        m_snapshotPortfolio(m_markets),
        m_publisher(
          [] (auto snapshot, auto& queue) {
            ForEach(*snapshot,
              [&] (const auto& update) {
                queue.Push(update);
              });
          }, Beam::SignalHandling::NullSlot(), &*m_portfolio)
        m_tasks({}) {
    BuildProcessors();
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::Reset(const Region& region) {
    auto lock = std::lock_guard(m_mutex);
    m_tasks = boost::none;
    m_tasks.emplace();
    m_tasks->Push(
      [=] {
        auto inventories = std::unordered_map<
          RiskInventory::Position::Key, RiskInventory>();
        for(auto& inventory :
            m_snapshotPortfolio.GetBookkeeper().GetInventoryRange()) {
          auto resetInventory = inventory.second;
          if(resetInventory.m_position.m_key.m_index <= region) {
            resetInventory.m_grossProfitAndLoss = Money::ZERO;
            resetInventory.m_fees = Money::ZERO;
            resetInventory.m_volume = Abs(resetInventory.m_position.m_quantity);
            resetInventory.m_transactionCount = [&] {
              if(resetInventory.m_volume == 0) {
                return 0;
              } else {
                return 1;
              }
            }();
          }
          inventories.insert(std::pair(inventory.first, resetInventory));
        }
        for(auto& order : m_excludedOrders) {
        }
        BuildProcessors();
      });
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::Reset(MarketCode market) {
    Reset(m_markets.FromCode(market));
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  const Beam::Publisher<RiskState>&
      RiskController<A, M, O, R, T, D>::GetRiskStatePublisher() const {
    return m_statePublisher;
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  const Beam::SnapshotPublisher<RiskPortfolio::UpdateEntry, RiskPortfolio*>&
      RiskController<A, M, O, R, T, D>::GetPortfolioPublisher() const {
    return m_portfolioPublisher;
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::UpdateSnapshot(
      const OrderExecutionService::Order& order) {
    auto executionReports =
      std::vector<OrderExecutionService::ExecutionReport>();
    order.GetPublisher().WithSnapshot(
      [&] (auto snapshot) {
        if(snapshot) {
          executionReports = *snapshot;
        }
      });
    for(auto& executionReport : executionReports) {
      m_snapshotPortfolio.Update(order.GetInfo().m_fields, executionReport);
    }
    m_excludedOrders.erase(&order);
    auto snapshot = InventorySnapshot();
    for(auto& inventory :
        m_snapshotPortfolio.GetBookkeeper().GetInventoryRange()) {
      snapshot.m_inventories.push_back(inventory.second);
    }
    snapshot.m_sequence = m_snapshotSequence;
    std::transform(m_excludedOrders.begin(), m_excludedOrders.end(),
      std::back_inserter(snapshot.m_excludedOrders),
      [] (auto& order) {
        return order->GetInfo().m_orderId;
      });
    std::sort(snapshot.m_excludedOrders.begin(),
      snapshot.m_excludedOrders.end());
    try {
      m_dataStore->Store(m_account, snapshot);
    } catch(const std::exception&) {
      std::cerr << "Unable to store inventory snapshot:\n\t" <<
        BEAM_REPORT_CURRENT_EXCEPTION() << std::endl;
    }
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::UpdatePortfolio(
      RiskPortfolio& portfolio) {
    auto reports = std::vector<OrderExecutionService::ExecutionReportEntry>();
    for(auto& order : m_excludedOrders) {
      auto snapshot = boost::optional<
        std::vector<OrderExecutionService::ExecutionReport>>();
      order->GetPublisher().Monitor(
        m_tasks->GetSlot<OrderExecutionService::ExecutionReport>(
        std::bind(&RiskController::OnExecutionReport, this,
        std::ref(*order), std::placeholders::_1)), Beam::Store(snapshot));
      if(snapshot) {
        std::transform(snapshot->begin(), snapshot->end(),
          std::back_inserter(reports),
          [&] (auto& report) {
            return OrderExecutionService::ExecutionReportEntry(order,
              std::move(report));
          });
      }
    }
    std::sort(reports.begin(), reports.end(),
      [] (auto& left, auto& right) {
        return std::tie(left.m_executionReport.m_timestamp,
          left.m_executionReport.m_id, left.m_executionReport.m_sequence) <
          std::tie(right.m_executionReport.m_timestamp,
          right.m_executionReport.m_id, right.m_executionReport.m_sequence);
      });
    for(auto& report : reports) {
      if(IsTerminal(report.m_executionReport.m_status)) {
        UpdateSnapshot(*report.m_order);
      }
      portfolio.Update(report.m_order->GetInfo().m_fields,
        report.m_executionReport);
    }
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  std::tuple<RiskPortfolio, Beam::Queries::Sequence>
      RiskController<A, M, O, R, T, D>::BuildPortfolio() {
    auto snapshot = m_dataStore->LoadInventorySnapshot(m_account);
    m_snapshotSequence = snapshot.m_sequence;
    auto excludedOrders = OrderExecutionService::LoadOrderSubmissions(m_account,
      snapshot.m_excludedOrders, *m_orderExecutionClient);
    m_excludedOrders = {excludedOrders.begin(), excludedOrders.end()};
    auto trailingOrderQuery = OrderExecutionService::AccountQuery();
    trailingOrderQuery.SetIndex(m_account);
    trailingOrderQuery.SetRange(Beam::Queries::Increment(snapshot.m_sequence),
      Beam::Queries::Sequence::Present());
    trailingOrderQuery.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Unlimited());
    auto trailingOrdersQueue = std::make_shared<
      Beam::Queue<Nexus::OrderExecutionService::SequencedOrder>>();
    m_orderExecutionClient->QueryOrderSubmissions(trailingOrderQuery,
      trailingOrdersQueue);
    auto lastSequence = Beam::Queries::Sequence::First();
    Beam::ForEach(trailingOrdersQueue, [&] (const auto& order) {
      m_excludedOrders.insert(*order);
      lastSequence = std::max(lastSequence, order.GetSequence());
    });
    auto portfolio = RiskPortfolio(m_markets,
      RiskPortfolio::Bookkeeper(snapshot.m_inventories));
    m_snapshotPortfolio = portfolio;
    UpdatePortfolio(portfolio);
    return {std::move(portfolio), Beam::Queries::Increment(lastSequence)};
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::BuildProcessors() {
    auto [portfolio, sequence] = BuildPortfolio();
    auto inventories = std::vector<RiskInventory>();
    for(auto& inventory : portfolio.GetBookkeeper().GetInventoryRange()) {
      inventories.push_back(inventory.second);
    }
    m_stateProcessor.emplace(std::move(portfolio),
      AdministrationService::LoadRiskParameters(*m_administrationClient,
      m_account), m_exchangeRates, &*m_timeClient);
    m_statePublisher.Push(m_stateProcessor->GetRiskState());
    m_transitionProcessor.emplace(m_account, std::move(inventories),
      m_stateProcessor->GetRiskState(), &*m_orderExecutionClient,
      m_destinations);
    auto realTimeQuery = OrderExecutionService::AccountQuery();
    realTimeQuery.SetIndex(m_account);
    realTimeQuery.SetRange(sequence, Beam::Queries::Sequence::Last());
    realTimeQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    m_orderExecutionClient->QueryOrderSubmissions(realTimeQuery,
      m_tasks->GetSlot<OrderExecutionService::SequencedOrder>(std::bind(
      &RiskController::OnOrderSubmission, this, std::placeholders::_1)));
    auto realTimeQueue = std::make_shared<
      Beam::Queue<const OrderExecutionService::Order*>>();
    m_orderExecutionClient->QueryOrderSubmissions(realTimeQuery, realTimeQueue);
    m_portfolioController.emplace(&m_stateProcessor->GetPortfolio(),
      &*m_marketDataClient, realTimeQueue);
    m_portfolioController->GetPublisher().Monitor(
      m_tasks->GetSlot<RiskPortfolio::UpdateEntry>(std::bind(
      &RiskController::OnPortfolioUpdate, this, std::placeholders::_1)));
    m_administrationClient->GetRiskParametersPublisher(m_account).Monitor(
      m_tasks->GetSlot<RiskParameters>(std::bind(
      &RiskController::OnRiskParametersUpdate, this, std::placeholders::_1)));
    m_transitionTimer->GetPublisher().Monitor(
      m_tasks->GetSlot<Beam::Threading::Timer::Result>(std::bind(
      &RiskController::OnTransitionTimer, this, std::placeholders::_1)));
    m_transitionTimer->Start();
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  template<typename F>
  void RiskController<A, M, O, R, T, D>::Update(F&& f) {
    auto previousState = m_stateProcessor->GetRiskState();
    f();
    auto& currentState = m_stateProcessor->GetRiskState();
    if(previousState != currentState) {
      m_transitionProcessor->Update(currentState);
      m_statePublisher.Push(currentState);
    }
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnTransitionTimer(
      Beam::Threading::Timer::Result result) {
    Update([&] {
      m_stateProcessor->UpdateTime();
    });
    m_transitionTimer->Start();
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnRiskParametersUpdate(
      const RiskParameters& parameters) {
    Update([&] {
      m_stateProcessor->Update(parameters);
    });
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnPortfolioUpdate(
      const RiskPortfolio::UpdateEntry& update) {
    Update([&] {
      m_portfolioController->GetPublisher().With([&] {
        m_stateProcessor->UpdatePortfolio();
      });
    });
    m_portfolioPublisher.Push(update);
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnOrderSubmission(
      const OrderExecutionService::SequencedOrder& order) {
    m_transitionProcessor->Add(**order);
    m_snapshotSequence = std::max(m_snapshotSequence, order.GetSequence());
    m_excludedOrders.insert(*order);
    (*order)->GetPublisher().Monitor(
      m_tasks->GetSlot<OrderExecutionService::ExecutionReport>(
      std::bind(&RiskController::OnExecutionReport, this, std::ref(**order),
      std::placeholders::_1)));
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnExecutionReport(
      const OrderExecutionService::Order& order,
      const OrderExecutionService::ExecutionReport& report) {
    if(IsTerminal(report.m_status)) {
      UpdateSnapshot(order);
    }
    m_transitionProcessor->Update(report);
  }
}

#endif
