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
#include <boost/noncopyable.hpp>
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/ChartingService/ChartingServices.hpp"
#include "Nexus/MarketDataService/CachedHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/ClientHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/HistoricalDataStoreUtilities.hpp"
#include "Nexus/MarketDataService/MarketDataClientUtilities.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus::ChartingService {
namespace Details {
  struct ExpressionConverter {
    template<typename T>
    static std::unique_ptr<Beam::Queries::BaseEvaluatorNode> Template(
        std::unique_ptr<Beam::Queries::BaseEvaluatorNode> baseExpression) {
      auto expression = Beam::StaticCast<std::unique_ptr<
        Beam::Queries::EvaluatorNode<T>>>(std::move(baseExpression));
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
  template<typename C, typename M>
  class ChartingServlet : private boost::noncopyable {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of MarketDataClient used. */
      using MarketDataClient = Beam::GetTryDereferenceType<M>;

      /**
       * Constructs a ChartingServlet.
       * @param marketDataClient Initializes the MarketDataClient.
       */
      template<typename MF>
      explicit ChartingServlet(MF&& marketDataClient);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      template<typename MarketDataType>
      struct QueryEntry {
        using Query = MarketDataService::GetMarketDataQueryType<
          MarketDataType>;
        Beam::Queries::IndexedExpressionSubscriptions<
          typename MarketDataType::Value, Queries::QueryVariant,
          typename Query::Index, ServiceProtocolClient> m_queries;
        Beam::SynchronizedUnorderedSet<Security, Beam::Threading::Mutex>
          m_realTimeSubscriptions;
      };
      Beam::GetOptionalLocalPtr<M> m_marketDataClient;
      MarketDataService::CachedHistoricalDataStore<
        MarketDataService::ClientHistoricalDataStore<MarketDataClient*>>
          m_dataStore;
      QueryEntry<SequencedTimeAndSale> m_timeAndSaleQueries;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void OnQuerySecurityRequest(Beam::Services::RequestToken<
        ServiceProtocolClient, QuerySecurityService>& request,
        const SecurityChartingQuery& query, int clientQueryId);
      void OnEndSecurityQuery(ServiceProtocolClient& client, int id);
      TimePriceQueryResult OnLoadSecurityTimePriceSeriesRequest(
        ServiceProtocolClient& client, const Security& security,
        boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
        boost::posix_time::time_duration interval);
      template<typename MarketDataType>
      void HandleQuery(Beam::Services::RequestToken<
        ServiceProtocolClient, QuerySecurityService>& request,
        const SecurityChartingQuery& query, int clientQueryId,
        QueryEntry<MarketDataType>& queryEntry);
      template<typename Index, typename MarketDataType>
      void OnQueryUpdate(const Index& index, const MarketDataType& value,
        QueryEntry<MarketDataType>& queryEntry);
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

  template<typename C, typename M>
  template<typename MF>
  ChartingServlet<C, M>::ChartingServlet(MF&& marketDataClient)
    : m_marketDataClient(std::forward<MF>(marketDataClient)),
      m_dataStore(Beam::Initialize(&*m_marketDataClient), 10000) {}

  template<typename C, typename M>
  void ChartingServlet<C, M>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    Queries::RegisterQueryTypes(Beam::Store(slots->GetRegistry()));
    RegisterChartingServices(Store(slots));
    RegisterChartingMessages(Store(slots));
    QuerySecurityService::AddRequestSlot(Store(slots),
      std::bind_front(&ChartingServlet::OnQuerySecurityRequest, this));
    Beam::Services::AddMessageSlot<EndSecurityQueryMessage>(Store(slots),
      std::bind_front(&ChartingServlet::OnEndSecurityQuery, this));
    LoadSecurityTimePriceSeriesService::AddSlot(Store(slots), std::bind_front(
      &ChartingServlet::OnLoadSecurityTimePriceSeriesRequest, this));
  }

  template<typename C, typename M>
  void ChartingServlet<C, M>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_timeAndSaleQueries.m_queries.RemoveAll(client);
  }

  template<typename C, typename M>
  void ChartingServlet<C, M>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_tasks.Break();
    m_tasks.Wait();
    m_marketDataClient->Close();
    m_openState.Close();
  }

  template<typename C, typename M>
  void ChartingServlet<C, M>::OnQuerySecurityRequest(
      Beam::Services::RequestToken<ServiceProtocolClient, QuerySecurityService>&
      request, const SecurityChartingQuery& query, int clientQueryId) {
    auto& session = request.GetSession();
    if(query.GetMarketDataType() ==
        MarketDataService::MarketDataType::TIME_AND_SALE) {
      HandleQuery(request, query, clientQueryId, m_timeAndSaleQueries);
    } else {
      request.SetResult(SecurityChartingQueryResult());
    }
  }

  template<typename C, typename M>
  void ChartingServlet<C, M>::OnEndSecurityQuery(ServiceProtocolClient& client,
      int id) {
    auto& session = client.GetSession();
    m_timeAndSaleQueries.m_queries.End(client, id);
  }

