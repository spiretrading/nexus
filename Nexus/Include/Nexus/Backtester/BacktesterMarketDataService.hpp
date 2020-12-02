#ifndef NEXUS_BACKTESTER_MARKET_DATA_SERVICE_HPP
#define NEXUS_BACKTESTER_MARKET_DATA_SERVICE_HPP
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/Sequence.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Utilities/HashTuple.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Nexus/MarketDataService/MarketDataClientUtilities.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"

namespace Nexus {

  /** Provides historical market data to the backtester. */
  class BacktesterMarketDataService {
    public:

      /**
       * Constructs a BacktesterMarketDataService.
       * @param eventHandler The BacktesterEventHandler to push historical
       *        market data events to.
       * @param marketDataEnvironment The object to publish market data updates
       *        to.
       * @param marketDataClient The MarketDataClient used to retrieve
       *        historicalMarketData.
       */
      BacktesterMarketDataService(
        Beam::Ref<BacktesterEventHandler> eventHandler,
        Beam::Ref<MarketDataService::Tests::MarketDataServiceTestEnvironment>
        marketDataEnvironment,
        MarketDataService::MarketDataClientBox marketDataClient);

      /**
       * Submits a query for OrderImbalances.
       * @param query The query to submit.
       */
      void QueryOrderImbalances(
        const MarketDataService::MarketWideDataQuery& query);

      /**
       * Submits a query for BboQuotes.
       * @param query The query to submit.
       */
      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);

