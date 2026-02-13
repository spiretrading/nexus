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
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Accounting/PortfolioController.hpp"
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/RiskService/RiskDataStore.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskStateModel.hpp"
#include "Nexus/RiskService/RiskTransitionModel.hpp"

namespace Nexus {

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
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  class RiskController {
    public:

      /**
       * The type of AdministrationClient used to load an account's
       * RiskParameters.
       */
      using AdministrationClient = Beam::dereference_t<A>;

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::dereference_t<M>;

      /** The type of OrderExecutionClient to use. */
      using OrderExecutionClient = Beam::dereference_t<O>;

      /** The type of TransitionTimer to use. */
      using TransitionTimer = Beam::dereference_t<R>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::dereference_t<T>;

      /** The type of RiskDataStore to use. */
      using RiskDataStore = Beam::dereference_t<D>;

      /**
       * Constructs a RiskController.
       * @param account The account whose risk is being controlled.
       * @param administration_client Initializes the AdministrationClient.
       * @param market_data_client Initializes the MarketDataClient.
       * @param order_execution_client Initializes the OrderExecutionClient.
       * @param transition_timer Initializes the transition Timer.
       * @param time_client Initializes the TimeClient.
       * @param data_store Initializes the RiskDataStore.
       * @param exchange_rates The exchange rates.
       * @param destinations The destination database used to flatten positions.
       */
      template<Beam::Initializes<A> AF, Beam::Initializes<M> MF,
        Beam::Initializes<O> OF, Beam::Initializes<R> RF,
        Beam::Initializes<T> TF, Beam::Initializes<D> DF>
      RiskController(Beam::DirectoryEntry account,
        AF&& administration_client, MF&& market_data_client,
        OF&& order_execution_client, RF&& transition_timer, TF&& time_client,
        DF&& data_store, const ExchangeRateTable& exchange_rates,
        DestinationDatabase destinations);

      /** Returns a Publisher for the account's RiskState. */
      const Beam::Publisher<RiskState>& get_risk_state_publisher() const;

      /** Returns a Publisher for the account's Portfolio. */
      const Beam::SnapshotPublisher<PortfolioUpdateEntry, RiskPortfolio*>&
        get_portfolio_publisher() const;

    private:
      mutable Beam::Mutex m_mutex;
      Beam::DirectoryEntry m_account;
      Beam::local_ptr_t<A> m_administration_client;
      Beam::local_ptr_t<O> m_order_execution_client;
      Beam::local_ptr_t<R> m_transition_timer;
      Beam::local_ptr_t<D> m_data_store;
      boost::optional<RiskStateModel<T>> m_state_model;
      boost::optional<PortfolioController<RiskPortfolio*, M>>
        m_portfolio_controller;
      boost::optional<RiskTransitionModel<OrderExecutionClient*>>
        m_transition_model;
      Beam::StatePublisher<RiskState> m_state_publisher;
      RiskPortfolio m_snapshot_portfolio;
      Beam::Sequence m_snapshot_sequence;
      std::unordered_set<OrderId> m_excluded_orders;
      Beam::RoutineTaskQueue m_tasks;

      RiskController(const RiskController&) = delete;
      RiskController& operator =(const RiskController&) = delete;
      void update_snapshot(const Order& order);
      std::tuple<RiskPortfolio, Beam::Sequence,
        std::vector<std::shared_ptr<Order>>> make_portfolio();
      template<typename F>
      void update(F&& f);
      void on_transition_timer(Beam::Timer::Result result);
      void on_risk_parameters_update(const RiskParameters& parameters);
      void on_portfolio_update(const PortfolioUpdateEntry& update);
      void on_order_submission(const SequencedOrder& order);
      void on_execution_report(
        const Order& order, const ExecutionReport& report);
  };

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  RiskController(const Beam::DirectoryEntry&, A&&, M&&, O&&,
    R&&, T&&, D&&, const ExchangeRateTable&, DestinationDatabase) ->
      RiskController<std::remove_reference_t<A>,
        std::remove_reference_t<M>, std::remove_reference_t<O>,
        std::remove_reference_t<R>, std::remove_reference_t<T>,
        std::remove_reference_t<D>>;

