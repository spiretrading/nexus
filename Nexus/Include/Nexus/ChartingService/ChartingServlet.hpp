#ifndef NEXUS_CHARTING_SERVLET_HPP
#define NEXUS_CHARTING_SERVLET_HPP
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/ConversionEvaluatorNode.hpp>
#include <Beam/Queries/IndexedExpressionSubscriptions.hpp>
#include <Beam/Queries/ExpressionSubscriptions.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/Casts.hpp>
#include <Beam/Utilities/InstantiateTemplate.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/ChartingService/ChartingServices.hpp"
#include "Nexus/MarketDataService/CachedHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/ClientHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/QueryTypes.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus::ChartingService {
namespace Details {
  struct ExpressionConverter {
    template<typename T>
    static std::unique_ptr<Beam::Queries::BaseEvaluatorNode> Template(
        std::unique_ptr<Beam::Queries::BaseEvaluatorNode> base_expression) {
      auto expression = Beam::StaticCast<std::unique_ptr<
        Beam::Queries::EvaluatorNode<T>>>(std::move(base_expression));
      return Beam::Queries::MakeConstructEvaluatorNode<
        T, Queries::QueryVariant>(std::move(expression));
    }

    using SupportedTypes = Queries::QueryTypes::NativeTypes;
  };
}

  /**
   * Provides historical and charting related data.
   * @param <C> The container instantiating this servlet.
   * @param <M> The type of MarketDataClient used to access real-time data.
   */
  template<typename C, MarketDataService::IsMarketDataClient M>
  class ChartingServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of MarketDataClient used. */
      using MarketDataClient = Beam::GetTryDereferenceType<M>;

      /**
       * Constructs a ChartingServlet.
       * @param market_data_client Initializes the MarketDataClient.
       */
      template<Beam::Initializes<M> MF>
      explicit ChartingServlet(MF&& market_data_client);

      void RegisterServices(
        Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      template<typename MarketDataType>
      struct QueryEntry {
        using Query =
          MarketDataService::market_data_query_type_t<MarketDataType>;
        Beam::Queries::IndexedExpressionSubscriptions<
          typename MarketDataType::Value, Queries::QueryVariant,
          typename Query::Index, ServiceProtocolClient> m_queries;
        Beam::SynchronizedUnorderedSet<Security, Beam::Threading::Mutex>
          m_real_time_subscriptions;
      };
      Beam::GetOptionalLocalPtr<M> m_market_data_client;
      MarketDataService::CachedHistoricalDataStore<
        MarketDataService::ClientHistoricalDataStore<MarketDataClient*>>
          m_data_store;
      QueryEntry<SequencedTimeAndSale> m_time_and_sale_queries;
      Beam::IO::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      void on_query_security_request(Beam::Services::RequestToken<
        ServiceProtocolClient, QuerySecurityService>& request,
        const SecurityChartingQuery& query, int client_query_id);
      void on_end_security_query(ServiceProtocolClient& client, int id);
      TimePriceQueryResult on_load_security_time_price_series_request(
        ServiceProtocolClient& client, const Security& security,
        boost::posix_time::ptime start_time, boost::posix_time::ptime end_time,
        boost::posix_time::time_duration interval);
      template<typename MarketDataType>
      void handle_query(Beam::Services::RequestToken<
        ServiceProtocolClient, QuerySecurityService>& request,
        const SecurityChartingQuery& query, int client_query_id,
        QueryEntry<MarketDataType>& query_entry);
      template<typename Index, typename MarketDataType>
      void on_query_update(const Index& index, const MarketDataType& value,
        QueryEntry<MarketDataType>& query_entry);
  };

  template<typename M>
  struct MetaChartingServlet {
    using Session = Beam::NullType;
    static constexpr auto SupportsParallelism = true;

    template<typename C>
    struct apply {
      using type = ChartingServlet<C, M>;
    };
  };

  template<typename C, MarketDataService::IsMarketDataClient M>
  template<Beam::Initializes<M> MF>
  ChartingServlet<C, M>::ChartingServlet(MF&& market_data_client)
    : m_market_data_client(std::forward<MF>(market_data_client)),
      m_data_store(Beam::Initialize(&*m_market_data_client), 10000) {}

  template<typename C, MarketDataService::IsMarketDataClient M>
  void ChartingServlet<C, M>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    Queries::RegisterQueryTypes(Beam::Store(slots->GetRegistry()));
    RegisterChartingServices(Store(slots));
    RegisterChartingMessages(Store(slots));
    QuerySecurityService::AddRequestSlot(Store(slots),
      std::bind_front(&ChartingServlet::on_query_security_request, this));
    Beam::Services::AddMessageSlot<EndSecurityQueryMessage>(Store(slots),
      std::bind_front(&ChartingServlet::on_end_security_query, this));
    LoadSecurityTimePriceSeriesService::AddSlot(Store(slots), std::bind_front(
      &ChartingServlet::on_load_security_time_price_series_request, this));
  }

  template<typename C, MarketDataService::IsMarketDataClient M>
  void ChartingServlet<C, M>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_time_and_sale_queries.m_queries.RemoveAll(client);
  }

  template<typename C, MarketDataService::IsMarketDataClient M>
  void ChartingServlet<C, M>::Close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_tasks.Break();
    m_tasks.Wait();
    m_market_data_client->close();
    m_open_state.Close();
  }

  template<typename C, MarketDataService::IsMarketDataClient M>
  void ChartingServlet<C, M>::on_query_security_request(
      Beam::Services::RequestToken<ServiceProtocolClient, QuerySecurityService>&
      request, const SecurityChartingQuery& query, int client_query_id) {
    auto& session = request.GetSession();
    if(query.get_market_data_type() ==
        MarketDataService::MarketDataType::TIME_AND_SALE) {
      handle_query(request, query, client_query_id, m_time_and_sale_queries);
    } else {
      request.SetResult(SecurityChartingQueryResult());
    }
  }

  template<typename C, MarketDataService::IsMarketDataClient M>
  void ChartingServlet<C, M>::on_end_security_query(
      ServiceProtocolClient& client, int id) {
    auto& session = client.GetSession();
    m_time_and_sale_queries.m_queries.End(client, id);
  }

  template<typename C, MarketDataService::IsMarketDataClient M>
  TimePriceQueryResult ChartingServlet<C, M>::
      on_load_security_time_price_series_request(ServiceProtocolClient& client,
      const Security& security, boost::posix_time::ptime start_time,
      boost::posix_time::ptime end_time,
      boost::posix_time::time_duration interval) {
    if(end_time < start_time + interval ||
        start_time == boost::posix_time::neg_infin  ||
        end_time == boost::posix_time::pos_infin) {
      throw Beam::Services::ServiceRequestException("Invalid time range.");
    }
    auto queue = std::make_shared<Beam::Queue<SequencedTimeAndSale>>();
    auto time_and_sale_query = MarketDataService::SecurityMarketDataQuery();
    time_and_sale_query.SetIndex(security);
    time_and_sale_query.SetRange(start_time, end_time);
    time_and_sale_query.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Unlimited());
    m_market_data_client->query(time_and_sale_query, queue);
    auto time_and_sales = std::vector<SequencedTimeAndSale>();
    Beam::Flush(queue, std::back_inserter(time_and_sales));
    auto result = TimePriceQueryResult();
    if(!time_and_sales.empty()) {
      result.start = time_and_sales.front().GetSequence();
      result.end = time_and_sales.back().GetSequence();
    }
    auto current_start = start_time;
    auto current_end = start_time + interval;
    auto time_and_sales_iterator = time_and_sales.begin();
    while(time_and_sales_iterator != time_and_sales.end() &&
        current_start <= end_time) {
      auto candlestick =
        TechnicalAnalysis::TimePriceCandlestick(current_start, current_end);
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

  template<typename C, MarketDataService::IsMarketDataClient M>
  template<typename MarketDataType>
  void ChartingServlet<C, M>::handle_query(
      Beam::Services::RequestToken<ServiceProtocolClient, QuerySecurityService>&
        request, const SecurityChartingQuery& query, int client_query_id,
      QueryEntry<MarketDataType>& query_entry) {
    using Query = MarketDataService::market_data_query_type_t<MarketDataType>;
    if(query.GetRange().GetEnd() == Beam::Queries::Sequence::Last()) {
      query_entry.m_real_time_subscriptions.TestAndSet(query.GetIndex(), [&] {
        auto real_time_query = Query();
        real_time_query.SetIndex(query.GetIndex());
        real_time_query.SetRange(Beam::Queries::Range::RealTime());
        m_market_data_client->query(
          real_time_query, m_tasks.GetSlot<MarketDataType>(
            [=, this, &query_entry] (const auto& value) {
              on_query_update(query.GetIndex(), value, query_entry);
            }));
      });
    }
    auto result = SecurityChartingQueryResult();
    result.m_queryId = client_query_id;
    auto filter =
      Beam::Queries::Translate<Queries::EvaluatorTranslator>(query.GetFilter());
    auto translator = Queries::EvaluatorTranslator();
    translator.Translate(query.GetExpression());
    auto base_expression = std::move(translator.GetEvaluator());
    auto expression = Beam::Instantiate<Details::ExpressionConverter>(
      base_expression->GetResultType())(std::move(base_expression));
    auto evaluator = std::make_unique<Beam::Queries::Evaluator>(
      std::move(expression), translator.GetParameters());
    query_entry.m_queries.Initialize(query.GetIndex(), request.GetClient(),
      client_query_id, query.GetRange(), std::move(filter),
      query.GetUpdatePolicy(), std::move(evaluator));
    auto snapshot_query = query;
    snapshot_query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    auto snapshot =
      MarketDataService::load<MarketDataType>(m_data_store, snapshot_query);
    query_entry.m_queries.Commit(query.GetIndex(), request.GetClient(),
      query.GetSnapshotLimit(), std::move(result), std::move(snapshot),
      [&] (auto&& result) {
        request.SetResult(std::forward<decltype(result)>(result));
      });
  }

  template<typename C, MarketDataService::IsMarketDataClient M>
  template<typename Index, typename MarketDataType>
  void ChartingServlet<C, M>::on_query_update(const Index& index,
      const MarketDataType& value, QueryEntry<MarketDataType>& queries) {
    auto indexed_value = Beam::Queries::SequencedValue(
      Beam::Queries::IndexedValue(*value, index), value.GetSequence());
    m_data_store.store(indexed_value);
    queries.m_queries.Publish(indexed_value,
      [&] (auto& client, auto id, const auto& value) {
        Beam::Services::SendRecordMessage<SecurityQueryMessage>(
          client, id, value);
      });
  }
}

#endif
