#ifndef NEXUS_RISK_CONTROLLER_HPP
#define NEXUS_RISK_CONTROLLER_HPP
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
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/RiskService/RiskDataStore.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskStateModel.hpp"
#include "Nexus/RiskService/RiskTransitionModel.hpp"

namespace Nexus::RiskService {

  /**
   * Implements a controller for a single account's RiskStateModel and
   * RiskTransitionModel updating both models based on Orders submitted and
   * market data. Portfolio's are valued using the BboQuote published by a
   * MarketDataClient and Orders published by an OrderExecutionPublisher.
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
        DF&& dataStore, const std::vector<ExchangeRate>& exchangeRates,
        MarketDatabase markets, DestinationDatabase destinations);

      /** Returns a Publisher for the account's RiskState. */
      const Beam::Publisher<RiskState>& GetRiskStatePublisher() const;

      /** Returns a Publisher for the account's Portfolio. */
      const Beam::SnapshotPublisher<RiskPortfolio::UpdateEntry, RiskPortfolio*>&
        GetPortfolioPublisher() const;

    private:
      mutable Beam::Threading::Mutex m_mutex;
      Beam::ServiceLocator::DirectoryEntry m_account;
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::GetOptionalLocalPtr<O> m_orderExecutionClient;
      Beam::GetOptionalLocalPtr<R> m_transitionTimer;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      boost::optional<RiskStateModel<T>> m_stateModel;
      boost::optional<Accounting::PortfolioController<RiskPortfolio*, M>>
        m_portfolioController;
      boost::optional<RiskTransitionModel<OrderExecutionClient*>>
        m_transitionModel;
      Beam::StatePublisher<RiskState> m_statePublisher;
      RiskPortfolio m_snapshotPortfolio;
      Beam::Queries::Sequence m_snapshotSequence;
      std::unordered_set<OrderExecutionService::OrderId> m_excludedOrders;
      Beam::RoutineTaskQueue m_tasks;

