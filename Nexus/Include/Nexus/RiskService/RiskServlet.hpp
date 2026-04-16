#ifndef NEXUS_RISK_SERVLET_HPP
#define NEXUS_RISK_SERVLET_HPP
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/QueueReaderPublisher.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/SnapshotPublisher.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/RiskService/ConsolidatedRiskController.hpp"
#include "Nexus/RiskService/RiskServices.hpp"
#include "Nexus/RiskService/RiskSession.hpp"

namespace Nexus {

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
  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  class RiskServlet {
    public:

      /** The type of AdministrationClient to use used. */
      using AdministrationClient = Beam::dereference_t<A>;

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::dereference_t<M>;

      /** The type of OrderExecutionClient to use. */
      using OrderExecutionClient = Beam::dereference_t<O>;

      /** The type of TransitionTimer to use. */
      using TransitionTimer = R;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::dereference_t<T>;

      /** The type of RiskDataStore to use. */
      using RiskDataStore = Beam::dereference_t<D>;

      /** Type of function used to make a unique TransitionTimer. */
      using TransitionTimerFactory =
        std::function<std::unique_ptr<TransitionTimer> ()>;

      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * Constructs a RiskServlet.
       * @param accounts Publishes the accounts whose risk is to be managed.
       * @param administration_client Initializes the AdministrationClient.
       * @param market_data_client Initializes the MarketDataClient.
       * @param order_execution_client Initializes the OrderExecutionClient.
       * @param transition_timer_factory The function used to build transition
       *        Timers.
       * @param time_client Initializes the TimeClient.
       * @param data_store Initializes the RiskDataStore.
       * @param exchange_rates The exchange rates.
       * @param destinations The destinations used to flatten positions.
       */
      template<Beam::Initializes<A> AF, Beam::Initializes<M> MF,
        Beam::Initializes<O> OF, Beam::Initializes<T> TF,
        Beam::Initializes<D> DF>
      RiskServlet(Beam::ScopedQueueReader<Beam::DirectoryEntry> accounts,
        AF&& administration_client, MF&& market_data_client,
        OF&& order_execution_client,
        TransitionTimerFactory transition_timer_factory, TF&& time_client,
        DF&& data_store, ExchangeRateTable exchange_rates,
        DestinationDatabase destinations);

      void register_services(
        Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots);
      void handle_accept(ServiceProtocolClient& client);
      void handle_close(ServiceProtocolClient& client);
      void close();

    private:
      using ConsolidatedRiskController =
        Nexus::ConsolidatedRiskController<AdministrationClient*,
          MarketDataClient*, OrderExecutionClient*, TransitionTimer,
          TimeClient*, RiskDataStore*>;
      Beam::local_ptr_t<A> m_administration_client;
      Beam::local_ptr_t<M> m_market_data_client;
      Beam::local_ptr_t<O> m_order_execution_client;
      TransitionTimerFactory m_transition_timer_factory;
      Beam::local_ptr_t<T> m_time_client;
      Beam::local_ptr_t<D> m_data_store;
      ExchangeRateTable m_exchange_rates;
      DestinationDatabase m_destinations;
      std::shared_ptr<Beam::SnapshotPublisher<Beam::DirectoryEntry,
        std::vector<Beam::DirectoryEntry>>> m_account_publisher;
      std::unordered_map<RiskPortfolioKey, Quantity> m_volumes;
      boost::optional<ConsolidatedRiskController> m_controller;
      Beam::SynchronizedUnorderedMap<Beam::DirectoryEntry, Beam::DirectoryEntry,
        Beam::Mutex> m_account_to_group;
      Beam::SynchronizedVector<ServiceProtocolClient*, Beam::Mutex>
        m_portfolio_subscribers;
      Beam::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      RiskServlet(const RiskServlet&) = delete;
      RiskServlet& operator =(const RiskServlet&) = delete;
      void make_controller();
      void reset(const Beam::DirectoryEntry& account, const Scope& scope);
      Beam::DirectoryEntry load_group(const Beam::DirectoryEntry& account);
      void on_risk_state(const RiskStateEntry& entry);
      void on_portfolio(const RiskInventoryEntry& entry);
      InventorySnapshot on_load_inventory_snapshot(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account);
      void on_reset_scope(ServiceProtocolClient& client, const Scope& scope);
      void on_subscribe_risk_portfolio_updates_request(
        Beam::RequestToken<ServiceProtocolClient,
          SubscribeRiskPortfolioUpdatesService>& request);
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

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  template<Beam::Initializes<A> AF, Beam::Initializes<M> MF,
    Beam::Initializes<O> OF, Beam::Initializes<T> TF, Beam::Initializes<D> DF>
  RiskServlet<C, A, M, O, R, T, D>::RiskServlet(
      Beam::ScopedQueueReader<Beam::DirectoryEntry> accounts,
      AF&& administration_client, MF&& market_data_client,
      OF&& order_execution_client,
      TransitionTimerFactory transition_timer_factory, TF&& time_client,
      DF&& data_store, ExchangeRateTable exchange_rates,
      DestinationDatabase destinations)
    : m_administration_client(std::forward<AF>(administration_client)),
      m_market_data_client(std::forward<MF>(market_data_client)),
      m_order_execution_client(std::forward<OF>(order_execution_client)),
      m_transition_timer_factory(std::move(transition_timer_factory)),
      m_time_client(std::forward<TF>(time_client)),
      m_data_store(std::forward<DF>(data_store)),
      m_exchange_rates(std::move(exchange_rates)),
      m_destinations(std::move(destinations)),
      m_account_publisher(Beam::make_sequence_publisher_adaptor(
        std::make_unique<Beam::QueueReaderPublisher<Beam::DirectoryEntry>>(
          std::move(accounts)))) {
    try {
      make_controller();
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskServlet<C, A, M, O, R, T, D>::register_services(
      Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots) {
    register_risk_services(out(slots));
    register_risk_messages(out(slots));
    LoadInventorySnapshotService::add_slot(out(slots),
      std::bind_front(&RiskServlet::on_load_inventory_snapshot, this));
    ResetScopeService::add_slot(
      out(slots), std::bind_front(&RiskServlet::on_reset_scope, this));
    SubscribeRiskPortfolioUpdatesService::add_request_slot(
      out(slots), std::bind_front(
        &RiskServlet::on_subscribe_risk_portfolio_updates_request, this));
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskServlet<C, A, M, O, R, T, D>::handle_accept(
      ServiceProtocolClient& client) {
    auto& session = client.get_session();
    if(m_administration_client->check_administrator(session.get_account())) {
      session.add_all();
    } else {
      auto groups = m_administration_client->load_managed_trading_groups(
        session.get_account());
      for(auto& group : groups) {
        session.add(group);
      }
    }
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskServlet<C, A, M, O, R, T, D>::handle_close(
      ServiceProtocolClient& client) {
    m_portfolio_subscribers.erase(&client);
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskServlet<C, A, M, O, R, T, D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_controller = boost::none;
    m_tasks.close();
    m_tasks.wait();
    m_open_state.close();
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskServlet<C, A, M, O, R, T, D>::make_controller() {
    auto accounts = std::make_shared<Beam::Queue<Beam::DirectoryEntry>>();
    m_account_publisher->monitor(accounts);
    m_controller.emplace(std::move(accounts), &*m_administration_client,
      &*m_market_data_client, &*m_order_execution_client,
      m_transition_timer_factory, &*m_time_client, &*m_data_store,
      m_exchange_rates, m_destinations);
    m_controller->get_risk_state_publisher().monitor(
      m_tasks.get_slot<RiskStateEntry>(
        std::bind_front(&RiskServlet::on_risk_state, this)));
    m_controller->get_portfolio_publisher().monitor(
      m_tasks.get_slot<RiskInventoryEntry>(
        std::bind_front(&RiskServlet::on_portfolio, this)));
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskServlet<C, A, M, O, R, T, D>::reset(
      const Beam::DirectoryEntry& account, const Scope& scope) {
    auto snapshot = m_data_store->load_inventory_snapshot(account);
    auto [portfolio, sequence, excluded_orders] =
      make_portfolio(snapshot, account, *m_order_execution_client);
    auto reports = std::vector<ExecutionReportEntry>();
    for(auto& order : excluded_orders) {
      if(auto order_snapshot = order->get_publisher().get_snapshot()) {
        std::transform(order_snapshot->begin(), order_snapshot->end(),
          std::back_inserter(reports), [&] (const auto& report) {
            return ExecutionReportEntry(order, report);
          });
      }
    }
    std::sort(reports.begin(), reports.end(),
      [] (const auto& left, const auto& right) {
        return std::tie(left.m_report.m_timestamp, left.m_report.m_id,
          left.m_report.m_sequence) < std::tie(right.m_report.m_timestamp,
            right.m_report.m_id, right.m_report.m_sequence);
      });
    auto base_portfolio = portfolio;
    for(auto& report : reports) {
      portfolio.update(report.m_order->get_info().m_fields, report.m_report);
    }
    auto updated_snapshot = InventorySnapshot();
    auto reset_inventories = std::vector<InventoryUpdate>();
    for(auto inventory : portfolio.get_bookkeeper().get_inventory_range()) {
      auto& base_inventory = base_portfolio.get_bookkeeper().get_inventory(
        inventory.m_position.m_ticker);
      if(base_inventory.m_position.m_ticker <= scope) {
        if(inventory.m_position.m_quantity == 0) {
          auto update = InventoryUpdate();
          update.account = account;
          update.inventory = Inventory(
            inventory.m_position.m_ticker, inventory.m_position.m_currency);
          reset_inventories.push_back(update);
        }
        inventory.m_position.m_quantity = base_inventory.m_position.m_quantity;
        inventory.m_position.m_cost_basis =
          base_inventory.m_position.m_cost_basis;
        inventory.m_fees = base_inventory.m_fees - inventory.m_fees;
        inventory.m_gross_profit_and_loss =
          base_inventory.m_gross_profit_and_loss -
            inventory.m_gross_profit_and_loss;
        inventory.m_volume = base_inventory.m_volume - inventory.m_volume;
        inventory.m_transaction_count = base_inventory.m_transaction_count -
          inventory.m_transaction_count;
        updated_snapshot.m_inventories.push_back(inventory);
        m_tasks.push([this,
            key = RiskPortfolioKey(account, inventory.m_position.m_ticker)] {
          m_volumes[key] = -1;
        });
      } else {
        updated_snapshot.m_inventories.push_back(base_inventory);
      }
    }
    updated_snapshot.m_sequence = sequence;
    std::transform(excluded_orders.begin(), excluded_orders.end(),
      std::back_inserter(updated_snapshot.m_excluded_orders),
      [] (const auto& order) {
        return order->get_info().m_id;
      });
    m_data_store->store(account, updated_snapshot);
    if(!reset_inventories.empty()) {
      auto group = load_group(account);
      m_portfolio_subscribers.with([&] (auto& subscribers) {
        for(auto& subscriber : subscribers) {
          auto& session = subscriber->get_session();
          if(session.has_subscription(group)) {
            Beam::send_record_message<InventoryMessage>(
              *subscriber, reset_inventories);
          }
        }
      });
    }
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  Beam::DirectoryEntry RiskServlet<C, A, M, O, R, T, D>::
      load_group(const Beam::DirectoryEntry& account) {
    return m_account_to_group.get_or_insert(account, [&] {
      return m_administration_client->load_parent_trading_group(account);
    });
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskServlet<C, A, M, O, R, T, D>::on_risk_state(
      const RiskStateEntry& entry) {
    m_administration_client->store(entry.m_key, entry.m_value);
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskServlet<C, A, M, O, R, T, D>::on_portfolio(
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
    auto group = load_group(entry.m_key.m_account);
    m_portfolio_subscribers.for_each([&] (auto& subscriber) {
      auto& session = subscriber->get_session();
      if(session.get_account() == entry.m_key.m_account ||
          session.has_subscription(group)) {
        Beam::send_record_message<InventoryMessage>(*subscriber, inventories);
      }
    });
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  InventorySnapshot RiskServlet<C, A, M, O, R, T, D>::
      on_load_inventory_snapshot(ServiceProtocolClient& client,
        const Beam::DirectoryEntry& account) {
    auto& session = client.get_session();
    if(account != session.get_account() &&
        !session.has_subscription(load_group(account))) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return m_data_store->load_inventory_snapshot(account);
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskServlet<C, A, M, O, R, T, D>::on_reset_scope(
      ServiceProtocolClient& client, const Scope& scope) {
    auto& session = client.get_session();
    if(!m_administration_client->check_administrator(session.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    m_controller = boost::none;
    if(auto accounts = m_account_publisher->get_snapshot()) {
      for(auto& account : *accounts) {
        try {
          reset(account, scope);
        } catch(const std::exception&) {
          std::cerr << "Scope reset failed for account:\n\t" << "Account: " <<
            account << "\n\t" << BEAM_REPORT_CURRENT_EXCEPTION() << std::endl;
        }
      }
    }
    make_controller();
  }

  template<typename C, typename A, typename M, typename O, Beam::IsTimer R,
    typename T, typename D> requires
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsMarketDataClient<Beam::dereference_t<M>> &&
          IsOrderExecutionClient<Beam::dereference_t<O>> &&
            Beam::IsTimeClient<Beam::dereference_t<T>> &&
              IsRiskDataStore<Beam::dereference_t<D>>
  void RiskServlet<C, A, M, O, R, T, D>::
      on_subscribe_risk_portfolio_updates_request(Beam::RequestToken<
        ServiceProtocolClient, SubscribeRiskPortfolioUpdatesService>& request) {
    auto& session = request.get_session();
    m_portfolio_subscribers.with([&] (auto& subscribers) {
      auto entries = std::vector<RiskInventoryEntry>();
      subscribers.push_back(&request.get_client());
      auto queue = std::make_shared<Beam::Queue<RiskInventoryEntry>>();
      m_controller->get_portfolio_publisher().monitor(queue);
      while(auto entry = queue->try_pop()) {
        if(session.get_account() == entry->m_key.m_account ||
            session.has_subscription(load_group(entry->m_key.m_account))) {
          entries.push_back(std::move(*entry));
        }
      }
      request.set(std::move(entries));
    });
  }
}

#endif
