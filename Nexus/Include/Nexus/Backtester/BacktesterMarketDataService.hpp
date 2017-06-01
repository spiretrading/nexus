#ifndef NEXUS_BACKTESTERMARKETDATASERVICE_HPP
#define NEXUS_BACKTESTERMARKETDATASERVICE_HPP
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/Sequence.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Utilities/HashTuple.hpp>
#include <boost/noncopyable.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/MarketDataService/MarketDataClientUtilities.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"

namespace Nexus {

  /*! \class BacktesterMarketDataService
      \brief Provides historical market data to the backtester.
   */
  class BacktesterMarketDataService : private boost::noncopyable {
    public:

      //! Constructs a BacktesterMarketDataService.
      /*!
        \param eventHandler The BacktesterEventHandler to push historical
               market data events to.
        \param marketDataClient The MarketDataClient used to retrieve
               historicalMarketData.
      */
      BacktesterMarketDataService(
        Beam::RefType<BacktesterEventHandler> eventHandler,
        Beam::RefType<MarketDataService::VirtualMarketDataClient>
        marketDataClient);

      //! Submits a query for BboQuotes.
      /*!
        \param query The query to submit.
      */
      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);

    private:
      template<typename, typename> friend class MarketDataEvent;
      template<typename> friend class MarketDataLoadEvent;
      template<typename> friend class MarketDataQueryEvent;
      BacktesterEventHandler* m_eventHandler;
      MarketDataService::VirtualMarketDataClient* m_marketDataClient;
      std::unordered_set<std::tuple<boost::variant<Security, MarketCode>,
        MarketDataService::MarketDataType>> m_queries;
  };

  template<typename MarketDataTypeType>
  class MarketDataQueryEvent : public BacktesterEvent {
    public:
      using MarketDataType = MarketDataTypeType;

      using Query = MarketDataService::GetMarketDataQueryType<
        Beam::Queries::SequencedValue<MarketDataType>>;

      MarketDataQueryEvent(Query query,
        Beam::RefType<BacktesterMarketDataService> service);

      virtual void Execute() override;

    private:
      Query m_query;
      BacktesterMarketDataService* m_service;
  };

  template<typename MarketDataTypeType>
  class MarketDataLoadEvent : public BacktesterEvent {
    public:
      using MarketDataType = MarketDataTypeType;

      using Query = MarketDataService::GetMarketDataQueryType<
        Beam::Queries::SequencedValue<MarketDataType>>;

      MarketDataLoadEvent(typename Query::Index index,
        Beam::Queries::Range::Point startPoint,
        Beam::RefType<BacktesterMarketDataService> service);

      virtual void Execute() override;

    private:
      typename Query::Index m_index;
      Beam::Queries::Range::Point m_startPoint;
      BacktesterMarketDataService* m_service;
  };

  template<typename IndexType, typename MarketDataTypeType>
  class MarketDataEvent : public BacktesterEvent {
    public:
      using Index = IndexType;

      using MarketDataType = MarketDataTypeType;

      MarketDataEvent(Index index, MarketDataType value,
        boost::posix_time::ptime timestamp,
        Beam::RefType<BacktesterMarketDataService> service);

      virtual void Execute() override;

    private:
      Index m_index;
      MarketDataType m_value;
      BacktesterMarketDataService* m_service;
  };

  inline BacktesterMarketDataService::BacktesterMarketDataService(
      Beam::RefType<BacktesterEventHandler> eventHandler,
      Beam::RefType<MarketDataService::VirtualMarketDataClient>
      marketDataClient)
      : m_eventHandler{eventHandler.Get()},
        m_marketDataClient{marketDataClient.Get()} {}

  inline void BacktesterMarketDataService::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<BboQuote>>(query,
      Beam::Ref(*this));
    m_eventHandler->Add(event);
  }

  template<typename MarketDataTypeType>
  MarketDataQueryEvent<MarketDataTypeType>::MarketDataQueryEvent(Query query,
      Beam::RefType<BacktesterMarketDataService> service)
      : BacktesterEvent{boost::posix_time::neg_infin},
        m_query{std::move(query)},
        m_service{service.Get()} {}

  template<typename MarketDataTypeType>
  void MarketDataQueryEvent<MarketDataTypeType>::Execute() {
    if(m_query.GetRange().GetEnd() != Beam::Queries::Sequence::Last()) {
      return;
    }
    if(!m_service->m_queries.insert(std::make_tuple(m_query.GetIndex(),
        MarketDataService::GetMarketDataType<MarketDataType>())).second) {
      return;
    }
    auto startTime = m_service->m_eventHandler->GetTime();
    auto event = std::make_shared<MarketDataLoadEvent<MarketDataType>>(
      m_query.GetIndex(), startTime, Beam::Ref(*m_service));
    m_service->m_eventHandler->Add(std::move(event));
  }

  template<typename MarketDataTypeType>
  MarketDataLoadEvent<MarketDataTypeType>::MarketDataLoadEvent(
      typename Query::Index index, Beam::Queries::Range::Point startPoint,
      Beam::RefType<BacktesterMarketDataService> service)
      : BacktesterEvent{boost::posix_time::neg_infin},
        m_index{std::move(index)},
        m_startPoint{startPoint},
        m_service{service.Get()} {}

  template<typename MarketDataTypeType>
  void MarketDataLoadEvent<MarketDataTypeType>::Execute() {
    auto QUERY_SIZE = 1000;
    auto endPoint =
      [&] () -> Beam::Queries::Range::Point {
        if(m_service->m_eventHandler->GetEndTime() ==
            boost::posix_time::pos_infin) {
          return Beam::Queries::Sequence::Present();
        }
        return m_service->m_eventHandler->GetEndTime();
      }();
    Query query;
    query.SetIndex(m_index);
    query.SetRange(m_startPoint, endPoint);
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::HEAD,
      QUERY_SIZE);
    auto queue = std::make_shared<Beam::Queue<
      Beam::Queries::SequencedValue<MarketDataType>>>();
    MarketDataService::QueryMarketDataClient(*m_service->m_marketDataClient,
      query, queue);
    std::vector<Beam::Queries::SequencedValue<MarketDataType>> data;
    Beam::FlushQueue(queue, std::back_inserter(data));
    if(data.empty()) {
      return;
    }
    std::vector<std::shared_ptr<BacktesterEvent>> events;
    auto reloadEvent = std::make_shared<MarketDataLoadEvent>(m_index,
      Beam::Queries::Increment(data.back().GetSequence()),
      Beam::Ref(*m_service));
    auto timestamp = m_service->m_eventHandler->GetTime();
    for(auto& value : data) {
      timestamp = std::max(timestamp,
        Beam::Queries::GetTimestamp(value.GetValue()));
      events.push_back(std::make_shared<
        MarketDataEvent<typename Query::Index, MarketDataType>>(
        query.GetIndex(), std::move(value), timestamp, Beam::Ref(*m_service)));
    }
    events.push_back(std::move(reloadEvent));
    m_service->m_eventHandler->Add(std::move(events));
  }

  template<typename IndexType, typename MarketDataTypeType>
  MarketDataEvent<IndexType, MarketDataTypeType>::MarketDataEvent(Index index,
      MarketDataType value, boost::posix_time::ptime timestamp,
      Beam::RefType<BacktesterMarketDataService> service)
      : BacktesterEvent{timestamp},
        m_index{std::move(index)},
        m_value{std::move(value)},
        m_service{service.Get()} {}

  template<typename IndexType, typename MarketDataTypeType>
  void MarketDataEvent<IndexType, MarketDataTypeType>::Execute() {

    // TODO
//    m_service->m_eventHandler->GetTestEnvironment().Update(m_index, m_value);
  }
}

#endif
