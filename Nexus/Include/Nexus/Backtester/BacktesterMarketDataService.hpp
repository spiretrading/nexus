#ifndef NEXUS_BACKTESTER_MARKET_DATA_SERVICE_HPP
#define NEXUS_BACKTESTER_MARKET_DATA_SERVICE_HPP
#include <memory>
#include <tuple>
#include <unordered_set>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Utilities/HashTuple.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/QueryTypes.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"

namespace Nexus {

  /** Provides historical market data to the backtester. */
  class BacktesterMarketDataService {
    public:

      /**
       * Constructs a BacktesterMarketDataService.
       * @param event_handler The BacktesterEventHandler to push historical
       *        market data events to.
       * @param market_data_environment The object to publish market data
       *        updates to.
       * @param market_data_client The MarketDataClient used to retrieve
       *        historical market data.
       */
      BacktesterMarketDataService(
        Beam::Ref<BacktesterEventHandler> event_handler,
        Beam::Ref<MarketDataService::Tests::MarketDataServiceTestEnvironment>
          market_data_environment,
        MarketDataService::MarketDataClient market_data_client) noexcept;

      /**
       * Submits a query for OrderImbalances.
       * @param query The venue market data query to submit.
       */
      void query_order_imbalances(
        const MarketDataService::VenueMarketDataQuery& query);

      /**
       * Submits a query for BboQuotes.
       * @param query The security market data query to submit.
       */
      void query_bbo_quotes(
        const MarketDataService::SecurityMarketDataQuery& query);

      /**
       * Submits a query for BookQuotes.
       * @param query The security market data query to submit.
       */
      void query_book_quotes(
        const MarketDataService::SecurityMarketDataQuery& query);

      /**
       * Submits a query for TimeAndSales.
       * @param query The security market data query to submit.
       */
      void query_time_and_sales(
        const MarketDataService::SecurityMarketDataQuery& query);

    private:
      template<typename, typename> friend class MarketDataEvent;
      template<typename> friend class MarketDataLoadEvent;
      template<typename> friend class MarketDataQueryEvent;
      BacktesterEventHandler* m_event_handler;
      MarketDataService::Tests::MarketDataServiceTestEnvironment*
        m_market_data_environment;
      MarketDataService::MarketDataClient m_market_data_client;
      std::unordered_set<std::tuple<boost::variant<Security, Venue>,
        MarketDataService::MarketDataType>> m_queries;

      BacktesterMarketDataService(const BacktesterMarketDataService&) = delete;
      BacktesterMarketDataService& operator =(
        const BacktesterMarketDataService&) = delete;
  };

  /**
   * Represents a query event for a specific market data type.
   * Triggers the loading of historical market data for the given query.
   * @param <T> The type of market data being queried.
   */
  template<typename T>
  class MarketDataQueryEvent : public BacktesterEvent {
    public:

      /** The type of market data being queried. */
      using MarketDataType = T;

      /** The query type for the market data. */
      using QueryType = MarketDataService::market_data_query_type_t<
        Beam::Queries::SequencedValue<MarketDataType>>;

      /**
       * Constructs a MarketDataQueryEvent.
       * @param query The market data query.
       * @param service The market data service to use.
       */
      MarketDataQueryEvent(QueryType query,
        Beam::Ref<BacktesterMarketDataService> service) noexcept;

      void execute() override;

    private:
      QueryType m_query;
      BacktesterMarketDataService* m_service;
  };

  /**
   * Loads a batch of historical market data for a specific type and index.
   * @param <T> The type of market data being loaded.
   */
  template<typename T>
  class MarketDataLoadEvent : public BacktesterEvent {
    public:

      /** The type of market data being loaded. */
      using MarketDataType = T;

      /** The query type for the market data. */
      using QueryType = MarketDataService::market_data_query_type_t<
        Beam::Queries::SequencedValue<MarketDataType>>;

      /**
       * Constructs a MarketDataLoadEvent.
       * @param index The index for the market data.
       * @param start The starting point for the query range.
       * @param timestamp The timestamp for the event.
       * @param service The market data service to use.
       */
      MarketDataLoadEvent(typename QueryType::Index index,
        Beam::Queries::Range::Point start, boost::posix_time::ptime timestamp,
        Beam::Ref<BacktesterMarketDataService> service) noexcept;

      void execute() override;

    private:
      typename QueryType::Index m_index;
      Beam::Queries::Range::Point m_start;
      BacktesterMarketDataService* m_service;
  };

  /**
   * Publishes a single market data value to the test environment.
   * @param <I> The index type for the market data.
   * @param <T> The type of market data being published.
   */
  template<typename I, typename T>
  class MarketDataEvent : public BacktesterEvent {
    public:

      /** The index type for the market data. */
      using Index = I;

      /** The type of market data being published. */
      using MarketDataType = T;

      /**
       * Constructs a MarketDataEvent.
       * @param index The index for the market data.
       * @param value The market data value.
       * @param timestamp The timestamp for the event.
       * @param service The market data service to use.
       */
      MarketDataEvent(Index index, MarketDataType value,
        boost::posix_time::ptime timestamp,
        Beam::Ref<BacktesterMarketDataService> service) noexcept;

