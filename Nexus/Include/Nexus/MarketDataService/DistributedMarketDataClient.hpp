#ifndef NEXUS_DISTRIBUTED_MARKET_DATA_CLIENT_HPP
#define NEXUS_DISTRIBUTED_MARKET_DATA_CLIENT_HPP
#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <unordered_set>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include <Beam/ServiceLocator/ServiceUpdate.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <Beam/Utilities/Expect.hpp>
#include "Nexus/Definitions/ScopeMap.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus {

  /**
   * Implements a MarketDataClient whose servers are distributed among multiple
   * instances.
   * @tparam T The timer used to retry failed connections.
   */
  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  class DistributedMarketDataClient {
    public:

      /** The timer used to retry failed connections. */
      using Timer = Beam::dereference_t<T>;

      /** Subscribes to market data service registrations. */
      using ServiceMonitor =
        std::function<void (Beam::ScopedQueueWriter<Beam::ServiceUpdate>)>;

      /** Returns the scope covered by a service registration. */
      using ScopeParser = std::function<Scope (const Beam::ServiceEntry&)>;

      /** Connects to a server covering a scope. */
      using ClientBuilder =
        std::function<std::shared_ptr<MarketDataClient> (const Scope&)>;

      /**
       * Constructs a DistributedMarketDataClient.
       * @param market_data_clients Maps scopes to the appropriate market data
       *        client.
       */
      explicit DistributedMarketDataClient(
        ScopeMap<std::shared_ptr<MarketDataClient>> market_data_clients);

      /**
       * Constructs a client that discovers scopes from service updates.
       * @param service_monitor Subscribes to available services and updates.
       * @param scope_parser Returns the scope covered by a registration.
       * @param client_builder Connects to a scope in a single attempt.
       * @param timer Determines how often to retry failed connections.
       */
      template<Beam::Initializes<T> TF>
      DistributedMarketDataClient(ServiceMonitor service_monitor,
        ScopeParser scope_parser, ClientBuilder client_builder, TF&& timer);

      ~DistributedMarketDataClient();

      void query(const VenueQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);
      void query(const VenueQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<SequencedTickerStatus> queue);
      void query(
        const TickerQuery& query, Beam::ScopedQueueWriter<TickerStatus> queue);
      std::vector<TickerInfo> query(const TickerInfoQuery& query);
      TickerSnapshot load_snapshot(const Ticker& ticker);
      SequencedSessionTechnicals load_session_technicals(const Ticker& ticker);
      std::vector<TickerInfo> load_ticker_info_from_prefix(
        const std::string& prefix);
      void close();

    private:
      Beam::Sync<ScopeMap<std::shared_ptr<MarketDataClient>>>
        m_market_data_clients;
      Beam::ConditionVariable m_available;
      std::exception_ptr m_discovery_exception;
      ScopeParser m_scope_parser;
      ClientBuilder m_client_builder;
      std::vector<std::pair<Beam::ServiceEntry, Scope>> m_services;
      std::optional<Beam::local_ptr_t<T>> m_timer;
      Beam::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      DistributedMarketDataClient(const DistributedMarketDataClient&) = delete;
      DistributedMarketDataClient& operator =(
        const DistributedMarketDataClient&) = delete;
      std::shared_ptr<MarketDataClient> find_client(const Scope& scope);
      std::shared_ptr<MarketDataClient> get_client(const Scope& scope);
      std::unordered_set<std::shared_ptr<MarketDataClient>> get_clients();
      void add_client(const Scope& scope);
      void on_service_update(const Beam::ServiceUpdate& update);
      void on_service_break(const std::exception_ptr& exception);
      void on_timer(Beam::Timer::Result result);
  };

  DistributedMarketDataClient(
    ScopeMap<std::shared_ptr<MarketDataClient>>) ->
      DistributedMarketDataClient<Beam::Timer>;

  template<typename M, typename P, typename B, typename T>
  DistributedMarketDataClient(M, P, B, T&&) ->
    DistributedMarketDataClient<std::remove_cvref_t<T>>;

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  DistributedMarketDataClient<T>::DistributedMarketDataClient(
    ScopeMap<std::shared_ptr<MarketDataClient>> market_data_clients)
    : m_market_data_clients(std::move(market_data_clients)) {}

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<T> TF>
  DistributedMarketDataClient<T>::DistributedMarketDataClient(
      ServiceMonitor service_monitor, ScopeParser scope_parser,
      ClientBuilder client_builder, TF&& timer)
      : m_market_data_clients(nullptr),
        m_scope_parser(std::move(scope_parser)),
        m_client_builder(std::move(client_builder)),
        m_timer(std::in_place, std::forward<TF>(timer)) {
    try {
      (*m_timer)->get_publisher().monitor(m_tasks.get_slot<Beam::Timer::Result>(
        std::bind_front(&DistributedMarketDataClient::on_timer, this)));
      service_monitor(m_tasks.get_slot<Beam::ServiceUpdate>(
        std::bind_front(&DistributedMarketDataClient::on_service_update, this),
        std::bind_front(&DistributedMarketDataClient::on_service_break, this)));
      (*m_timer)->start();
    } catch(...) {
      close();
      throw;
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  DistributedMarketDataClient<T>::~DistributedMarketDataClient() {
    close();
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::query(const VenueQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    if(auto client = get_client(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::query(const VenueQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    if(auto client = get_client(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    if(auto client = get_client(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    if(auto client = get_client(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    if(auto client = get_client(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    if(auto client = get_client(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    if(auto client = get_client(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<TimeAndSale> queue) {
    if(auto client = get_client(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedTickerStatus> queue) {
    if(auto client = get_client(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<TickerStatus> queue) {
    if(auto client = get_client(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  std::vector<TickerInfo> DistributedMarketDataClient<T>::query(
      const TickerInfoQuery& query) {
    auto& scope = query.get_index();
    if(m_client_builder && scope.get_countries().empty() &&
        scope.get_venues().empty() && scope.get_tickers().size() == 1) {
      return get_client(scope)->query(query);
    }
    if(auto client = find_client(scope)) {
      return client->query(query);
    }
    auto limit = query.get_snapshot_limit();
    if(limit.get_size() == 0) {
      return {};
    }
    auto request = query;
    request.set_offset(0);
    request.set_snapshot_limit(Beam::SnapshotLimit(limit.get_type(),
      limit.get_size() + std::min(query.get_offset(),
        std::numeric_limits<int>::max() - limit.get_size())));
    auto infos = std::vector<TickerInfo>();
    for(auto& client : get_clients()) {
      auto result = client->query(request);
      infos.insert(infos.end(), std::make_move_iterator(result.begin()),
        std::make_move_iterator(result.end()));
    }
    std::ranges::sort(infos, std::ranges::less(), &TickerInfo::m_ticker);
    auto duplicates = std::ranges::unique(
      infos, std::ranges::equal_to(), &TickerInfo::m_ticker);
    infos.erase(duplicates.begin(), duplicates.end());
    auto offset = std::min<std::size_t>(query.get_offset(), infos.size());
    auto size = std::min<std::size_t>(limit.get_size(), infos.size() - offset);
    auto begin = [&] {
      if(limit.get_type() == Beam::SnapshotLimit::Type::HEAD) {
        return infos.begin() + offset;
      }
      return infos.end() - offset - size;
    }();
    return std::vector<TickerInfo>(
      std::make_move_iterator(begin), std::make_move_iterator(begin + size));
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  TickerSnapshot DistributedMarketDataClient<T>::load_snapshot(
      const Ticker& ticker) {
    if(auto client = get_client(ticker)) {
      return client->load_snapshot(ticker);
    }
    return {};
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  SequencedSessionTechnicals
      DistributedMarketDataClient<T>::load_session_technicals(
        const Ticker& ticker) {
    if(auto client = get_client(ticker)) {
      return client->load_session_technicals(ticker);
    }
    return {};
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  std::vector<TickerInfo>
      DistributedMarketDataClient<T>::load_ticker_info_from_prefix(
        const std::string& prefix) {
    auto ticker_infos = std::vector<TickerInfo>();
    for(auto& client : get_clients()) {
      auto result = client->load_ticker_info_from_prefix(prefix);
      ticker_infos.insert(ticker_infos.end(),
        std::make_move_iterator(result.begin()),
        std::make_move_iterator(result.end()));
    }
    return ticker_infos;
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_market_data_clients.with([&] (const auto&) {
      m_available.notify_all();
    });
    m_tasks.close();
    m_tasks.wait();
    if(m_timer) {
      (*m_timer)->cancel();
    }
    m_market_data_clients.exchange(
      ScopeMap<std::shared_ptr<MarketDataClient>>(nullptr));
    m_open_state.close();
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  std::shared_ptr<MarketDataClient>
      DistributedMarketDataClient<T>::find_client(const Scope& scope) {
    return m_market_data_clients.with([&] (const auto& clients) {
      return clients.get(scope);
    });
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  std::shared_ptr<MarketDataClient>
      DistributedMarketDataClient<T>::get_client(const Scope& scope) {
    return m_market_data_clients.with([&] (const auto& clients) {
      auto& lock = m_market_data_clients.get_lock();
      while(true) {
        m_open_state.ensure_open();
        auto client = clients.get(scope);
        if(client || !m_client_builder) {
          return client;
        }
        if(m_discovery_exception) {
          std::rethrow_exception(m_discovery_exception);
        }
        m_available.wait(lock);
      }
    });
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  std::unordered_set<std::shared_ptr<MarketDataClient>>
      DistributedMarketDataClient<T>::get_clients() {
    return m_market_data_clients.with([&] (const auto& clients) {
      auto& lock = m_market_data_clients.get_lock();
      while(true) {
        m_open_state.ensure_open();
        auto result = std::unordered_set<std::shared_ptr<MarketDataClient>>();
        for(auto& [scope, client] : clients) {
          if(client) {
            result.insert(client);
          }
        }
        if(!result.empty() || !m_client_builder) {
          return result;
        }
        if(m_discovery_exception) {
          std::rethrow_exception(m_discovery_exception);
        }
        m_available.wait(lock);
      }
    });
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::add_client(const Scope& scope) {
    if(!m_open_state.is_open()) {
      return;
    }
    auto skip = m_market_data_clients.with([&] (const auto& clients) {
      if(m_discovery_exception) {
        return true;
      }
      auto entry = clients.find(scope);
      return std::get<0>(*entry) == scope && std::get<1>(*entry);
    });
    if(skip) {
      return;
    }
    try {
      auto client = m_client_builder(scope);
      m_market_data_clients.with([&] (auto& clients) {
        clients.set(scope, client);
        m_available.notify_all();
      });
    } catch(const Beam::AuthenticationException&) {
      m_market_data_clients.with([&] (const auto&) {
        m_discovery_exception = std::current_exception();
        m_available.notify_all();
      });
      Beam::report_current_exception();
    } catch(const Beam::ConnectException&) {
    } catch(...) {
      Beam::report_current_exception();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::on_service_update(
      const Beam::ServiceUpdate& update) {
    if(!m_open_state.is_open()) {
      return;
    }
    if(update.m_type == Beam::ServiceUpdate::Type::REMOVED) {
      std::erase_if(m_services, [&] (const auto& entry) {
        return entry.first == update.m_service;
      });
      return;
    }
    try {
      auto scope = m_scope_parser(update.m_service);
      auto entry = std::ranges::find_if(m_services, [&] (const auto& entry) {
        return entry.first.get_id() == update.m_service.get_id();
      });
      if(entry == m_services.end()) {
        m_services.emplace_back(update.m_service, scope);
      } else {
        *entry = std::pair(update.m_service, scope);
      }
      add_client(scope);
    } catch(...) {
      Beam::report_current_exception();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::on_service_break(
      const std::exception_ptr& exception) {
    if(!m_open_state.is_open()) {
      return;
    }
    m_services.clear();
    m_market_data_clients.with([&] (const auto&) {
      m_discovery_exception = exception;
      m_available.notify_all();
    });
    try {
      std::rethrow_exception(exception);
    } catch(const Beam::PipeBrokenException&) {
    } catch(...) {
      Beam::report_current_exception();
    }
  }

  template<typename T> requires Beam::IsTimer<Beam::dereference_t<T>>
  void DistributedMarketDataClient<T>::on_timer(
      Beam::Timer::Result result) {
    if(!m_open_state.is_open() || result == Beam::Timer::Result::CANCELED) {
      return;
    }
    auto scopes = std::unordered_set<Scope>();
    for(auto& [service, scope] : m_services) {
      if(scopes.insert(scope).second) {
        add_client(scope);
      }
    }
    if(m_open_state.is_open()) {
      (*m_timer)->start();
    }
  }
}

#endif
