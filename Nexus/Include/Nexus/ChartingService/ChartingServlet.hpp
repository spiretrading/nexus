#ifndef NEXUS_CHARTING_SERVLET_HPP
#define NEXUS_CHARTING_SERVLET_HPP
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/ConversionEvaluatorNode.hpp>
#include <Beam/Queries/IndexedExpressionSubscriptions.hpp>
#include <Beam/Queries/ExpressionSubscriptions.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/Casts.hpp>
#include <Beam/Utilities/Instantiate.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/ChartingService/ChartingServices.hpp"
#include "Nexus/MarketDataService/CachedHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/ClientHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/QueryTypes.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus {
namespace Details {
  struct ExpressionConverter {
    using type = QueryTypes::NativeTypes;

    template<typename T>
    std::unique_ptr<Beam::BaseEvaluatorNode> operator ()(
        std::unique_ptr<Beam::BaseEvaluatorNode> base_expression) const {
      auto expression = Beam::static_pointer_cast<Beam::EvaluatorNode<T>>(
        std::move(base_expression));
      return Beam::make_construct_evaluator_node<T, QueryVariant>(
        std::move(expression));
    }
  };
}

  /**
   * Provides historical and charting related data.
   * @param <C> The container instantiating this servlet.
   * @param <M> The type of MarketDataClient used to access real-time data.
   */
  template<typename C, typename M> requires
    IsMarketDataClient<Beam::dereference_t<M>>
  class ChartingServlet {
    public:

      /** The type of MarketDataClient used. */
      using MarketDataClient = Beam::dereference_t<M>;

      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * Constructs a ChartingServlet.
       * @param market_data_client Initializes the MarketDataClient.
       */
      template<Beam::Initializes<M> MF>
      explicit ChartingServlet(MF&& market_data_client);

      void register_services(
        Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots);
      void handle_close(ServiceProtocolClient& client);
      void close();

    private:
      template<typename MarketDataType>
      struct QueryEntry {
        using Query = market_data_query_type_t<MarketDataType>;
        Beam::IndexedExpressionSubscriptions<
          typename MarketDataType::Value, QueryVariant, typename Query::Index,
          ServiceProtocolClient> m_queries;
        Beam::SynchronizedUnorderedSet<Security, Beam::Mutex>
          m_real_time_subscriptions;
      };
      Beam::local_ptr_t<M> m_market_data_client;
      CachedHistoricalDataStore<ClientHistoricalDataStore<MarketDataClient*>>
        m_data_store;
      QueryEntry<SequencedTimeAndSale> m_time_and_sale_queries;
      Beam::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      void on_query_security_request(Beam::RequestToken<
        ServiceProtocolClient, QuerySecurityService>& request,
        const SecurityChartingQuery& query, int client_query_id);
      void on_end_security_query(ServiceProtocolClient& client, int id);
      TimePriceQueryResult on_load_security_time_price_series_request(
        ServiceProtocolClient& client, const Security& security,
        boost::posix_time::ptime start_time, boost::posix_time::ptime end_time,
        boost::posix_time::time_duration interval);
      template<typename MarketDataType>
      void handle_query(Beam::RequestToken<
          ServiceProtocolClient, QuerySecurityService>& request,
        const SecurityChartingQuery& query, int client_query_id,
        QueryEntry<MarketDataType>& query_entry);
      template<typename Index, typename MarketDataType>
      void on_query_update(const Index& index, const MarketDataType& value,
        QueryEntry<MarketDataType>& query_entry);
  };

  template<typename M>
  struct MetaChartingServlet {
    using Session = Beam::NullSession;
    static constexpr auto SUPPORTS_PARALLELISM = true;

    template<typename C>
    struct apply {
      using type = ChartingServlet<C, M>;
    };
  };

  template<typename C, typename M> requires
    IsMarketDataClient<Beam::dereference_t<M>>
  template<Beam::Initializes<M> MF>
  ChartingServlet<C, M>::ChartingServlet(MF&& market_data_client)
    : m_market_data_client(std::forward<MF>(market_data_client)),
      m_data_store(Beam::init(&*m_market_data_client), 10000) {}

  template<typename C, typename M> requires
    IsMarketDataClient<Beam::dereference_t<M>>
  void ChartingServlet<C, M>::register_services(
      Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots) {
    Nexus::register_query_types(Beam::out(slots->get_registry()));
    register_charting_services(out(slots));
    register_charting_messages(out(slots));
    QuerySecurityService::add_request_slot(out(slots),
      std::bind_front(&ChartingServlet::on_query_security_request, this));
    Beam::add_message_slot<EndSecurityQueryMessage>(out(slots),
      std::bind_front(&ChartingServlet::on_end_security_query, this));
    LoadSecurityTimePriceSeriesService::add_slot(out(slots), std::bind_front(
      &ChartingServlet::on_load_security_time_price_series_request, this));
  }

  template<typename C, typename M> requires
    IsMarketDataClient<Beam::dereference_t<M>>
  void ChartingServlet<C, M>::handle_close(ServiceProtocolClient& client) {
    m_time_and_sale_queries.m_queries.remove_all(client);
  }

  template<typename C, typename M> requires
    IsMarketDataClient<Beam::dereference_t<M>>
  void ChartingServlet<C, M>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_tasks.close();
    m_tasks.wait();
    m_market_data_client->close();
    m_open_state.close();
  }

  template<typename C, typename M> requires
    IsMarketDataClient<Beam::dereference_t<M>>
  void ChartingServlet<C, M>::on_query_security_request(
      Beam::RequestToken<ServiceProtocolClient, QuerySecurityService>&
        request, const SecurityChartingQuery& query, int client_query_id) {
    auto& session = request.get_session();
    if(query.get_market_data_type() == MarketDataType::TIME_AND_SALE) {
      handle_query(request, query, client_query_id, m_time_and_sale_queries);
    } else {
      request.set(SecurityChartingQueryResult());
    }
  }

  template<typename C, typename M> requires
    IsMarketDataClient<Beam::dereference_t<M>>
  void ChartingServlet<C, M>::on_end_security_query(
      ServiceProtocolClient& client, int id) {
    auto& session = client.get_session();
    m_time_and_sale_queries.m_queries.end(client, id);
  }

  template<typename C, typename M> requires
    IsMarketDataClient<Beam::dereference_t<M>>
  TimePriceQueryResult ChartingServlet<C, M>::
      on_load_security_time_price_series_request(ServiceProtocolClient& client,
        const Security& security, boost::posix_time::ptime start_time,
        boost::posix_time::ptime end_time,
        boost::posix_time::time_duration interval) {
    if(end_time < start_time + interval ||
        start_time == boost::posix_time::neg_infin  ||
        end_time == boost::posix_time::pos_infin) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Invalid time range."));
    }
    auto queue = std::make_shared<Beam::Queue<SequencedTimeAndSale>>();
    auto time_and_sale_query = SecurityMarketDataQuery();
    time_and_sale_query.set_index(security);
    time_and_sale_query.set_range(start_time, end_time);
    time_and_sale_query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
    m_market_data_client->query(time_and_sale_query, queue);
    auto time_and_sales = std::vector<SequencedTimeAndSale>();
    Beam::flush(queue, std::back_inserter(time_and_sales));
    auto result = TimePriceQueryResult();
    if(!time_and_sales.empty()) {
      result.start = time_and_sales.front().get_sequence();
      result.end = time_and_sales.back().get_sequence();
    }
    auto current_start = start_time;
    auto current_end = start_time + interval;
    auto time_and_sales_iterator = time_and_sales.begin();
    while(time_and_sales_iterator != time_and_sales.end() &&
        current_start <= end_time) {
      auto candlestick = TimePriceCandlestick(current_start, current_end);
      auto has_point = false;
      while(time_and_sales_iterator != time_and_sales.end() &&
          (*time_and_sales_iterator)->m_timestamp < current_end) {
        candlestick.update((*time_and_sales_iterator)->m_price);
        has_point = true;
        ++time_and_sales_iterator;
      }
      if(has_point) {
        result.series.push_back(candlestick);
      }
      current_start = current_end;
      current_end = current_start + interval;
    }
    return result;
  }

  template<typename C, typename M> requires
    IsMarketDataClient<Beam::dereference_t<M>>
  template<typename MarketDataType>
  void ChartingServlet<C, M>::handle_query(
      Beam::RequestToken<ServiceProtocolClient, QuerySecurityService>& request,
      const SecurityChartingQuery& query, int client_query_id,
      QueryEntry<MarketDataType>& query_entry) {
    using Query = market_data_query_type_t<MarketDataType>;
    if(query.get_range().get_end() == Beam::Sequence::LAST) {
      query_entry.m_real_time_subscriptions.test_and_set(query.get_index(),
        [&] {
          auto real_time_query = Query();
          real_time_query.set_index(query.get_index());
          real_time_query.set_range(Beam::Range::REAL_TIME);
          m_market_data_client->query(
            real_time_query, m_tasks.get_slot<MarketDataType>(
              [=, this, &query_entry] (const auto& value) {
                on_query_update(query.get_index(), value, query_entry);
              }));
        });
    }
    auto result = SecurityChartingQueryResult();
    result.m_id = client_query_id;
    auto filter = Beam::translate<EvaluatorTranslator>(query.get_filter());
    auto translator = EvaluatorTranslator();
    translator.translate(query.get_expression());
    auto base_expression = translator.get_evaluator();
    auto expression = Beam::instantiate<Details::ExpressionConverter>(
      base_expression->get_type())(std::move(base_expression));
    auto evaluator = std::make_unique<Beam::Evaluator>(
      std::move(expression), translator.get_parameters());
    query_entry.m_queries.init(query.get_index(), request.get_client(),
      client_query_id, query.get_range(), std::move(filter),
      query.get_update_policy(), std::move(evaluator));
    auto snapshot_query = query;
    snapshot_query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
    auto snapshot = load<MarketDataType>(m_data_store, snapshot_query);
    query_entry.m_queries.commit(query.get_index(), request.get_client(),
      query.get_snapshot_limit(), std::move(result), std::move(snapshot),
      [&] (auto&& result) {
        request.set(std::forward<decltype(result)>(result));
      });
  }

  template<typename C, typename M> requires
    IsMarketDataClient<Beam::dereference_t<M>>
  template<typename Index, typename MarketDataType>
  void ChartingServlet<C, M>::on_query_update(const Index& index,
      const MarketDataType& value, QueryEntry<MarketDataType>& queries) {
    auto indexed_value = Beam::SequencedValue(
      Beam::IndexedValue(*value, index), value.get_sequence());
    m_data_store.store(indexed_value);
    queries.m_queries.publish(indexed_value,
      [&] (auto& client, auto id, const auto& value) {
        Beam::send_record_message<SecurityQueryMessage>(client, id, value);
      });
  }
}

#endif