      void execute() override;

    private:
      Index m_index;
      MarketDataType m_value;
      BacktesterMarketDataService* m_service;
  };

  inline BacktesterMarketDataService::BacktesterMarketDataService(
    Beam::Ref<BacktesterEventHandler> event_handler,
    Beam::Ref<MarketDataService::Tests::MarketDataServiceTestEnvironment>
      market_data_environment,
    MarketDataService::MarketDataClient market_data_client) noexcept
    : m_event_handler(event_handler.Get()),
      m_market_data_environment(market_data_environment.Get()),
      m_market_data_client(std::move(market_data_client)) {}

  inline void BacktesterMarketDataService::query_order_imbalances(
      const MarketDataService::VenueMarketDataQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<OrderImbalance>>(
      query, Beam::Ref(*this));
    m_event_handler->add(event);
  }

  inline void BacktesterMarketDataService::query_bbo_quotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<BboQuote>>(
      query, Beam::Ref(*this));
    m_event_handler->add(event);
  }

  inline void BacktesterMarketDataService::query_book_quotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<BookQuote>>(
      query, Beam::Ref(*this));
    m_event_handler->add(event);
  }

  inline void BacktesterMarketDataService::query_time_and_sales(
      const MarketDataService::SecurityMarketDataQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<TimeAndSale>>(
      query, Beam::Ref(*this));
    m_event_handler->add(event);
  }

  template<typename T>
  MarketDataQueryEvent<T>::MarketDataQueryEvent(QueryType query,
    Beam::Ref<BacktesterMarketDataService> service) noexcept
    : BacktesterEvent(boost::posix_time::neg_infin),
      m_query(std::move(query)),
      m_service(service.Get()) {}

  template<typename T>
  void MarketDataQueryEvent<T>::execute() {
    auto type = MarketDataService::get_market_data_type<MarketDataType>();
    auto key = std::tuple(m_query.GetIndex(), type);
    if(m_query.GetRange().GetEnd() != Beam::Queries::Sequence::Last() ||
        !m_service->m_queries.insert(key).second) {
      return;
    }
    auto time = m_service->m_event_handler->get_time();
    auto event = std::make_shared<MarketDataLoadEvent<MarketDataType>>(
      m_query.GetIndex(), time, boost::posix_time::neg_infin,
      Beam::Ref(*m_service));
    m_service->m_event_handler->add(event);
  }

  template<typename T>
  MarketDataLoadEvent<T>::MarketDataLoadEvent(typename QueryType::Index index,
    Beam::Queries::Range::Point start, boost::posix_time::ptime timestamp,
    Beam::Ref<BacktesterMarketDataService> service) noexcept
    : BacktesterEvent(timestamp),
      m_index(std::move(index)),
      m_start(start),
      m_service(service.Get()) {}

  template<typename T>
  void MarketDataLoadEvent<T>::execute() {
    const auto QUERY_SIZE = 1000;
    auto end = [&] () -> Beam::Queries::Range::Point {
      if(m_service->m_event_handler->get_end_time() ==
          boost::posix_time::pos_infin) {
        return Beam::Queries::Sequence::Present();
      }
      return m_service->m_event_handler->get_end_time();
    }();
    auto query = QueryType();
    query.SetIndex(m_index);
    query.SetRange(m_start, end);
    query.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Type::HEAD, QUERY_SIZE);
    auto queue = std::make_shared<
      Beam::Queue<Beam::Queries::SequencedValue<MarketDataType>>>();
    m_service->m_market_data_client.query(query, queue);
    auto data = std::vector<Beam::Queries::SequencedValue<MarketDataType>>();
    Beam::Flush(queue, std::back_inserter(data));
    if(data.empty()) {
      return;
    }
    auto events = std::vector<std::shared_ptr<BacktesterEvent>>();
    auto timestamp = m_service->m_event_handler->get_time();
    for(auto& value : data) {
      timestamp =
        std::max(timestamp, Beam::Queries::GetTimestamp(value.GetValue()));
      events.push_back(std::make_shared<MarketDataEvent<
        typename QueryType::Index, MarketDataType>>(query.GetIndex(),
          std::move(value), timestamp, Beam::Ref(*m_service)));
    }
    auto reload_event = std::make_shared<MarketDataLoadEvent>(m_index,
      Beam::Queries::Increment(data.back().GetSequence()),
      events.back()->get_timestamp(), Beam::Ref(*m_service));
    events.push_back(reload_event);
    m_service->m_event_handler->add(events);
  }

  template<typename I, typename T>
  MarketDataEvent<I, T>::MarketDataEvent(Index index, MarketDataType value,
    boost::posix_time::ptime timestamp,
    Beam::Ref<BacktesterMarketDataService> service) noexcept
    : BacktesterEvent(timestamp),
      m_index(std::move(index)),
      m_value(std::move(value)),
      m_service(service.Get()) {}

  template<typename I, typename T>
  void MarketDataEvent<I, T>::execute() {
    m_service->m_market_data_environment->get_feed_client().publish(
      Beam::Queries::IndexedValue(m_value, m_index));
  }
}

#endif
