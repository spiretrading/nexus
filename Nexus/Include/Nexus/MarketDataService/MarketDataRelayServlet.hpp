#ifndef NEXUS_MARKET_DATA_RELAY_SERVLET_HPP
#define NEXUS_MARKET_DATA_RELAY_SERVLET_HPP
#include <cstdint>
#include <functional>
#include <type_traits>
#include <vector>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/IndexedSubscriptions.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Services/ServiceProtocolServlet.hpp>
#include <Beam/Utilities/ResourcePool.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"
#include "Nexus/MarketDataService/MarketDataRegistrySession.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {

  /**
   * Implements a relay servlet for querying market data.
   * @param C container instantiating this servlet.
   * @param M The type of MarketDataClient connected to the source providing
   *          market data queries.
   * @param A The type of AdministrationClient to use.
   */
  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  class MarketDataRelayServlet {
    public:

      /**
       * The type of MarketDataClient connected to the source providing market
       * data queries.
       */
      using MarketDataClient = Beam::dereference_t<M>;

      /** The type of AdministrationClient to use. */
      using AdministrationClient = Beam::dereference_t<A>;

      /** The type of function used to builds MarketDataClients. */
      using MarketDataClientBuilder =
        std::function<std::unique_ptr<MarketDataClient> ()>;

      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * Constructs a MarketDataRelayServlet.
       * @param client_timeout The amount of time to wait before building
       *        another MarketDataClient.
       * @param market_data_client_builder Constructs MarketDataClients used to
       *        distribute queries.
       * @param min_market_data_clients The minimum number of MarketDataClients
       *        to pool.
       * @param max_market_data_clients The maximum number of MarketDataClients
       *        to pool.
       * @param administration_client Used to check for entitlements.
       */
      template<Beam::Initializes<A> AF>
      MarketDataRelayServlet(boost::posix_time::time_duration client_timeout,
        MarketDataClientBuilder market_data_client_builder,
        std::size_t min_market_data_clients,
        std::size_t max_market_data_clients, AF&& administrationClient);

      void register_services(
        Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots);
      void handle_accept(ServiceProtocolClient& client);
      void handle_close(ServiceProtocolClient& client);
      void close();

    private:
      struct RealTimeQueryEntry {
        std::unique_ptr<MarketDataClient> m_market_data_client;
        Beam::RoutineTaskQueue m_tasks;

        RealTimeQueryEntry(
          std::unique_ptr<MarketDataClient> market_data_client);
      };
      template<typename T>
      using VenueSubscriptions =
        Beam::IndexedSubscriptions<T, Venue, ServiceProtocolClient>;
      template<typename T>
      using SecuritySubscriptions =
        Beam::IndexedSubscriptions<T, Security, ServiceProtocolClient>;
      using RealTimeVenueSubscriptionSet =
        Beam::SynchronizedUnorderedSet<Venue, Beam::Mutex>;
      using RealTimeSecuritySubscriptionSet =
        Beam::SynchronizedUnorderedSet<Security, Beam::Mutex>;
      VenueSubscriptions<OrderImbalance> m_order_imbalance_subscriptions;
      SecuritySubscriptions<BboQuote> m_bbo_quote_subscriptions;
      SecuritySubscriptions<BookQuote> m_book_quote_subscriptions;
      SecuritySubscriptions<TimeAndSale> m_time_and_sale_subscriptions;
      RealTimeVenueSubscriptionSet m_order_imbalance_real_time_subscriptions;
      RealTimeSecuritySubscriptionSet m_bbo_quote_real_time_subscriptions;
      RealTimeSecuritySubscriptionSet m_book_quote_real_time_subscriptions;
      RealTimeSecuritySubscriptionSet m_time_and_sale_real_time_subscriptions;
      Beam::SynchronizedUnorderedSet<Security> m_securities;
      Beam::ResourcePool<MarketDataClient, MarketDataClientBuilder>
        m_market_data_clients;
      Beam::local_ptr_t<A> m_administration_client;
      EntitlementDatabase m_entitlement_database;
      Beam::OpenState m_open_state;
      std::vector<std::unique_ptr<RealTimeQueryEntry>>
        m_real_time_query_entries;

      MarketDataRelayServlet(const MarketDataRelayServlet&) = delete;
      MarketDataRelayServlet& operator =(
        const MarketDataRelayServlet&) = delete;
      template<typename T>
      RealTimeQueryEntry& get_real_time_query_entry(const T& index);
      template<typename Service, typename Query, typename Subscriptions,
        typename RealTimeSubscriptions>
      void handle_query_request(
        Beam::RequestToken<ServiceProtocolClient, Service>& request,
        const Query& query, Subscriptions& subscriptions,
        RealTimeSubscriptions& real_time_subscriptions);
      template<typename Subscriptions>
      void on_end_query(ServiceProtocolClient& client,
        const typename Subscriptions::Index& index, int id,
        Subscriptions& subscriptions);
      SecuritySnapshot on_load_security_snapshot(
        ServiceProtocolClient& client, const Security& security);
      SecurityTechnicals on_load_security_technicals(
        ServiceProtocolClient& client, const Security& security);
      std::vector<SecurityInfo> on_query_security_info(
        ServiceProtocolClient& client, const SecurityInfoQuery& query);
      std::vector<SecurityInfo> on_load_security_info_from_prefix(
        ServiceProtocolClient& client, const std::string& prefix);
      template<typename Index, typename Value, typename Subscriptions>
      std::enable_if_t<!std::is_same_v<Value, SequencedBookQuote>>
        on_real_time_update(const Index& index, const Value& value,
          Subscriptions& subscriptions);
      template<typename Index, typename Value, typename Subscriptions>
      std::enable_if_t<std::is_same_v<Value, SequencedBookQuote>>
        on_real_time_update(const Index& index, const Value& value,
          Subscriptions& subscriptions);
  };

  template<typename M, typename A>
  struct MetaMarketDataRelayServlet {
    using Session = MarketDataRegistrySession;
    static constexpr auto SUPPORTS_PARALLELISM = true;

    template<typename C>
    struct apply {
      using type = MarketDataRelayServlet<C, M, A>;
    };
  };

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  MarketDataRelayServlet<C, M, A>::RealTimeQueryEntry::RealTimeQueryEntry(
    std::unique_ptr<MarketDataClient> market_data_client)
    : m_market_data_client(std::move(market_data_client)) {}

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  template<Beam::Initializes<A> AF>
  MarketDataRelayServlet<C, M, A>::MarketDataRelayServlet(
      boost::posix_time::time_duration client_timeout,
      MarketDataClientBuilder market_data_client_builder,
      std::size_t min_market_data_clients, std::size_t max_market_data_clients,
      AF&& administration_client)
      : m_market_data_clients(client_timeout, market_data_client_builder,
          min_market_data_clients, max_market_data_clients),
        m_administration_client(std::forward<AF>(administration_client)),
        m_entitlement_database(m_administration_client->load_entitlements()) {
    for(auto i = std::size_t(0); i < boost::thread::hardware_concurrency();
        ++i) {
      m_real_time_query_entries.emplace_back(
        std::make_unique<RealTimeQueryEntry>(market_data_client_builder()));
    }
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRelayServlet<C, M, A>::register_services(
      Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots) {
    Nexus::register_query_types(Beam::out(slots->get_registry()));
    register_market_data_registry_services(out(slots));
    register_market_data_registry_messages(out(slots));
    QueryOrderImbalancesService::add_request_slot(out(slots),
      [=, this] (auto& request, const auto& query) {
        handle_query_request(request, query, m_order_imbalance_subscriptions,
          m_order_imbalance_real_time_subscriptions);
      });
    Beam::add_message_slot<EndOrderImbalanceQueryMessage>(out(slots),
      [=, this] (auto& client, const auto& index, auto id) {
        on_end_query(client, index, id, m_order_imbalance_subscriptions);
      });
    QueryBboQuotesService::add_request_slot(out(slots),
      [=, this] (auto& request, const auto& query) {
        handle_query_request(request, query, m_bbo_quote_subscriptions,
          m_bbo_quote_real_time_subscriptions);
      });
    Beam::add_message_slot<EndBboQuoteQueryMessage>(out(slots),
      [=, this] (auto& client, const auto& index, auto id) {
        on_end_query(client, index, id, m_bbo_quote_subscriptions);
      });
    QueryBookQuotesService::add_request_slot(out(slots),
      [=, this] (auto& request, const auto& query) {
        handle_query_request(request, query, m_book_quote_subscriptions,
          m_book_quote_real_time_subscriptions);
      });
    Beam::add_message_slot<EndBookQuoteQueryMessage>(out(slots),
      [=, this] (auto& client, const auto& index, auto id) {
        on_end_query(client, index, id, m_book_quote_subscriptions);
      });
    QueryTimeAndSalesService::add_request_slot(out(slots),
      [=, this] (auto& request, const auto& query) {
        handle_query_request(request, query, m_time_and_sale_subscriptions,
          m_time_and_sale_real_time_subscriptions);
      });
    Beam::add_message_slot<EndTimeAndSaleQueryMessage>(out(slots),
      [=, this] (auto& client, const auto& index, auto id) {
        on_end_query(client, index, id, m_time_and_sale_subscriptions);
      });
    LoadSecuritySnapshotService::add_slot(out(slots), std::bind_front(
      &MarketDataRelayServlet::on_load_security_snapshot, this));
    LoadSecurityTechnicalsService::add_slot(out(slots), std::bind_front(
      &MarketDataRelayServlet::on_load_security_technicals, this));
    QuerySecurityInfoService::add_slot(out(slots),
      std::bind_front(&MarketDataRelayServlet::on_query_security_info, this));
    LoadSecurityInfoFromPrefixService::add_slot(out(slots), std::bind_front(
      &MarketDataRelayServlet::on_load_security_info_from_prefix, this));
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRelayServlet<C, M, A>::handle_accept(
      ServiceProtocolClient& client) {
    auto& session = client.get_session();
    session.m_roles =
      m_administration_client->load_account_roles(session.get_account());
    auto account_entitlements =
      m_administration_client->load_entitlements(session.get_account());
    for(auto& entitlement : m_entitlement_database.get_entries()) {
      auto i = std::find(account_entitlements.begin(),
        account_entitlements.end(), entitlement.m_group_entry);
      if(i != account_entitlements.end()) {
        for(auto& applicability : entitlement.m_applicability) {
          session.m_entitlements.grant(
            applicability.first, applicability.second);
        }
      }
    }
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRelayServlet<C, M, A>::handle_close(
      ServiceProtocolClient& client) {
    m_order_imbalance_subscriptions.remove_all(client);
    m_bbo_quote_subscriptions.remove_all(client);
    m_book_quote_subscriptions.remove_all(client);
    m_time_and_sale_subscriptions.remove_all(client);
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRelayServlet<C, M, A>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    auto close_group = Beam::RoutineHandlerGroup();
    for(auto& entry : m_real_time_query_entries) {
      close_group.spawn([&] {
        entry->m_tasks.close();
        entry->m_tasks.wait();
        entry->m_market_data_client->close();
      });
    }
    auto pooled_clients = std::vector<
      Beam::ScopedResource<MarketDataClient, MarketDataClientBuilder>>();
    while(auto client = m_market_data_clients.try_load()) {
      pooled_clients.push_back(std::move(*client));
    }
    for(auto& client : pooled_clients) {
      close_group.spawn([&] {
        client->close();
      });
    }
    close_group.wait();
    m_open_state.close();
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  template<typename T>
  typename MarketDataRelayServlet<C, M, A>::RealTimeQueryEntry&
      MarketDataRelayServlet<C, M, A>::get_real_time_query_entry(
        const T& index) {
    auto i = std::hash<T>()(index) % m_real_time_query_entries.size();
    return *m_real_time_query_entries[i];
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  template<typename Service, typename Query, typename Subscriptions,
    typename RealTimeSubscriptions>
  void MarketDataRelayServlet<C, M, A>::handle_query_request(
      Beam::RequestToken<ServiceProtocolClient, Service>& request,
      const Query& query, Subscriptions& subscriptions,
      RealTimeSubscriptions& real_time_subscriptions) {
    using Result = typename Service::Return;
    using MarketDataType = typename Result::Type;
    auto& session = request.get_session();
    auto result = Result();
    if(!has_entitlement<typename MarketDataType::Value>(session, query)) {
      request.set(result);
      return;
    }
    if constexpr(std::is_same_v<typename Query::Index, Security>) {
      if(!query.get_index().get_venue()) {
        request.set(result);
        return;
      }
      if(!m_securities.try_load(query.get_index())) {
        auto client = m_market_data_clients.load();
        auto info = load_security_info(*client, query.get_index());
        if(info) {
          m_securities.update(info->m_security);
        }
        if(!info ||
            info->m_security.get_venue() != query.get_index().get_venue()) {
          request.set(result);
          return;
        }
      }
    }
    if(query.get_range().get_end() == Beam::Sequence::LAST) {
      auto filter =
        Beam::translate<EvaluatorTranslator>(query.get_filter());
      result.m_id = subscriptions.init(query.get_index(), request.get_client(),
        Beam::Range::TOTAL, std::move(filter));
      real_time_subscriptions.test_and_set(query.get_index(), [&] {
        auto& query_entry = get_real_time_query_entry(query.get_index());
        auto initial_value_queue =
          std::make_shared<Beam::Queue<MarketDataType>>();
        query_entry.m_market_data_client->query(
          Beam::make_latest_query(query.get_index()), initial_value_queue);
        auto initial_values = std::vector<MarketDataType>();
        Beam::flush(initial_value_queue, std::back_inserter(initial_values));
        auto initial_sequence = [&] {
          if(initial_values.empty()) {
            return Beam::Sequence::FIRST;
          } else {
            return Beam::increment(initial_values.back().get_sequence());
          }
        }();
        auto real_time_query = Query();
        real_time_query.set_index(query.get_index());
        real_time_query.set_interruption_policy(
          Beam::InterruptionPolicy::RECOVER_DATA);
        real_time_query.set_range(initial_sequence, Beam::Sequence::LAST);
        query_entry.m_market_data_client->query(real_time_query,
          query_entry.m_tasks.template get_slot<MarketDataType>(
            [=, this, &subscriptions] (const auto& value) {
              on_real_time_update(query.get_index(), value, subscriptions);
            }));
      });
      auto queue = std::make_shared<Beam::Queue<MarketDataType>>();
      auto client = m_market_data_clients.load();
      auto snapshot_query = query;
      snapshot_query.set_range(
        query.get_range().get_start(), Beam::Sequence::PRESENT);
      client->query(snapshot_query, queue);
      Beam::flush(queue, std::back_inserter(result.m_snapshot));
      subscriptions.commit(query.get_index(), std::move(result),
        [&] (auto&& result) {
          request.set(std::forward<decltype(result)>(result));
        });
    } else {
      auto queue = std::make_shared<Beam::Queue<MarketDataType>>();
      auto client = m_market_data_clients.load();
      client->query(query, queue);
      Beam::flush(queue, std::back_inserter(result.m_snapshot));
      request.set(result);
    }
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  template<typename Subscriptions>
  void MarketDataRelayServlet<C, M, A>::on_end_query(
      ServiceProtocolClient& client, const typename Subscriptions::Index& index,
      int id, Subscriptions& subscriptions) {
    subscriptions.end(index, id);
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  SecuritySnapshot MarketDataRelayServlet<C, M, A>::on_load_security_snapshot(
      ServiceProtocolClient& client, const Security& security) {
    auto& session = client.get_session();
    auto market_data_client = m_market_data_clients.load();
    auto snapshot = market_data_client->load_snapshot(security);
    if(!has_entitlement(
        session, security.get_venue(), MarketDataType::BBO_QUOTE)) {
      snapshot.m_bbo_quote = SequencedBboQuote();
    }
    if(!has_entitlement(
        session, security.get_venue(), MarketDataType::TIME_AND_SALE)) {
      snapshot.m_time_and_sale = SequencedTimeAndSale();
    }
    auto ask_end_range = std::remove_if(
      snapshot.m_asks.begin(), snapshot.m_asks.end(), [&] (auto& quote) {
        return !has_entitlement(
          session, EntitlementKey(security.get_venue(), quote->m_venue),
          MarketDataType::BOOK_QUOTE);
      });
    snapshot.m_asks.erase(ask_end_range, snapshot.m_asks.end());
    auto bid_end_range = std::remove_if(
      snapshot.m_bids.begin(), snapshot.m_bids.end(), [&] (auto& quote) {
        return !has_entitlement(
          session, EntitlementKey(security.get_venue(), quote->m_venue),
          MarketDataType::BOOK_QUOTE);
      });
    snapshot.m_bids.erase(bid_end_range, snapshot.m_bids.end());
    return snapshot;
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  SecurityTechnicals MarketDataRelayServlet<C, M, A>::
      on_load_security_technicals(
        ServiceProtocolClient& client, const Security& security) {
    auto market_data_client = m_market_data_clients.load();
    return market_data_client->load_technicals(security);
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  std::vector<SecurityInfo> MarketDataRelayServlet<C, M, A>::
      on_query_security_info(
        ServiceProtocolClient& client, const SecurityInfoQuery& query) {
    auto market_data_client = m_market_data_clients.load();
    return market_data_client->query(query);
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  std::vector<SecurityInfo> MarketDataRelayServlet<C, M, A>::
      on_load_security_info_from_prefix(
        ServiceProtocolClient& client, const std::string& prefix) {
    auto market_data_client = m_market_data_clients.load();
    return market_data_client->load_security_info_from_prefix(prefix);
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  template<typename Index, typename Value, typename Subscriptions>
  std::enable_if_t<!std::is_same_v<Value, SequencedBookQuote>>
      MarketDataRelayServlet<C, M, A>::on_real_time_update(
        const Index& index, const Value& value, Subscriptions& subscriptions) {
    auto indexed_value = Beam::SequencedValue(
      Beam::IndexedValue(*value, index), value.get_sequence());
    subscriptions.publish(indexed_value, [&] (auto& clients) {
      Beam::broadcast_record_message<
        market_data_message_type_t<typename Value::Value>>(
          clients, indexed_value);
    });
  }

  template<typename C, typename M, typename A> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  template<typename Index, typename Value, typename Subscriptions>
  std::enable_if_t<std::is_same_v<Value, SequencedBookQuote>>
      MarketDataRelayServlet<C, M, A>::on_real_time_update(
        const Index& index, const Value& value, Subscriptions& subscriptions) {
    auto key = EntitlementKey(index.get_venue(), value->m_venue);
    auto indexed_value = Beam::SequencedValue(
      Beam::IndexedValue(*value, index), value.get_sequence());
    subscriptions.publish(indexed_value, [&] (auto& client) {
      return has_entitlement(
        client.get_session(), key, MarketDataType::BOOK_QUOTE);
    },
    [&] (auto& clients) {
      Beam::broadcast_record_message<
        market_data_message_type_t<typename Value::Value>>(
          clients, indexed_value);
    });
  }
}

#endif
