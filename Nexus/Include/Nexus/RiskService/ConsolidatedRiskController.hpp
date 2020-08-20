#ifndef NEXUS_CONSOLIDATED_RISK_CONTROLLER_HPP
#define NEXUS_CONSOLIDATED_RISK_CONTROLLER_HPP
#include <iostream>
#include <memory>
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/ScopedQueueReader.hpp>
#include <Beam/Queues/TablePublisher.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/RiskService/RiskController.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus::RiskService {

  /** Represents an entry in a RiskState table. */
  using RiskStateEntry = Beam::KeyValuePair<
    Beam::ServiceLocator::DirectoryEntry, RiskState>;

  /** Represents an entry in a RiskPortfolio table. */
  using RiskPortfolioEntry = Beam::KeyValuePair<RiskPortfolioKey,
    RiskInventory>;

  /**
   * Consolidates the RiskControllers for multiple accounts.
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
  class ConsolidatedRiskController {
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
      using TransitionTimer = R;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /** The type of RiskDataStore to use. */
      using RiskDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a ConsolidatedRiskController.
       * @param accounts Publishes the accounts whose RiskControllers are to be
       *        consolidated.
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
      ConsolidatedRiskController(
        Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
        AF&& administrationClient, MF&& marketDataClient,
        OF&& orderExecutionClient, std::function<
        std::unique_ptr<TransitionTimer> ()> transitionTimerFactory,
        TF&& timeClient, DF&& dataStore,
        std::vector<ExchangeRate> exchangeRates, MarketDatabase markets,
        DestinationDatabase destinations);

      /** Returns a Publisher for all accounts RiskStates. */
      const Beam::Publisher<RiskStateEntry>& GetRiskStatePublisher() const;

      /** Returns a Publisher for all accounts Portfolio. */
      const Beam::Publisher<RiskPortfolioEntry>& GetPortfolioPublisher() const;

      void Open();

      void Close();

    private:
      using RiskController = RiskService::RiskController<AdministrationClient*,
        MarketDataClient*, OrderExecutionClient*,
        std::unique_ptr<TransitionTimer>, TimeClient*, RiskDataStore*>;
      Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> m_accounts;
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
      Beam::TablePublisher<Beam::ServiceLocator::DirectoryEntry, RiskState>
        m_statePublisher;
      Beam::TablePublisher<RiskPortfolioKey, RiskInventory>
        m_portfolioPublisher;
      std::vector<std::unique_ptr<RiskController>> m_controllers;
      boost::optional<Beam::QueuePipe<Beam::ServiceLocator::DirectoryEntry>>
        m_accountsPipe;
      Beam::RoutineTaskQueue m_tasks;

      ConsolidatedRiskController(const ConsolidatedRiskController&) = delete;
      ConsolidatedRiskController& operator =(
        const ConsolidatedRiskController&) = delete;
      void OnAccount(const Beam::ServiceLocator::DirectoryEntry& account);
      void OnRiskState(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskState& state);
      void OnPortfolioEntry(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskPortfolio::UpdateEntry& entry);
  };

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  ConsolidatedRiskController(
    Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry>, A&&, M&&,
    O&&, R&&, T&&, D&&, std::vector<ExchangeRate>, MarketDatabase,
    DestinationDatabase) -> ConsolidatedRiskController<
    std::remove_reference_t<A>, std::remove_reference_t<M>,
    std::remove_reference_t<O>, typename std::invoke_result_t<R>::element_type,
    std::remove_reference_t<T>, std::remove_reference_t<D>>;

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  template<typename AF, typename MF, typename OF, typename TF, typename DF>
  ConsolidatedRiskController<A, M, O, R, T, D>::ConsolidatedRiskController(
    Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
    AF&& administrationClient, MF&& marketDataClient,
    OF&& orderExecutionClient, std::function<
    std::unique_ptr<TransitionTimer> ()> transitionTimerFactory,
    TF&& timeClient, DF&& dataStore, std::vector<ExchangeRate> exchangeRates,
    MarketDatabase markets, DestinationDatabase destinations)
    : m_accounts(std::move(accounts)),
      m_administrationClient(std::forward<AF>(administrationClient)),
      m_marketDataClient(std::forward<MF>(marketDataClient)),
      m_orderExecutionClient(std::forward<OF>(orderExecutionClient)),
      m_transitionTimerFactory(std::move(transitionTimerFactory)),
      m_timeClient(std::forward<TF>(timeClient)),
      m_dataStore(std::forward<DF>(dataStore)),
      m_exchangeRates(std::move(exchangeRates)),
      m_markets(std::move(markets)),
      m_destinations(std::move(destinations)) {}

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  const Beam::Publisher<RiskStateEntry>& ConsolidatedRiskController<
      A, M, O, R, T, D>::GetRiskStatePublisher() const {
    return m_statePublisher;
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  const Beam::Publisher<RiskPortfolioEntry>& ConsolidatedRiskController<
      A, M, O, R, T, D>::GetPortfolioPublisher() const {
    return m_portfolioPublisher;
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void ConsolidatedRiskController<A, M, O, R, T, D>::Open() {
    m_marketDataClient->Open();
    m_orderExecutionClient->Open();
    m_timeClient->Open();
    m_dataStore->Open();
    m_accountsPipe.emplace(std::move(m_accounts),
      m_tasks.GetSlot<Beam::ServiceLocator::DirectoryEntry>(std::bind(
      &ConsolidatedRiskController::OnAccount, this,
      std::placeholders::_1)));
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void ConsolidatedRiskController<A, M, O, R, T, D>::Close() {
    m_accountsPipe = boost::none;
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void ConsolidatedRiskController<A, M, O, R, T, D>::OnAccount(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto controller = [&] {
      try {
        return std::make_unique<RiskController>(account,
          &*m_administrationClient, &*m_marketDataClient,
          &*m_orderExecutionClient, m_transitionTimerFactory(), &*m_timeClient,
          &*m_dataStore, m_exchangeRates, m_markets, m_destinations);
      } catch(const std::exception&) {
        std::cerr << "Unable to load risk controller:\n\t" <<
          "Account: " << account << "\n\t" <<
          BEAM_REPORT_CURRENT_EXCEPTION() << std::endl;
        return std::unique_ptr<RiskController>();
      }
    }();
    if(!controller) {
      m_statePublisher.Push(account, RiskState::Type::DISABLED);
      return;
    }
    controller->GetRiskStatePublisher().Monitor(m_tasks.GetSlot<RiskState>(
      std::bind(&ConsolidatedRiskController::OnRiskState, this, account,
      std::placeholders::_1)));
    controller->GetPortfolioPublisher().Monitor(
      m_tasks.GetSlot<RiskPortfolio::UpdateEntry>(
      std::bind(&ConsolidatedRiskController::OnPortfolioEntry, this, account,
      std::placeholders::_1)));
    m_controllers.push_back(std::move(controller));
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void ConsolidatedRiskController<A, M, O, R, T, D>::OnRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskState& state) {
    m_statePublisher.Push(account, state);
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  void ConsolidatedRiskController<A, M, O, R, T, D>::OnPortfolioEntry(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskPortfolio::UpdateEntry& entry) {
    auto key = RiskPortfolioKey(account,
      entry.m_securityInventory.m_position.m_key.m_index);
    m_portfolioPublisher.Push(std::move(key), entry.m_securityInventory);
  }
}

#endif
