#ifndef NEXUS_RISK_CONTROLLER_HPP
#define NEXUS_RISK_CONTROLLER_HPP
#include <memory>
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/StatePublisher.hpp>
#include <Beam/Queues/ValueSnapshotPublisher.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
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
      RiskController(const Beam::ServiceLocator::DirectoryEntry& account,
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
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::GetOptionalLocalPtr<O> m_orderExecutionClient;
      Beam::GetOptionalLocalPtr<R> m_transitionTimer;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      DestinationDatabase m_destinations;
      RiskStateProcessor<T> m_stateProcessor;
      boost::optional<RiskTransitionProcessor<OrderExecutionClient*>>
        m_transitionProcessor;
      Beam::StatePublisher<RiskState> m_statePublisher;
      boost::optional<Accounting::PortfolioController<RiskPortfolio*, M>>
        m_portfolioController;
      Beam::RoutineTaskQueue m_tasks;

      RiskController(const RiskController&) = delete;
      RiskController& operator =(const RiskController&) = delete;
      template<typename F>
      void Update(F&& f);
      void OnTransitionTimer(Beam::Threading::Timer::Result result);
      void OnRiskParametersUpdate(const RiskParameters& parameters);
      void OnPortfolioUpdate(const RiskPortfolio::UpdateEntry& update);
      void OnOrderSubmission(const OrderExecutionService::Order* order);
      void OnExecutionReport(
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
      const Beam::ServiceLocator::DirectoryEntry& account,
      AF&& administrationClient, MF&& marketDataClient,
      OF&& orderExecutionClient, RF&& transitionTimer, TF&& timeClient,
      DF&& dataStore, const std::vector<ExchangeRate>& exchangeRates,
      MarketDatabase markets, DestinationDatabase destinations)
      : m_administrationClient(std::forward<AF>(administrationClient)),
        m_orderExecutionClient(std::forward<OF>(orderExecutionClient)),
        m_transitionTimer(std::forward<RF>(transitionTimer)),
        m_dataStore(std::forward<DF>(dataStore)),
        m_destinations(std::move(destinations)),
        m_stateProcessor(markets, exchangeRates, std::forward<TF>(timeClient)) {
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
    auto lastSequence = Beam::Queries::Sequence::First();
    Beam::ForEach(trailingOrdersQueue, [&] (const auto& order) {
      excludedOrders.push_back(order.GetValue());
      lastSequence = std::max(lastSequence, order.GetSequence());
    });
    auto realTimeQuery = OrderExecutionService::AccountQuery();
    realTimeQuery.SetIndex(account);
    realTimeQuery.SetRange(lastSequence, Beam::Queries::Sequence::Last());
    realTimeQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    auto realTimeQueue = std::make_shared<
      Beam::Queue<const OrderExecutionService::Order*>>();
    m_orderExecutionClient->QueryOrderSubmissions(realTimeQuery, realTimeQueue);
    m_orderExecutionClient->QueryOrderSubmissions(realTimeQuery,
      m_tasks.GetSlot<const OrderExecutionService::Order*>(std::bind(
      &RiskController::OnOrderSubmission, this, std::placeholders::_1)));
    m_portfolioController.emplace(&m_stateProcessor.GetPortfolio(),
      std::forward<MF>(marketDataClient), realTimeQueue);
    m_portfolioController->GetPublisher().Monitor(
      m_tasks.GetSlot<RiskPortfolio::UpdateEntry>(std::bind(
      &RiskController::OnPortfolioUpdate, this, std::placeholders::_1)));
    m_administrationClient->GetRiskParametersPublisher(account).Monitor(
      m_tasks.GetSlot<RiskParameters>(std::bind(
      &RiskController::OnRiskParametersUpdate, this, std::placeholders::_1)));
    m_transitionTimer->GetPublisher().Monitor(
      m_tasks.GetSlot<Beam::Threading::Timer::Result>(std::bind(
      &RiskController::OnTransitionTimer, this, std::placeholders::_1)));
    m_transitionTimer->Start();
    m_statePublisher.Push(m_stateProcessor.GetRiskState());
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
  template<typename F>
  void RiskController<A, M, O, R, T, D>::Update(F&& f) {
    auto previousState = m_stateProcessor.GetRiskState();
    f();
    auto& currentState = m_stateProcessor.GetRiskState();
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
      m_stateProcessor.UpdateTime();
    });
    m_transitionTimer->Start();
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnRiskParametersUpdate(
      const RiskParameters& parameters) {
    Update([&] {
      m_stateProcessor.Update(parameters);
    });
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnPortfolioUpdate(
      const RiskPortfolio::UpdateEntry& update) {
    Update([&] {
      m_portfolioController->GetPublisher().With([&] {
        m_stateProcessor.UpdatePortfolio();
      });
    });
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnOrderSubmission(
      const OrderExecutionService::Order* order) {
    m_transitionProcessor->Add(*order);
    order->GetPublisher().Monitor(
      m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(
      std::bind(&RiskController::OnExecutionReport, this,
      std::placeholders::_1)));
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void RiskController<A, M, O, R, T, D>::OnExecutionReport(
      const OrderExecutionService::ExecutionReport& report) {
    m_transitionProcessor->Update(report);
  }
}

#endif
