#ifndef NEXUS_CONSOLIDATED_RISK_CONTROLLER_HPP
#define NEXUS_CONSOLIDATED_RISK_CONTROLLER_HPP
#include <functional>
#include <iostream>
#include <memory>
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/ScopedQueueReader.hpp>
#include <Beam/Queues/TablePublisher.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/RiskService/RiskController.hpp"

namespace Nexus::RiskService {

  /** Represents an entry in a RiskState table. */
  using RiskStateEntry =
    Beam::KeyValuePair<Beam::ServiceLocator::DirectoryEntry, RiskState>;

  /** Represents an entry in a RiskPortfolio table. */
  using RiskPortfolioEntry =
    Beam::KeyValuePair<RiskPortfolioKey, Inventory>;

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
  template<IsAdministrationClient A, MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
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

      /** Type of function used to make a unique TransitionTimer. */
      using TransitionTimerFactory =
        std::function<std::unique_ptr<TransitionTimer> ()>;

      /**
       * Constructs a ConsolidatedRiskController.
       * @param accounts Publishes the accounts whose RiskControllers are to be
       *        consolidated.
       * @param administration_client Initializes the AdministrationClient.
       * @param market_data_client Initializes the MarketDataClient.
       * @param order_execution_client Initializes the OrderExecutionClient.
       * @param transition_timer_factory The function used to make unique
       *        TransitionTimer's per account.
       * @param time_client Initializes the TimeClient.
       * @param data_store Initializes the RiskDataStore.
       * @param exchange_rates The exchange rates used by portfolios.
       * @param venues The venues used by portfolios.
       * @param destinations The destination database used to flatten positions.
       */
      template<Beam::Initializes<A> AF, Beam::Initializes<M> MF,
        Beam::Initializes<O> OF, Beam::Initializes<T> TF,
        Beam::Initializes<D> DF>
      ConsolidatedRiskController(
        Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
        AF&& administration_client, MF&& market_data_client,
        OF&& order_execution_client,
        TransitionTimerFactory transition_timer_factory, TF&& time_client,
        DF&& data_store, ExchangeRateTable exchange_rates, VenueDatabase venues,
        DestinationDatabase destinations);

      /** Returns a Publisher for all accounts RiskStates. */
      const Beam::Publisher<RiskStateEntry>& get_risk_state_publisher() const;

      /** Returns a Publisher for all accounts Portfolio. */
      const Beam::Publisher<RiskPortfolioEntry>&
        get_portfolio_publisher() const;

    private:
      using RiskController = RiskService::RiskController<
        AdministrationClient*, MarketDataClient*, OrderExecutionClient*,
        std::unique_ptr<TransitionTimer>, TimeClient*, RiskDataStore*>;
      Beam::GetOptionalLocalPtr<A> m_administration_client;
      Beam::GetOptionalLocalPtr<M> m_market_data_client;
      Beam::GetOptionalLocalPtr<O> m_order_execution_client;
      TransitionTimerFactory m_transition_timer_factory;
      Beam::GetOptionalLocalPtr<T> m_time_client;
      Beam::GetOptionalLocalPtr<D> m_data_store;
      ExchangeRateTable m_exchange_rates;
      VenueDatabase m_venues;
      DestinationDatabase m_destinations;
      Beam::TablePublisher<Beam::ServiceLocator::DirectoryEntry, RiskState>
        m_state_publisher;
      Beam::TablePublisher<RiskPortfolioKey, Inventory> m_portfolio_publisher;
      std::vector<std::unique_ptr<RiskController>> m_controllers;
      Beam::RoutineTaskQueue m_tasks;
      Beam::QueuePipe<Beam::ServiceLocator::DirectoryEntry> m_accounts_pipe;

