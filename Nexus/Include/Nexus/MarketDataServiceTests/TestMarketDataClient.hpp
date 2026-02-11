#ifndef NEXUS_TEST_MARKET_DATA_CLIENT_HPP
#define NEXUS_TEST_MARKET_DATA_CLIENT_HPP
#include <variant>
#include <Beam/Queues/Queue.hpp>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include <Beam/ServicesTests/TestServiceClientOperationQueue.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus::Tests {

  /**
   * Implements a MarketDataClient for testing purposes by pushing all
   * operations performed on this client onto a queue.
   */
  class TestMarketDataClient {
    public:

      /** Records a call to query(...) for SequencedOrderImbalance. */
      struct QuerySequencedOrderImbalanceOperation {

        /** The VenueMarketDataQuery passed. */
        VenueMarketDataQuery m_query;

        /** The queue writer for SequencedOrderImbalance. */
        Beam::ScopedQueueWriter<SequencedOrderImbalance> m_queue;
      };

      /** Records a call to query(...) for OrderImbalance. */
      struct QueryOrderImbalanceOperation {

        /** The VenueMarketDataQuery passed. */
        VenueMarketDataQuery m_query;

        /** The queue writer for OrderImbalance. */
        Beam::ScopedQueueWriter<OrderImbalance> m_queue;
      };

      /** Records a call to query(...) for SequencedBboQuote. */
      struct QuerySequencedBboQuoteOperation {

        /** The TickerMarketDataQuery passed. */
        TickerMarketDataQuery m_query;

        /** The queue writer for SequencedBboQuote. */
        Beam::ScopedQueueWriter<SequencedBboQuote> m_queue;
      };

      /** Records a call to query(...) for BboQuote. */
      struct QueryBboQuoteOperation {

        /** The TickerMarketDataQuery passed. */
        TickerMarketDataQuery m_query;

        /** The queue writer for BboQuote. */
        Beam::ScopedQueueWriter<BboQuote> m_queue;
      };

      /** Records a call to query(...) for SequencedBookQuote. */
      struct QuerySequencedBookQuoteOperation {

        /** The TickerMarketDataQuery passed. */
        TickerMarketDataQuery m_query;

        /** The queue writer for SequencedBookQuote. */
        Beam::ScopedQueueWriter<SequencedBookQuote> m_queue;
      };

      /** Records a call to query(...) for BookQuote. */
      struct QueryBookQuoteOperation {

        /** The TickerMarketDataQuery passed. */
        TickerMarketDataQuery m_query;

        /** The queue writer for BookQuote. */
        Beam::ScopedQueueWriter<BookQuote> m_queue;
      };

      /** Records a call to query(...) for SequencedTimeAndSale. */
      struct QuerySequencedTimeAndSaleOperation {

        /** The TickerMarketDataQuery passed. */
        TickerMarketDataQuery m_query;

        /** The queue writer for SequencedTimeAndSale. */
        Beam::ScopedQueueWriter<SequencedTimeAndSale> m_queue;
      };

      /** Records a call to query(...) for TimeAndSale. */
      struct QueryTimeAndSaleOperation {

        /** The TickerMarketDataQuery passed. */
        TickerMarketDataQuery m_query;

        /** The queue writer for TimeAndSale. */
        Beam::ScopedQueueWriter<TimeAndSale> m_queue;
      };

      /** Records a call to query(...) for TickerInfoQuery. */
      struct TickerInfoQueryOperation {

        /** The TickerInfoQuery passed. */
        TickerInfoQuery m_query;

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<std::vector<TickerInfo>> m_result;
      };

      /** Records a call to load_snapshot(...). */
      struct LoadTickerSnapshotOperation {

        /** The Ticker passed. */
        Ticker m_ticker;

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<TickerSnapshot> m_result;
      };

      /** Records a call to load_session_candlestick(...). */
      struct LoadSessionCandlestickOperation {

        /** The Ticker passed. */
        Ticker m_ticker;

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<PriceCandlestick> m_result;
      };

      /** Records a call to load_ticker_info_from_prefix(...). */
      struct LoadTickerInfoFromPrefixOperation {

        /** The prefix string passed. */
        std::string m_prefix;

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<std::vector<TickerInfo>> m_result;
      };

      /** A variant covering all possible TestMarketDataClient operations. */
      using Operation = std::variant<QuerySequencedOrderImbalanceOperation,
        QueryOrderImbalanceOperation, QuerySequencedBboQuoteOperation,
        QueryBboQuoteOperation, QuerySequencedBookQuoteOperation,
        QueryBookQuoteOperation, QuerySequencedTimeAndSaleOperation,
        QueryTimeAndSaleOperation, TickerInfoQueryOperation,
        LoadTickerSnapshotOperation, LoadSessionCandlestickOperation,
        LoadTickerInfoFromPrefixOperation>;

      /** The type of Queue used to send and receive operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestMarketDataClient.
       * @param operations The queue to push all operations on.
       */
      explicit TestMarketDataClient(Beam::ScopedQueueWriter<
        std::shared_ptr<Operation>> operations) noexcept;

      ~TestMarketDataClient();

      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);
      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);
      std::vector<TickerInfo> query(const TickerInfoQuery& query);
      TickerSnapshot load_snapshot(const Ticker& ticker);
      PriceCandlestick load_session_candlestick(const Ticker& ticker);
      std::vector<TickerInfo> load_ticker_info_from_prefix(
        const std::string& prefix);
      void close();

    private:
      Beam::Tests::TestServiceClientOperationQueue<Operation> m_queue;

      TestMarketDataClient(const TestMarketDataClient&) = delete;
      TestMarketDataClient& operator =(const TestMarketDataClient&) = delete;
  };

  inline TestMarketDataClient::TestMarketDataClient(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_queue(std::move(operations)) {}

  inline TestMarketDataClient::~TestMarketDataClient() {
    close();
  }

  inline void TestMarketDataClient::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    auto operation = std::make_shared<Operation>(
      std::in_place_type<QuerySequencedOrderImbalanceOperation>,
      query, std::move(queue));
    m_queue.append_queue<QuerySequencedOrderImbalanceOperation>(operation);
  }

  inline void TestMarketDataClient::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    auto operation = std::make_shared<Operation>(
      std::in_place_type<QueryOrderImbalanceOperation>,
      query, std::move(queue));
    m_queue.append_queue<QueryOrderImbalanceOperation>(operation);
  }

  inline void TestMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    auto operation = std::make_shared<Operation>(
      std::in_place_type<QuerySequencedBboQuoteOperation>,
      query, std::move(queue));
    m_queue.append_queue<QuerySequencedBboQuoteOperation>(operation);
  }

  inline void TestMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    auto operation = std::make_shared<Operation>(
      std::in_place_type<QueryBboQuoteOperation>,
      query, std::move(queue));
    m_queue.append_queue<QueryBboQuoteOperation>(operation);
  }

  inline void TestMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    auto operation = std::make_shared<Operation>(
      std::in_place_type<QuerySequencedBookQuoteOperation>,
      query, std::move(queue));
    m_queue.append_queue<QuerySequencedBookQuoteOperation>(operation);
  }

  inline void TestMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    auto operation = std::make_shared<Operation>(
      std::in_place_type<QueryBookQuoteOperation>,
      query, std::move(queue));
    m_queue.append_queue<QueryBookQuoteOperation>(operation);
  }

  inline void TestMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    auto operation = std::make_shared<Operation>(
      std::in_place_type<QuerySequencedTimeAndSaleOperation>,
      query, std::move(queue));
    m_queue.append_queue<QuerySequencedTimeAndSaleOperation>(operation);
  }

  inline void TestMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    auto operation = std::make_shared<Operation>(
      std::in_place_type<QueryTimeAndSaleOperation>,
      query, std::move(queue));
    m_queue.append_queue<QueryTimeAndSaleOperation>(operation);
  }

  inline std::vector<TickerInfo> TestMarketDataClient::query(
      const TickerInfoQuery& query) {
    return m_queue.append_result<
      TickerInfoQueryOperation, std::vector<TickerInfo>>(query);
  }

  inline TickerSnapshot TestMarketDataClient::load_snapshot(
      const Ticker& ticker) {
    return m_queue.append_result<
      LoadTickerSnapshotOperation, TickerSnapshot>(ticker);
  }

  inline PriceCandlestick TestMarketDataClient::load_session_candlestick(
      const Ticker& ticker) {
    return m_queue.append_result<
      LoadSessionCandlestickOperation, PriceCandlestick>(ticker);
  }

  inline std::vector<TickerInfo> TestMarketDataClient::
      load_ticker_info_from_prefix(const std::string& prefix) {
    return m_queue.append_result<
      LoadTickerInfoFromPrefixOperation, std::vector<TickerInfo>>(prefix);
  }

  inline void TestMarketDataClient::close() {
    m_queue.close();
  }
}

#endif
