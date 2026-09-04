#ifndef NEXUS_BACKTESTER_MARKET_DATA_SERVICE_HPP
#define NEXUS_BACKTESTER_MARKET_DATA_SERVICE_HPP
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/HashTuple.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/QueryTypes.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"

namespace Nexus {
  class BacktesterMarketDataService;

namespace Details {
  using BacktesterMarketDataServiceHandle =
    std::shared_ptr<Beam::Sync<BacktesterMarketDataService*, Beam::Mutex>>;
}

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
        Beam::Ref<Tests::MarketDataServiceTestEnvironment>
          market_data_environment,
        MarketDataClient market_data_client);

      ~BacktesterMarketDataService();

      /**
       * Submits a query for OrderImbalances.
       * @param query The venue query to submit.
       */
      void query_order_imbalances(const VenueQuery& query);

      /**
       * Submits a query for BboQuotes.
       * @param query The ticker query to submit.
       */
      void query_bbo_quotes(const TickerQuery& query);

      /**
       * Submits a query for BookQuotes.
       * @param query The ticker query to submit.
       */
      void query_book_quotes(const TickerQuery& query);

      /**
       * Submits a query for TimeAndSales.
       * @param query The ticker query to submit.
       */
      void query_time_and_sales(const TickerQuery& query);

      /**
       * Submits a query for TickerStatuses.
       * @param query The ticker query to submit.
       */
      void query_ticker_statuses(const TickerQuery& query);

      /**
       * Sets the callback invoked with every BboQuote before it is published.
       * @param slot The callback to invoke.
       */
      void set_bbo_slot(
        std::function<void (const Ticker&, const BboQuote&)> slot);

      /**
       * Sets the callback invoked with every TimeAndSale before it is
       * published.
       * @param slot The callback to invoke.
       */
      void set_time_and_sale_slot(
        std::function<void (const Ticker&, const TimeAndSale&)> slot);

      /**
       * Sets the callback invoked with every BookQuote before it is published.
       * @param slot The callback to invoke.
       */
      void set_book_quote_slot(
        std::function<void (const Ticker&, const BookQuote&)> slot);

    private:
      template<typename, typename> friend class MarketDataEvent;
      template<typename> friend class MarketDataLoadEvent;
      template<typename> friend class MarketDataQueryEvent;
      BacktesterEventHandler* m_event_handler;
      Tests::MarketDataServiceTestEnvironment* m_market_data_environment;
      MarketDataClient m_market_data_client;
      std::function<void (const Ticker&, const BboQuote&)> m_bbo_slot;
      std::function<void (const Ticker&, const TimeAndSale&)>
        m_time_and_sale_slot;
      std::function<void (const Ticker&, const BookQuote&)> m_book_quote_slot;
      std::unordered_set<
        std::tuple<boost::variant<Ticker, Venue>, MarketDataType>> m_queries;
      Details::BacktesterMarketDataServiceHandle m_self;

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
      using QueryType =
        market_data_query_type_t<Beam::SequencedValue<MarketDataType>>;

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
      Details::BacktesterMarketDataServiceHandle m_service;
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
      using QueryType =
        market_data_query_type_t<Beam::SequencedValue<MarketDataType>>;

      /**
       * Constructs a MarketDataLoadEvent.
       * @param index The index for the market data.
       * @param start The starting point for the query range.
       * @param timestamp The timestamp for the event.
       * @param service The market data service to use.
       */
      MarketDataLoadEvent(typename QueryType::Index index,
        Beam::Range::Point start, boost::posix_time::ptime timestamp,
        Beam::Ref<BacktesterMarketDataService> service) noexcept;

      void execute() override;

    private:
      typename QueryType::Index m_index;
      Beam::Range::Point m_start;
      Details::BacktesterMarketDataServiceHandle m_service;
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
      Details::BacktesterMarketDataServiceHandle m_service;
  };

  inline BacktesterMarketDataService::BacktesterMarketDataService(
      Beam::Ref<BacktesterEventHandler> event_handler,
      Beam::Ref<Tests::MarketDataServiceTestEnvironment>
        market_data_environment, MarketDataClient market_data_client)
      : m_event_handler(event_handler.get()),
        m_market_data_environment(market_data_environment.get()),
        m_market_data_client(std::move(market_data_client)) {
    m_self = std::make_shared<
      Beam::Sync<BacktesterMarketDataService*, Beam::Mutex>>(this);
  }

  inline BacktesterMarketDataService::~BacktesterMarketDataService() {
    m_self->with([] (auto& service) {
      service = nullptr;
    });
  }

  inline void BacktesterMarketDataService::query_order_imbalances(
      const VenueQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<OrderImbalance>>(
      query, Beam::Ref(*this));
    m_event_handler->add(event);
  }

  inline void BacktesterMarketDataService::query_bbo_quotes(
      const TickerQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<BboQuote>>(
      query, Beam::Ref(*this));
    m_event_handler->add(event);
  }

  inline void BacktesterMarketDataService::query_book_quotes(
      const TickerQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<BookQuote>>(
      query, Beam::Ref(*this));
    m_event_handler->add(event);
  }

  inline void BacktesterMarketDataService::query_time_and_sales(
      const TickerQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<TimeAndSale>>(
      query, Beam::Ref(*this));
    m_event_handler->add(event);
  }

  inline void BacktesterMarketDataService::query_ticker_statuses(
      const TickerQuery& query) {
    auto event = std::make_shared<MarketDataQueryEvent<TickerStatus>>(
      query, Beam::Ref(*this));
    m_event_handler->add(event);
  }

  inline void BacktesterMarketDataService::set_bbo_slot(
      std::function<void (const Ticker&, const BboQuote&)> slot) {
    m_bbo_slot = std::move(slot);
  }

  inline void BacktesterMarketDataService::set_time_and_sale_slot(
      std::function<void (const Ticker&, const TimeAndSale&)> slot) {
    m_time_and_sale_slot = std::move(slot);
  }

  inline void BacktesterMarketDataService::set_book_quote_slot(
      std::function<void (const Ticker&, const BookQuote&)> slot) {
    m_book_quote_slot = std::move(slot);
  }

  template<typename T>
  MarketDataQueryEvent<T>::MarketDataQueryEvent(QueryType query,
    Beam::Ref<BacktesterMarketDataService> service) noexcept
    : BacktesterEvent(boost::posix_time::neg_infin),
      m_query(std::move(query)),
      m_service(service->m_self) {}

  template<typename T>
  void MarketDataQueryEvent<T>::execute() {
    Beam::with(*m_service, [&] (const auto& service) {
      if(!service) {
        return;
      }
      auto type = get_market_data_type<MarketDataType>();
      auto key = std::tuple(m_query.get_index(), type);
      if(m_query.get_range().get_end() != Beam::Sequence::LAST ||
          !service->m_queries.insert(key).second) {
        return;
      }
      auto time = service->m_event_handler->get_time();
      auto event = std::make_shared<MarketDataLoadEvent<MarketDataType>>(
        m_query.get_index(), time, boost::posix_time::neg_infin,
        Beam::Ref(*service));
      service->m_event_handler->add(event);
    });
  }

  template<typename T>
  MarketDataLoadEvent<T>::MarketDataLoadEvent(typename QueryType::Index index,
    Beam::Range::Point start, boost::posix_time::ptime timestamp,
    Beam::Ref<BacktesterMarketDataService> service) noexcept
    : BacktesterEvent(timestamp),
      m_index(std::move(index)),
      m_start(start),
      m_service(service->m_self) {}

  template<typename T>
  void MarketDataLoadEvent<T>::execute() {
    Beam::with(*m_service, [&] (const auto& service) {
      if(!service) {
        return;
      }
      const auto QUERY_SIZE = 1000;
      auto end = [&] () -> Beam::Range::Point {
        if(service->m_event_handler->get_end_time() ==
            boost::posix_time::pos_infin) {
          return Beam::Sequence::PRESENT;
        }
        return service->m_event_handler->get_end_time();
      }();
      auto query = QueryType();
      query.set_index(m_index);
      query.set_range(m_start, end);
      query.set_snapshot_limit(Beam::SnapshotLimit::Type::HEAD, QUERY_SIZE);
      auto queue =
        std::make_shared<Beam::Queue<Beam::SequencedValue<MarketDataType>>>();
      service->m_market_data_client.query(query, queue);
      auto data = std::vector<Beam::SequencedValue<MarketDataType>>();
      Beam::flush(queue, std::back_inserter(data));
      if(data.empty()) {
        return;
      }
      auto events = std::vector<std::shared_ptr<BacktesterEvent>>();
      auto timestamp = service->m_event_handler->get_time();
      for(auto& value : data) {
        timestamp = std::max(timestamp, Beam::get_timestamp(value.get_value()));
        events.push_back(std::make_shared<MarketDataEvent<
          typename QueryType::Index, MarketDataType>>(query.get_index(),
            std::move(value), timestamp, Beam::Ref(*service)));
      }
      auto reload_event = std::make_shared<MarketDataLoadEvent>(
        m_index, Beam::increment(data.back().get_sequence()),
        events.back()->get_timestamp(), Beam::Ref(*service));
      events.push_back(reload_event);
      service->m_event_handler->add(events);
    });
  }

  template<typename I, typename T>
  MarketDataEvent<I, T>::MarketDataEvent(Index index, MarketDataType value,
    boost::posix_time::ptime timestamp,
    Beam::Ref<BacktesterMarketDataService> service) noexcept
    : BacktesterEvent(timestamp),
      m_index(std::move(index)),
      m_value(std::move(value)),
      m_service(service->m_self) {}

  template<typename I, typename T>
  void MarketDataEvent<I, T>::execute() {
    Beam::with(*m_service, [&] (const auto& service) {
      if(!service) {
        return;
      }
      if constexpr(std::is_same_v<Index, Ticker> &&
          std::is_same_v<MarketDataType, BboQuote>) {
        if(service->m_bbo_slot) {
          service->m_bbo_slot(m_index, m_value);
        }
      } else if constexpr(std::is_same_v<Index, Ticker> &&
          std::is_same_v<MarketDataType, TimeAndSale>) {
        if(service->m_time_and_sale_slot) {
          service->m_time_and_sale_slot(m_index, m_value);
        }
      } else if constexpr(std::is_same_v<Index, Ticker> &&
          std::is_same_v<MarketDataType, BookQuote>) {
        if(service->m_book_quote_slot) {
          service->m_book_quote_slot(m_index, m_value);
        }
      }
      service->m_market_data_environment->get_feed_client().publish(
        Beam::IndexedValue(m_value, m_index));
    });
  }
}

#endif