      ConsolidatedRiskController(const ConsolidatedRiskController&) = delete;
      ConsolidatedRiskController& operator =(
        const ConsolidatedRiskController&) = delete;
      void on_account(const Beam::ServiceLocator::DirectoryEntry& account);
      void on_risk_state(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskState& state);
      void on_portfolio_entry(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const PortfolioUpdateEntry& entry);
  };

  template<IsAdministrationClient A, MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  ConsolidatedRiskController(
    Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry>, A&&, M&&,
    O&&, R&&, T&&, D&&, ExchangeRateTable, VenueDatabase,
    DestinationDatabase) -> ConsolidatedRiskController<
      std::remove_reference_t<A>, std::remove_reference_t<M>,
      std::remove_reference_t<O>,
      typename std::invoke_result_t<R>::element_type,
      std::remove_reference_t<T>, std::remove_reference_t<D>>;

  template<IsAdministrationClient A, MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  template<Beam::Initializes<A> AF, Beam::Initializes<M> MF,
    Beam::Initializes<O> OF, Beam::Initializes<T> TF, Beam::Initializes<D> DF>
  ConsolidatedRiskController<A, M, O, R, T, D>::ConsolidatedRiskController(
    Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
    AF&& administration_client, MF&& market_data_client,
    OF&& order_execution_client,
    TransitionTimerFactory transition_timer_factory, TF&& time_client,
    DF&& data_store, ExchangeRateTable exchange_rates, VenueDatabase venues,
    DestinationDatabase destinations)
  BEAM_SUPPRESS_THIS_INITIALIZER()
    : m_administration_client(std::forward<AF>(administration_client)),
      m_market_data_client(std::forward<MF>(market_data_client)),
      m_order_execution_client(std::forward<OF>(order_execution_client)),
      m_transition_timer_factory(std::move(transition_timer_factory)),
      m_time_client(std::forward<TF>(time_client)),
      m_data_store(std::forward<DF>(data_store)),
      m_exchange_rates(std::move(exchange_rates)),
      m_venues(std::move(venues)),
      m_destinations(std::move(destinations)),
      m_accounts_pipe(std::move(accounts),
        m_tasks.GetSlot<Beam::ServiceLocator::DirectoryEntry>(
          std::bind_front(&ConsolidatedRiskController::on_account, this))) {}
  BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<IsAdministrationClient A, MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  const Beam::Publisher<RiskStateEntry>& ConsolidatedRiskController<
      A, M, O, R, T, D>::get_risk_state_publisher() const {
    return m_state_publisher;
  }

  template<IsAdministrationClient A, MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  const Beam::Publisher<RiskPortfolioEntry>& ConsolidatedRiskController<
      A, M, O, R, T, D>::get_portfolio_publisher() const {
    return m_portfolio_publisher;
  }

  template<IsAdministrationClient A, MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void ConsolidatedRiskController<A, M, O, R, T, D>::on_account(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto controller = [&] {
      try {
        return std::make_unique<RiskController>(
          account, &*m_administration_client, &*m_market_data_client,
          &*m_order_execution_client, m_transition_timer_factory(),
          &*m_time_client, &*m_data_store, m_exchange_rates, m_venues,
          m_destinations);
      } catch(const std::exception&) {
        std::cerr << "Unable to load risk controller:\n\t" <<
          "Account: " << account << "\n\t" <<
          BEAM_REPORT_CURRENT_EXCEPTION() << std::endl;
        return std::unique_ptr<RiskController>();
      }
    }();
    if(!controller) {
      m_state_publisher.Push(account, RiskState::Type::DISABLED);
      return;
    }
    controller->get_risk_state_publisher().Monitor(
      m_tasks.GetSlot<RiskState>(std::bind_front(
        &ConsolidatedRiskController::on_risk_state, this, account)));
    controller->get_portfolio_publisher().Monitor(
      m_tasks.GetSlot<PortfolioUpdateEntry>(std::bind_front(
        &ConsolidatedRiskController::on_portfolio_entry, this, account)));
    m_controllers.push_back(std::move(controller));
  }

  template<IsAdministrationClient A, MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void ConsolidatedRiskController<A, M, O, R, T, D>::on_risk_state(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskState& state) {
    m_state_publisher.Push(account, state);
  }

  template<IsAdministrationClient A, MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void ConsolidatedRiskController<A, M, O, R, T, D>::on_portfolio_entry(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const PortfolioUpdateEntry& entry) {
    auto key = RiskPortfolioKey(
      account, entry.m_security_inventory.m_position.m_security);
    m_portfolio_publisher.Push(std::move(key), entry.m_security_inventory);
  }
}

#endif