      /**
       * Submits a query for BookQuotes.
       * @param query The query to submit.
       */
      void QueryBookQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);

      /**
       * Submits a query for MarketQuotes.
       * @param query The query to submit.
       */
      void QueryMarketQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);

      /**
       * Submits a query for TimeAndSales.
       * @param query The query to submit.
       */
      void QueryTimeAndSales(
        const MarketDataService::SecurityMarketDataQuery& query);

    private:
      template<typename, typename> friend class MarketDataEvent;
      template<typename> friend class MarketDataLoadEvent;
      template<typename> friend class MarketDataQueryEvent;
      BacktesterEventHandler* m_eventHandler;
      MarketDataService::Tests::MarketDataServiceTestEnvironment*
        m_marketDataEnvironment;
      MarketDataService::MarketDataClientBox m_marketDataClient;
      std::unordered_set<std::tuple<boost::variant<Security, MarketCode>,
        MarketDataService::MarketDataType>> m_queries;

      BacktesterMarketDataService(const BacktesterMarketDataService&) = delete;
      BacktesterMarketDataService& operator =(
        const BacktesterMarketDataService&) = delete;
  };

  template<typename T>
  class MarketDataQueryEvent : public BacktesterEvent {
    public:
      using MarketDataType = T;

      using Query = MarketDataService::GetMarketDataQueryType<
        Beam::Queries::SequencedValue<MarketDataType>>;

      MarketDataQueryEvent(Query query,
        Beam::Ref<BacktesterMarketDataService> service);

      void Execute() override;

    private:
      Query m_query;
      BacktesterMarketDataService* m_service;
  };

  template<typename T>
  class MarketDataLoadEvent : public BacktesterEvent {
    public:
      using MarketDataType = T;

      using Query = MarketDataService::GetMarketDataQueryType<
        Beam::Queries::SequencedValue<MarketDataType>>;

      MarketDataLoadEvent(typename Query::Index index,
        Beam::Queries::Range::Point startPoint,
        boost::posix_time::ptime timestamp,
        Beam::Ref<BacktesterMarketDataService> service);

      void Execute() override;

    private:
      typename Query::Index m_index;
      Beam::Queries::Range::Point m_startPoint;
      BacktesterMarketDataService* m_service;
  };

  template<typename I, typename T>
  class MarketDataEvent : public BacktesterEvent {
    public:
      using Index = I;

      using MarketDataType = T;

      MarketDataEvent(Index index, MarketDataType value,
        boost::posix_time::ptime timestamp,
        Beam::Ref<BacktesterMarketDataService> service);

      void Execute() override;

    private:
      Index m_index;
      MarketDataType m_value;
      BacktesterMarketDataService* m_service;
  };

  inline BacktesterMarketDataService::BacktesterMarketDataService(
    Beam::Ref<BacktesterEventHandler> eventHandler,
    Beam::Ref<MarketDataService::Tests::MarketDataServiceTestEnvironment>
      marketDataEnvironment,
    MarketDataService::MarketDataClientBox marketDataClient)
    : m_eventHandler(eventHandler.Get()),
      m_marketDataEnvironment(marketDataEnvironment.Get()),
      m_marketDataClient(std::move(marketDataClient)) {}

  inline void BacktesterMarketDataService::QueryOrderImbalances(
      const MarketDataService::MarketWideDataQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<OrderImbalance>>(query,
      Beam::Ref(*this));
    m_eventHandler->Add(event);
  }

  inline void BacktesterMarketDataService::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<BboQuote>>(query,
      Beam::Ref(*this));
    m_eventHandler->Add(event);
  }

  inline void BacktesterMarketDataService::QueryBookQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<BookQuote>>(query,
      Beam::Ref(*this));
    m_eventHandler->Add(event);
  }

  inline void BacktesterMarketDataService::QueryMarketQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<MarketQuote>>(query,
      Beam::Ref(*this));
    m_eventHandler->Add(event);
  }

  inline void BacktesterMarketDataService::QueryTimeAndSales(
      const MarketDataService::SecurityMarketDataQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<TimeAndSale>>(query,
      Beam::Ref(*this));
    m_eventHandler->Add(event);
  }

  template<typename T>
  MarketDataQueryEvent<T>::MarketDataQueryEvent(Query query,
    Beam::Ref<BacktesterMarketDataService> service)
    : BacktesterEvent(boost::posix_time::neg_infin),
      m_query(std::move(query)),
      m_service(service.Get()) {}

  template<typename T>
  void MarketDataQueryEvent<T>::Execute() {
    if(m_query.GetRange().GetEnd() != Beam::Queries::Sequence::Last()) {
      return;
    }
    if(!m_service->m_queries.insert(std::make_tuple(m_query.GetIndex(),
        MarketDataService::GetMarketDataType<MarketDataType>())).second) {
      return;
    }
    auto startTime = m_service->m_eventHandler->GetTime();
    auto event = std::make_shared<MarketDataLoadEvent<MarketDataType>>(
      m_query.GetIndex(), startTime, boost::posix_time::neg_infin,
      Beam::Ref(*m_service));
    m_service->m_eventHandler->Add(std::move(event));
  }

  template<typename T>
  MarketDataLoadEvent<T>::MarketDataLoadEvent(typename Query::Index index,
    Beam::Queries::Range::Point startPoint, boost::posix_time::ptime timestamp,
    Beam::Ref<BacktesterMarketDataService> service)
    : BacktesterEvent(timestamp),
      m_index(std::move(index)),
      m_startPoint(startPoint),
      m_service(service.Get()) {}

  template<typename T>
  void MarketDataLoadEvent<T>::Execute() {
    const auto QUERY_SIZE = 1000;
    auto endPoint =
      [&] () -> Beam::Queries::Range::Point {
        if(m_service->m_eventHandler->GetEndTime() ==
            boost::posix_time::pos_infin) {
          return Beam::Queries::Sequence::Present();
        }
        return m_service->m_eventHandler->GetEndTime();
      }();
    auto query = Query();
    query.SetIndex(m_index);
    query.SetRange(m_startPoint, endPoint);
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::HEAD,
      QUERY_SIZE);
    auto queue = std::make_shared<Beam::Queue<
      Beam::Queries::SequencedValue<MarketDataType>>>();
    MarketDataService::QueryMarketDataClient(m_service->m_marketDataClient,
      query, Beam::ScopedQueueWriter(queue));
    auto data = std::vector<Beam::Queries::SequencedValue<MarketDataType>>();
    Beam::Flush(queue, std::back_inserter(data));
    if(data.empty()) {
      return;
    }
    auto events = std::vector<std::shared_ptr<BacktesterEvent>>();
    auto timestamp = m_service->m_eventHandler->GetTime();
    for(auto& value : data) {
      timestamp = std::max(timestamp,
        Beam::Queries::GetTimestamp(value.GetValue()));
      events.push_back(std::make_shared<
        MarketDataEvent<typename Query::Index, MarketDataType>>(
          query.GetIndex(), std::move(value), timestamp,
          Beam::Ref(*m_service)));
    }
    auto reloadEvent = std::make_shared<MarketDataLoadEvent>(m_index,
      Beam::Queries::Increment(data.back().GetSequence()),
      events.back()->GetTimestamp(), Beam::Ref(*m_service));
    events.push_back(std::move(reloadEvent));
    m_service->m_eventHandler->Add(std::move(events));
  }

  template<typename I, typename T>
  MarketDataEvent<I, T>::MarketDataEvent(Index index, MarketDataType value,
    boost::posix_time::ptime timestamp,
    Beam::Ref<BacktesterMarketDataService> service)
    : BacktesterEvent(timestamp),
      m_index(std::move(index)),
      m_value(std::move(value)),
      m_service(service.Get()) {}

  template<typename I, typename T>
  void MarketDataEvent<I, T>::Execute() {
    m_service->m_marketDataEnvironment->Publish(m_index, m_value);
  }
}

#endif
