#ifndef NEXUS_TEST_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_TEST_MARKET_DATA_FEED_CLIENT_HPP
#include <variant>
#include <Beam/Queues/Queue.hpp>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include <Beam/ServicesTests/TestServiceClientOperationQueue.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

namespace Nexus::Tests {

  /** Records feed-client operations and lets tests supply their results. */
  class TestMarketDataFeedClient {
    public:

      /** Records a call to add(). */
      struct AddOperation {
        TickerInfo m_info;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to publish() for an order imbalance. */
      struct PublishOrderImbalanceOperation {
        VenueOrderImbalance m_imbalance;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to publish() for a BBO quote. */
      struct PublishBboQuoteOperation {
        TickerBboQuote m_quote;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to publish() for a book quote. */
      struct PublishBookQuoteOperation {
        TickerBookQuote m_quote;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to publish() for a time and sale. */
      struct PublishTimeAndSaleOperation {
        TickerTimeAndSale m_time_and_sale;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to publish() for a ticker status. */
      struct PublishTickerStatusOperation {
        IndexedTickerStatus m_status;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to add_order(). */
      struct AddOrderOperation {
        Ticker m_ticker;
        Venue m_venue;
        std::string m_mpid;
        bool m_is_primary_mpid;
        std::string m_id;
        Side m_side;
        Money m_price;
        Quantity m_size;
        boost::posix_time::ptime m_timestamp;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to modify_order_size(). */
      struct ModifyOrderSizeOperation {
        std::string m_id;
        Quantity m_size;
        boost::posix_time::ptime m_timestamp;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to offset_order_size(). */
      struct OffsetOrderSizeOperation {
        std::string m_id;
        Quantity m_delta;
        boost::posix_time::ptime m_timestamp;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to modify_order_price(). */
      struct ModifyOrderPriceOperation {
        std::string m_id;
        Money m_price;
        boost::posix_time::ptime m_timestamp;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to remove_order(). */
      struct RemoveOrderOperation {
        std::string m_id;
        boost::posix_time::ptime m_timestamp;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** The operations performed by the client. */
      using Operation = std::variant<AddOperation,
        PublishOrderImbalanceOperation, PublishBboQuoteOperation,
        PublishBookQuoteOperation, PublishTimeAndSaleOperation,
        PublishTickerStatusOperation, AddOrderOperation,
        ModifyOrderSizeOperation, OffsetOrderSizeOperation,
        ModifyOrderPriceOperation, RemoveOrderOperation>;

      /** The queue receiving recorded operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestMarketDataFeedClient.
       * @param operations The queue receiving operations.
       */
      explicit TestMarketDataFeedClient(Beam::ScopedQueueWriter<
        std::shared_ptr<Operation>> operations) noexcept;

      ~TestMarketDataFeedClient();

      void add(const TickerInfo& info);
      void publish(const VenueOrderImbalance& imbalance);
      void publish(const TickerBboQuote& quote);
      void publish(const TickerBookQuote& quote);
      void publish(const TickerTimeAndSale& time_and_sale);
      void publish(const IndexedTickerStatus& status);
      void add_order(const Ticker& ticker, Venue venue, const std::string& mpid,
        bool is_primary_mpid, const std::string& id, Side side, Money price,
        Quantity size, boost::posix_time::ptime timestamp);
      void modify_order_size(const std::string& id, Quantity size,
        boost::posix_time::ptime timestamp);
      void offset_order_size(const std::string& id, Quantity delta,
        boost::posix_time::ptime timestamp);
      void modify_order_price(
        const std::string& id, Money price, boost::posix_time::ptime timestamp);
      void remove_order(
        const std::string& id, boost::posix_time::ptime timestamp);
      void close();

    private:
      Beam::Tests::TestServiceClientOperationQueue<Operation> m_operations;

      TestMarketDataFeedClient(const TestMarketDataFeedClient&) = delete;
      TestMarketDataFeedClient& operator =(
        const TestMarketDataFeedClient&) = delete;
  };

  inline TestMarketDataFeedClient::TestMarketDataFeedClient(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_operations(std::move(operations)) {}

  inline TestMarketDataFeedClient::~TestMarketDataFeedClient() {
    close();
  }

  inline void TestMarketDataFeedClient::add(const TickerInfo& info) {
    m_operations.append_result<AddOperation, void>(info);
  }

  inline void TestMarketDataFeedClient::publish(
      const VenueOrderImbalance& imbalance) {
    m_operations.append_result<PublishOrderImbalanceOperation, void>(
      imbalance);
  }

  inline void TestMarketDataFeedClient::publish(const TickerBboQuote& quote) {
    m_operations.append_result<PublishBboQuoteOperation, void>(quote);
  }

  inline void TestMarketDataFeedClient::publish(const TickerBookQuote& quote) {
    m_operations.append_result<PublishBookQuoteOperation, void>(quote);
  }

  inline void TestMarketDataFeedClient::publish(
      const TickerTimeAndSale& time_and_sale) {
    m_operations.append_result<PublishTimeAndSaleOperation, void>(
      time_and_sale);
  }

  inline void TestMarketDataFeedClient::publish(
      const IndexedTickerStatus& status) {
    m_operations.append_result<PublishTickerStatusOperation, void>(status);
  }

  inline void TestMarketDataFeedClient::add_order(const Ticker& ticker,
      Venue venue, const std::string& mpid, bool is_primary_mpid,
      const std::string& id, Side side, Money price, Quantity size,
      boost::posix_time::ptime timestamp) {
    m_operations.append_result<AddOrderOperation, void>(ticker, venue, mpid,
      is_primary_mpid, id, side, price, size, timestamp);
  }

  inline void TestMarketDataFeedClient::modify_order_size(const std::string& id,
      Quantity size, boost::posix_time::ptime timestamp) {
    m_operations.append_result<ModifyOrderSizeOperation, void>(
      id, size, timestamp);
  }

  inline void TestMarketDataFeedClient::offset_order_size(const std::string& id,
      Quantity delta, boost::posix_time::ptime timestamp) {
    m_operations.append_result<OffsetOrderSizeOperation, void>(
      id, delta, timestamp);
  }

  inline void TestMarketDataFeedClient::modify_order_price(
      const std::string& id, Money price, boost::posix_time::ptime timestamp) {
    m_operations.append_result<ModifyOrderPriceOperation, void>(
      id, price, timestamp);
  }

  inline void TestMarketDataFeedClient::remove_order(
      const std::string& id, boost::posix_time::ptime timestamp) {
    m_operations.append_result<RemoveOrderOperation, void>(id, timestamp);
  }

  inline void TestMarketDataFeedClient::close() {
    m_operations.close();
  }
}

#endif