      RiskController(const RiskController&) = delete;
      RiskController& operator =(const RiskController&) = delete;
      void UpdateSnapshot(const OrderExecutionService::Order& order);
      std::tuple<RiskPortfolio, Beam::Queries::Sequence,
        std::vector<const OrderExecutionService::Order*>> MakePortfolio(
        MarketDatabase markets);
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
      DF&& dataStore, const std::vector<ExchangeRate>& exchangeRates,
      MarketDatabase markets, DestinationDatabase destinations)
      : m_account(std::move(account)),
        m_administrationClient(std::forward<AF>(administrationClient)),
        m_orderExecutionClient(std::forward<OF>(orderExecutionClient)),
        m_transitionTimer(std::forward<RF>(transitionTimer)),
        m_dataStore(std::forward<DF>(dataStore)),
        m_snapshotPortfolio(markets) {
    auto lock = std::lock_guard(m_mutex);
    auto [portfolio, sequence, excludedOrders] = MakePortfolio(
      std::move(markets));
    auto inventories = std::vector<RiskInventory>();
    for(auto& inventory : portfolio.GetBookkeeper().GetInventoryRange()) {
      inventories.push_back(inventory);
    }
    m_stateModel.emplace(std::move(portfolio),
      AdministrationService::LoadRiskParameters(*m_administrationClient,
        m_account), exchangeRates, std::forward<TF>(timeClient));
    auto realTimeQuery = OrderExecutionService::AccountQuery();
    realTimeQuery.SetIndex(m_account);
    realTimeQuery.SetRange(sequence, Beam::Queries::Sequence::Last());
    realTimeQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    realTimeQuery.SetInterruptionPolicy(
      Beam::Queries::InterruptionPolicy::RECOVER_DATA);
    auto realTimeQueue = std::make_shared<
      Beam::Queue<const OrderExecutionService::Order*>>();
    for(auto& order : excludedOrders) {
      realTimeQueue->Push(order);
    }
    m_orderExecutionClient->QueryOrderSubmissions(realTimeQuery, realTimeQueue);
    m_portfolioController.emplace(&m_stateModel->GetPortfolio(),
      std::forward<MF>(marketDataClient), realTimeQueue);
    m_transitionModel.emplace(m_account, std::move(inventories),
      m_stateModel->GetRiskState(), &*m_orderExecutionClient,
      std::move(destinations));
    m_orderExecutionClient->QueryOrderSubmissions(realTimeQuery,
      m_tasks.GetSlot<OrderExecutionService::SequencedOrder>(std::bind(
        &RiskController::OnOrderSubmission, this, std::placeholders::_1)));
    m_portfolioController->GetPublisher().Monitor(
      m_tasks.GetSlot<RiskPortfolio::UpdateEntry>(std::bind(
        &RiskController::OnPortfolioUpdate, this, std::placeholders::_1)));
    m_administrationClient->GetRiskParametersPublisher(m_account).Monitor(
      m_tasks.GetSlot<RiskParameters>(std::bind(
        &RiskController::OnRiskParametersUpdate, this, std::placeholders::_1)));
    m_transitionTimer->GetPublisher().Monitor(
      m_tasks.GetSlot<Beam::Threading::Timer::Result>(std::bind(
        &RiskController::OnTransitionTimer, this, std::placeholders::_1)));
    m_transitionTimer->Start();
    m_statePublisher.Push(m_stateModel->GetRiskState());
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
    return m_portfolioController->GetPublisher();
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::UpdateSnapshot(
      const OrderExecutionService::Order& order) {
    if(auto executionReports = order.GetPublisher().GetSnapshot()) {
      for(auto& executionReport : *executionReports) {
        m_snapshotPortfolio.Update(order.GetInfo().m_fields, executionReport);
      }
    }
    m_excludedOrders.erase(order.GetInfo().m_orderId);
    auto snapshot = InventorySnapshot();
    for(auto& inventory :
        m_snapshotPortfolio.GetBookkeeper().GetInventoryRange()) {
      snapshot.m_inventories.push_back(inventory);
    }
    snapshot.m_sequence = m_snapshotSequence;
    snapshot.m_excludedOrders.insert(snapshot.m_excludedOrders.end(),
      m_excludedOrders.begin(), m_excludedOrders.end());
    try {
      m_dataStore->Store(m_account, snapshot);
    } catch(const std::exception&) {
      std::cerr << "Snapshot update failed for account:\n\t" <<
        "Account: " << m_account << "\n\t" <<
        BEAM_REPORT_CURRENT_EXCEPTION() << std::endl;
    }
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  std::tuple<RiskPortfolio, Beam::Queries::Sequence,
      std::vector<const OrderExecutionService::Order*>>
      RiskController<A, M, O, R, T, D>::MakePortfolio(MarketDatabase markets) {
    auto [portfolio, sequence, excludedOrders] = RiskService::MakePortfolio(
      m_dataStore->LoadInventorySnapshot(m_account), m_account,
      std::move(markets), *m_orderExecutionClient);
    m_snapshotPortfolio = portfolio;
    m_snapshotSequence = sequence;
    std::transform(excludedOrders.begin(), excludedOrders.end(),
      std::inserter(m_excludedOrders, m_excludedOrders.end()),
      [] (const auto& order) { return order->GetInfo().m_orderId; });
    for(auto& order : excludedOrders) {
      order->GetPublisher().Monitor(
        m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(
          std::bind(&RiskController::OnExecutionReport, this, std::ref(*order),
            std::placeholders::_1)));
    }
    return {std::move(portfolio), Beam::Queries::Increment(sequence),
      std::move(excludedOrders)};
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  template<typename F>
  void RiskController<A, M, O, R, T, D>::Update(F&& f) {
    auto previousState = m_stateModel->GetRiskState();
    std::forward<F>(f)();
    auto& currentState = m_stateModel->GetRiskState();
    if(previousState != currentState) {
      m_transitionModel->Update(currentState);
      m_statePublisher.Push(currentState);
    }
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnTransitionTimer(
      Beam::Threading::Timer::Result result) {
    Update([&] {
      m_stateModel->UpdateTime();
    });
    m_transitionTimer->Start();
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnRiskParametersUpdate(
      const RiskParameters& parameters) {
    Update([&] {
      m_stateModel->Update(parameters);
    });
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnPortfolioUpdate(
      const RiskPortfolio::UpdateEntry& update) {
    Update([&] {
      m_portfolioController->GetPublisher().With([&] {
        m_stateModel->UpdatePortfolio();
      });
    });
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnOrderSubmission(
      const OrderExecutionService::SequencedOrder& order) {
    m_transitionModel->Add(**order);
    m_snapshotSequence = std::max(m_snapshotSequence, order.GetSequence());
    m_excludedOrders.insert((*order)->GetInfo().m_orderId);
    (*order)->GetPublisher().Monitor(
      m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(
        std::bind(&RiskController::OnExecutionReport, this, std::ref(**order),
          std::placeholders::_1)));
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnExecutionReport(
      const OrderExecutionService::Order& order,
      const OrderExecutionService::ExecutionReport& report) {
    auto lock = std::lock_guard(m_mutex);
    if(IsTerminal(report.m_status)) {
      UpdateSnapshot(order);
    }
    m_transitionModel->Update(report);
  }
}

#endif