  template<typename A, typename M, typename O, typename R, typename T,
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  template<Beam::Initializes<A> AF, Beam::Initializes<M> MF,
    Beam::Initializes<O> OF, Beam::Initializes<R> RF,
    Beam::Initializes<T> TF, Beam::Initializes<D> DF>
  RiskController<A, M, O, R, T, D>::RiskController(Beam::DirectoryEntry account,
      AF&& administration_client, MF&& market_data_client,
      OF&& order_execution_client, RF&& transition_timer, TF&& time_client,
      DF&& data_store, const ExchangeRateTable& exchange_rates,
      DestinationDatabase destinations)
      : m_account(std::move(account)),
        m_administration_client(std::forward<AF>(administration_client)),
        m_order_execution_client(std::forward<OF>(order_execution_client)),
        m_transition_timer(std::forward<RF>(transition_timer)),
        m_data_store(std::forward<DF>(data_store)) {
    auto lock = std::lock_guard(m_mutex);
    auto [portfolio, sequence, excluded_orders] = make_portfolio();
    auto inventories = std::vector<Inventory>();
    for(auto& inventory : portfolio.get_bookkeeper().get_inventory_range()) {
      inventories.push_back(inventory);
    }
    m_state_model.emplace(std::move(portfolio),
      load_risk_parameters(*m_administration_client, m_account), exchange_rates,
      std::forward<TF>(time_client));
    auto real_time_query = AccountQuery();
    real_time_query.set_index(m_account);
    real_time_query.set_range(sequence, Beam::Sequence::LAST);
    real_time_query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
    real_time_query.set_interruption_policy(
      Beam::InterruptionPolicy::RECOVER_DATA);
    auto real_time_queue =
      std::make_shared<Beam::Queue<std::shared_ptr<Order>>>();
    for(auto& order : excluded_orders) {
      real_time_queue->push(order);
    }
    m_order_execution_client->query(real_time_query, real_time_queue);
    m_portfolio_controller.emplace(&m_state_model->get_portfolio(),
      std::forward<MF>(market_data_client), real_time_queue);
    m_transition_model.emplace(m_account, std::move(inventories),
      m_state_model->get_risk_state(), &*m_order_execution_client,
      std::move(destinations));
    m_order_execution_client->query(real_time_query,
      m_tasks.get_slot<SequencedOrder>(
        std::bind_front(&RiskController::on_order_submission, this)));
    m_portfolio_controller->get_publisher().monitor(
      m_tasks.get_slot<PortfolioUpdateEntry>(
        std::bind_front(&RiskController::on_portfolio_update, this)));
    m_administration_client->get_risk_parameters_publisher(m_account).monitor(
      m_tasks.get_slot<RiskParameters>(
        std::bind_front(&RiskController::on_risk_parameters_update, this)));
    m_transition_timer->get_publisher().monitor(
      m_tasks.get_slot<Beam::Timer::Result>(
        std::bind_front(&RiskController::on_transition_timer, this)));
    m_transition_timer->start();
    m_state_publisher.push(m_state_model->get_risk_state());
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  const Beam::Publisher<RiskState>&
      RiskController<A, M, O, R, T, D>::get_risk_state_publisher() const {
    return m_state_publisher;
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  const Beam::SnapshotPublisher<PortfolioUpdateEntry, RiskPortfolio*>&
      RiskController<A, M, O, R, T, D>::get_portfolio_publisher() const {
    return m_portfolio_controller->get_publisher();
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskController<A, M, O, R, T, D>::update_snapshot(const Order& order) {
    if(auto reports = order.get_publisher().get_snapshot()) {
      for(auto& report : *reports) {
        m_snapshot_portfolio.update(order.get_info().m_fields, report);
      }
    }
    m_excluded_orders.erase(order.get_info().m_id);
    auto snapshot = InventorySnapshot();
    for(auto& inventory :
        m_snapshot_portfolio.get_bookkeeper().get_inventory_range()) {
      snapshot.m_inventories.push_back(inventory);
    }
    snapshot.m_sequence = m_snapshot_sequence;
    snapshot.m_excluded_orders.insert(snapshot.m_excluded_orders.end(),
      m_excluded_orders.begin(), m_excluded_orders.end());
    try {
      m_data_store->store(m_account, snapshot);
    } catch(const std::exception&) {
      std::cerr << "Snapshot update failed for account:\n\t" << "Account: " <<
        m_account << "\n\t" << BEAM_REPORT_CURRENT_EXCEPTION() << std::endl;
    }
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  std::tuple<RiskPortfolio, Beam::Sequence,
      std::vector<std::shared_ptr<Order>>>
        RiskController<A, M, O, R, T, D>::make_portfolio() {
    auto [portfolio, sequence, excluded_orders] = Nexus::make_portfolio(
      m_data_store->load_inventory_snapshot(m_account), m_account,
      *m_order_execution_client);
    m_snapshot_portfolio = portfolio;
    m_snapshot_sequence = sequence;
    std::transform(excluded_orders.begin(), excluded_orders.end(),
      std::inserter(m_excluded_orders, m_excluded_orders.end()),
      [] (const auto& order) {
        return order->get_info().m_id;
      });
    for(auto& order : excluded_orders) {
      order->get_publisher().monitor(m_tasks.get_slot<ExecutionReport>(
        std::bind_front(
          &RiskController::on_execution_report, this, std::cref(*order))));
    }
    return std::tuple(std::move(portfolio), Beam::increment(sequence),
      std::move(excluded_orders));
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  template<typename F>
  void RiskController<A, M, O, R, T, D>::update(F&& f) {
    auto previous_state = m_state_model->get_risk_state();
    std::forward<F>(f)();
    auto& current_state = m_state_model->get_risk_state();
    if(previous_state != current_state) {
      m_transition_model->update(current_state);
      m_state_publisher.push(current_state);
    }
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskController<A, M, O, R, T, D>::on_transition_timer(
      Beam::Timer::Result result) {
    update([&] {
      m_state_model->update_time();
    });
    m_transition_timer->start();
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskController<A, M, O, R, T, D>::on_risk_parameters_update(
      const RiskParameters& parameters) {
    update([&] {
      m_state_model->update(parameters);
    });
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskController<A, M, O, R, T, D>::on_portfolio_update(
      const PortfolioUpdateEntry& update) {
    this->update([&] {
      m_portfolio_controller->get_publisher().with([&] {
        m_state_model->update_portfolio();
      });
    });
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskController<A, M, O, R, T, D>::on_order_submission(
      const SequencedOrder& order) {
    m_transition_model->add(*order);
    m_snapshot_sequence = std::max(m_snapshot_sequence, order.get_sequence());
    m_excluded_orders.insert((*order)->get_info().m_id);
    (*order)->get_publisher().monitor(m_tasks.get_slot<ExecutionReport>(
      std::bind_front(
        &RiskController::on_execution_report, this, std::cref(**order))));
  }

  template<typename A, typename M, typename O, typename R, typename T,
    typename D> requires IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>> &&
        IsOrderExecutionClient<Beam::dereference_t<O>> &&
          Beam::IsTimer<Beam::dereference_t<R>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskController<A, M, O, R, T, D>::on_execution_report(
      const Order& order, const ExecutionReport& report) {
    auto lock = std::lock_guard(m_mutex);
    if(is_terminal(report.m_status)) {
      update_snapshot(order);
    }
    m_transition_model->update(report);
  }
}

#endif
