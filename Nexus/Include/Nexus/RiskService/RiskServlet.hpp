#ifndef NEXUS_RISK_SERVLET_HPP
#define NEXUS_RISK_SERVLET_HPP
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/QueueReaderPublisher.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/SnapshotPublisher.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/RiskService/ConsolidatedRiskController.hpp"
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
  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
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

      /** Type of function used to make a unique TransitionTimer. */
      using TransitionTimerFactory =
        std::function<std::unique_ptr<TransitionTimer> ()>;

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
       * @param venues The venues used by the portfolio.
       * @param destinations The destinations used to flatten positions.
       */
      template<Beam::Initializes<A> AF, Beam::Initializes<M> MF,
        Beam::Initializes<O> OF, Beam::Initializes<T> TF,
        Beam::Initializes<D> DF>
      RiskServlet(
        Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
        AF&& administration_client, MF&& market_data_client,
        OF&& order_execution_client,
        TransitionTimerFactory transition_timer_factory, TF&& time_client,
        DF&& data_store, ExchangeRateTable exchange_rates, VenueDatabase venues,
        DestinationDatabase destinations);

      void RegisterServices(
        Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      using ConsolidatedRiskController =
        RiskService::ConsolidatedRiskController<AdministrationClient*,
          MarketDataClient*, OrderExecutionClient*, TransitionTimer,
          TimeClient*, RiskDataStore*>;
      Beam::GetOptionalLocalPtr<A> m_administration_client;
      Beam::GetOptionalLocalPtr<M> m_market_data_client;
      Beam::GetOptionalLocalPtr<O> m_order_execution_client;
      TransitionTimerFactory m_transition_timer_factory;
      Beam::GetOptionalLocalPtr<T> m_time_client;
      Beam::GetOptionalLocalPtr<D> m_data_store;
      ExchangeRateTable m_exchange_rates;
      VenueDatabase m_venues;
      DestinationDatabase m_destinations;
      std::shared_ptr<
        Beam::SnapshotPublisher<Beam::ServiceLocator::DirectoryEntry,
        std::vector<Beam::ServiceLocator::DirectoryEntry>>> m_account_publisher;
      std::unordered_map<RiskPortfolioKey, Quantity> m_volumes;
      boost::optional<ConsolidatedRiskController> m_controller;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        Beam::ServiceLocator::DirectoryEntry, Beam::Threading::Mutex>
        m_account_to_group;
      Beam::SynchronizedVector<ServiceProtocolClient*, Beam::Threading::Mutex>
        m_portfolio_subscribers;
      Beam::IO::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      RiskServlet(const RiskServlet&) = delete;
      RiskServlet& operator =(const RiskServlet&) = delete;
      void make_controller();
      void reset(const Beam::ServiceLocator::DirectoryEntry& account,
        const Region& region);
      Beam::ServiceLocator::DirectoryEntry load_group(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void on_risk_state(const RiskStateEntry& entry);
      void on_portfolio(const RiskInventoryEntry& entry);
      InventorySnapshot on_load_inventory_snapshot(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      void on_reset_region(ServiceProtocolClient& client, const Region& region);
      void on_subscribe_risk_portfolio_updates_request(
        Beam::Services::RequestToken<ServiceProtocolClient,
          SubscribeRiskPortfolioUpdatesService>& request);
  };

  template<AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  struct MetaRiskServlet {
    using Session = RiskSession;
    template<typename C>
    struct apply {
      using type = RiskServlet<C, A, M, O, R, T, D>;
    };
  };

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  template<Beam::Initializes<A> AF, Beam::Initializes<M> MF,
    Beam::Initializes<O> OF, Beam::Initializes<T> TF, Beam::Initializes<D> DF>
  RiskServlet<C, A, M, O, R, T, D>::RiskServlet(
      Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
      AF&& administration_client, MF&& market_data_client,
      OF&& order_execution_client,
      TransitionTimerFactory transition_timer_factory, TF&& time_client,
      DF&& data_store, ExchangeRateTable exchange_rates, VenueDatabase venues,
      DestinationDatabase destinations)
    : m_administration_client(std::forward<AF>(administration_client)),
      m_market_data_client(std::forward<MF>(market_data_client)),
      m_order_execution_client(std::forward<OF>(order_execution_client)),
      m_transition_timer_factory(std::move(transition_timer_factory)),
      m_time_client(std::forward<TF>(time_client)),
      m_data_store(std::forward<DF>(data_store)),
      m_exchange_rates(std::move(exchange_rates)),
      m_venues(std::move(venues)),
      m_destinations(std::move(destinations)),
      m_account_publisher(Beam::MakeSequencePublisherAdaptor(std::make_unique<
        Beam::QueueReaderPublisher<Beam::ServiceLocator::DirectoryEntry>>(
          std::move(accounts)))) {
    try {
      make_controller();
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void RiskServlet<C, A, M, O, R, T, D>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterRiskServices(Store(slots));
    RegisterRiskMessages(Store(slots));
    LoadInventorySnapshotService::AddSlot(Store(slots),
      std::bind_front(&RiskServlet::on_load_inventory_snapshot, this));
    ResetRegionService::AddSlot(
      Store(slots), std::bind_front(&RiskServlet::on_reset_region, this));
    SubscribeRiskPortfolioUpdatesService::AddRequestSlot(
      Store(slots), std::bind_front(
        &RiskServlet::on_subscribe_risk_portfolio_updates_request, this));
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void RiskServlet<C, A, M, O, R, T, D>::HandleClientAccepted(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    if(m_administration_client->check_administrator(session.GetAccount())) {
      session.add_all();
    } else {
      auto groups = m_administration_client->load_managed_trading_groups(
        session.GetAccount());
      for(auto& group : groups) {
        session.add(group);
      }
    }
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void RiskServlet<C, A, M, O, R, T, D>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_portfolio_subscribers.Remove(&client);
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void RiskServlet<C, A, M, O, R, T, D>::Close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_controller = boost::none;
    m_tasks.Break();
    m_tasks.Wait();
    m_open_state.Close();
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void RiskServlet<C, A, M, O, R, T, D>::make_controller() {
    auto accounts =
      std::make_shared<Beam::Queue<Beam::ServiceLocator::DirectoryEntry>>();
    m_account_publisher->Monitor(accounts);
    m_controller.emplace(std::move(accounts), &*m_administration_client,
      &*m_market_data_client, &*m_order_execution_client,
      m_transition_timer_factory, &*m_time_client, &*m_data_store,
      m_exchange_rates, m_venues, m_destinations);
    m_controller->get_risk_state_publisher().Monitor(
      m_tasks.GetSlot<RiskStateEntry>(
        std::bind_front(&RiskServlet::on_risk_state, this)));
    m_controller->get_portfolio_publisher().Monitor(
      m_tasks.GetSlot<RiskInventoryEntry>(
        std::bind_front(&RiskServlet::on_portfolio, this)));
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void RiskServlet<C, A, M, O, R, T, D>::reset(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Region& region) {
    auto snapshot = m_data_store->load_inventory_snapshot(account);
    auto [portfolio, sequence, excluded_orders] = make_portfolio(
      snapshot, account, m_venues, *m_order_execution_client);
    auto reports = std::vector<OrderExecutionService::ExecutionReportEntry>();
    for(auto& order : excluded_orders) {
      if(auto order_snapshot = order->get_publisher().GetSnapshot()) {
        std::transform(order_snapshot->begin(), order_snapshot->end(),
          std::back_inserter(reports), [&] (const auto& report) {
            return OrderExecutionService::ExecutionReportEntry(order, report);
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
        inventory.m_position.m_security, inventory.m_position.m_currency);
      if(base_inventory.m_position.m_security <= region) {
        if(inventory.m_position.m_quantity == 0) {
          auto update = InventoryUpdate();
          update.account = account;
          update.inventory =
            Accounting::Inventory(get_key(inventory.m_position));
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
        m_tasks.Push([this,
            key = RiskPortfolioKey(account, inventory.m_position.m_security)] {
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
      m_portfolio_subscribers.With([&] (auto& subscribers) {
        for(auto& subscriber : subscribers) {
          auto& session = subscriber->GetSession();
          if(session.has_subscription(group)) {
            Beam::Services::SendRecordMessage<InventoryMessage>(
              *subscriber, reset_inventories);
          }
        }
      });
    }
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  Beam::ServiceLocator::DirectoryEntry RiskServlet<C, A, M, O, R, T, D>::
      load_group(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_account_to_group.GetOrInsert(account, [&] {
      return m_administration_client->load_parent_trading_group(account);
    });
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void RiskServlet<C, A, M, O, R, T, D>::on_risk_state(
      const RiskStateEntry& entry) {
    m_administration_client->store(entry.m_key, entry.m_value);
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
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
    m_portfolio_subscribers.With([&] (auto& subscribers) {
      for(auto& subscriber : subscribers) {
        auto& session = subscriber->GetSession();
        if(session.GetAccount() == entry.m_key.m_account ||
            session.has_subscription(group)) {
          Beam::Services::SendRecordMessage<InventoryMessage>(
            *subscriber, inventories);
        }
      }
    });
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  InventorySnapshot RiskServlet<C, A, M, O, R, T, D>::
      on_load_inventory_snapshot(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(account != session.GetAccount() &&
        !session.has_subscription(load_group(account))) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    return m_data_store->load_inventory_snapshot(account);
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void RiskServlet<C, A, M, O, R, T, D>::on_reset_region(
      ServiceProtocolClient& client, const Region& region) {
    auto& session = client.GetSession();
    if(!m_administration_client->check_administrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    m_controller = boost::none;
    if(auto accounts = m_account_publisher->GetSnapshot()) {
      for(auto& account : *accounts) {
        try {
          reset(account, region);
        } catch(const std::exception&) {
          std::cerr << "Region reset failed for account:\n\t" << "Account: " <<
            account << "\n\t" << BEAM_REPORT_CURRENT_EXCEPTION() << std::endl;
        }
      }
    }
    make_controller();
  }

  template<typename C, AdministrationService::IsAdministrationClient A,
    MarketDataService::IsMarketDataClient M,
    OrderExecutionService::IsOrderExecutionClient O, typename R, typename T,
    IsRiskDataStore D>
  void RiskServlet<C, A, M, O, R, T, D>::
      on_subscribe_risk_portfolio_updates_request(Beam::Services::RequestToken<
        ServiceProtocolClient, SubscribeRiskPortfolioUpdatesService>& request) {
    auto& session = request.GetSession();
    m_portfolio_subscribers.With([&] (auto& subscribers) {
      auto entries = std::vector<RiskInventoryEntry>();
      subscribers.push_back(&request.GetClient());
      auto queue = std::make_shared<Beam::Queue<RiskInventoryEntry>>();
      m_controller->get_portfolio_publisher().Monitor(queue);
      while(auto entry = queue->TryPop()) {
        if(session.GetAccount() == entry->m_key.m_account ||
            session.has_subscription(load_group(entry->m_key.m_account))) {
          entries.push_back(std::move(*entry));
        }
      }
      request.SetResult(std::move(entries));
    });
  }
}

#endif