  template<typename C, typename M>
  TimePriceQueryResult ChartingServlet<C, M>::
      OnLoadSecurityTimePriceSeriesRequest(ServiceProtocolClient& client,
      const Security& security, boost::posix_time::ptime startTime,
      boost::posix_time::ptime endTime,
      boost::posix_time::time_duration interval) {
    if(endTime < startTime + interval ||
        startTime == boost::posix_time::neg_infin  ||
        endTime == boost::posix_time::pos_infin) {
      throw Beam::Services::ServiceRequestException("Invalid time range.");
    }
    auto queue = std::make_shared<Beam::Queue<SequencedTimeAndSale>>();
    auto timeAndSaleQuery = MarketDataService::SecurityMarketDataQuery();
    timeAndSaleQuery.SetIndex(security);
    timeAndSaleQuery.SetRange(startTime, endTime);
    timeAndSaleQuery.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Unlimited());
    m_marketDataClient->QueryTimeAndSales(timeAndSaleQuery, queue);
    auto timeAndSales = std::vector<SequencedTimeAndSale>();
    Beam::Flush(queue, std::back_inserter(timeAndSales));
    auto result = TimePriceQueryResult();
    if(!timeAndSales.empty()) {
      result.start = timeAndSales.front().GetSequence();
      result.end = timeAndSales.back().GetSequence();
    }
    auto currentStart = startTime;
    auto currentEnd = startTime + interval;
    auto timeAndSalesIterator = timeAndSales.begin();
    while(timeAndSalesIterator != timeAndSales.end() &&
        currentStart <= endTime) {
      auto candlestick = TechnicalAnalysis::TimePriceCandlestick(
        currentStart, currentEnd);
      auto hasPoint = false;
      while(timeAndSalesIterator != timeAndSales.end() &&
          (*timeAndSalesIterator)->m_timestamp < currentEnd) {
        candlestick.Update((*timeAndSalesIterator)->m_price);
        hasPoint = true;
        ++timeAndSalesIterator;
      }
      if(hasPoint) {
        result.series.push_back(candlestick);
      }
      currentStart = currentEnd;
      currentEnd = currentStart + interval;
    }
    return result;
  }

  template<typename C, typename M>
  template<typename MarketDataType>
  void ChartingServlet<C, M>::HandleQuery(
      Beam::Services::RequestToken<ServiceProtocolClient,
        QuerySecurityService>& request, const SecurityChartingQuery& query,
      int clientQueryId, QueryEntry<MarketDataType>& queryEntry) {
    using Query = MarketDataService::GetMarketDataQueryType<MarketDataType>;
    if(query.GetRange().GetEnd() == Beam::Queries::Sequence::Last()) {
      queryEntry.m_realTimeSubscriptions.TestAndSet(query.GetIndex(), [&] {
        auto realTimeQuery = Query();
        realTimeQuery.SetIndex(query.GetIndex());
        realTimeQuery.SetRange(Beam::Queries::Range::RealTime());
        MarketDataService::QueryMarketDataClient(*m_marketDataClient,
          realTimeQuery, m_tasks.GetSlot<MarketDataType>(
            [=, &queryEntry] (auto&& value) {
              OnQueryUpdate<typename Query::Index, MarketDataType>(
                query.GetIndex(), std::forward<decltype(value)>(value),
                queryEntry);
            }));
      });
    }
    auto result = SecurityChartingQueryResult();
    result.m_queryId = clientQueryId;
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    auto translator = Queries::EvaluatorTranslator();
    translator.Translate(query.GetExpression());
    auto baseExpression = std::move(translator.GetEvaluator());
    auto expression = Beam::Instantiate<Details::ExpressionConverter>(
      baseExpression->GetResultType())(std::move(baseExpression));
    auto expressionEvaluator = std::make_unique<Beam::Queries::Evaluator>(
      std::move(expression), translator.GetParameters());
    queryEntry.m_queries.Initialize(query.GetIndex(), request.GetClient(),
      clientQueryId, query.GetRange(), std::move(filter),
      query.GetUpdatePolicy(), std::move(expressionEvaluator));
    auto snapshotQuery = query;
    snapshotQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    auto snapshot = MarketDataService::HistoricalDataStoreLoad<MarketDataType>(
      m_dataStore, snapshotQuery);
    queryEntry.m_queries.Commit(query.GetIndex(), request.GetClient(),
      query.GetSnapshotLimit(), std::move(result), std::move(snapshot),
      [&] (auto&& result) {
        request.SetResult(std::forward<decltype(result)>(result));
      });
  }

  template<typename C, typename M>
  template<typename Index, typename MarketDataType>
  void ChartingServlet<C, M>::OnQueryUpdate(const Index& index,
      const MarketDataType& value, QueryEntry<MarketDataType>& queries) {
    auto indexedValue = Beam::Queries::SequencedValue(
      Beam::Queries::IndexedValue(*value, index), value.GetSequence());
    m_dataStore.Store(indexedValue);
    queries.m_queries.Publish(indexedValue,
      [&] (auto& client, auto id, auto& value) {
        Beam::Services::SendRecordMessage<SecurityQueryMessage>(client, id,
          value);
      });
  }
}

#endif
