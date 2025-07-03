#ifndef NEXUS_TEST_MARKET_DATA_CLIENT_HPP
#define NEXUS_TEST_MARKET_DATA_CLIENT_HPP
#include <variant>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus::MarketDataService::Tests {

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

        /** The SecurityMarketDataQuery passed. */
        SecurityMarketDataQuery m_query;

        /** The queue writer for SequencedBboQuote. */
        Beam::ScopedQueueWriter<SequencedBboQuote> m_queue;
      };

      /** Records a call to query(...) for BboQuote. */
      struct QueryBboQuoteOperation {

        /** The SecurityMarketDataQuery passed. */
        SecurityMarketDataQuery m_query;

        /** The queue writer for BboQuote. */
        Beam::ScopedQueueWriter<BboQuote> m_queue;
      };

      /** Records a call to query(...) for SequencedBookQuote. */
      struct QuerySequencedBookQuoteOperation {

        /** The SecurityMarketDataQuery passed. */
        SecurityMarketDataQuery m_query;

        /** The queue writer for SequencedBookQuote. */
        Beam::ScopedQueueWriter<SequencedBookQuote> m_queue;
      };

      /** Records a call to query(...) for BookQuote. */
      struct QueryBookQuoteOperation {

        /** The SecurityMarketDataQuery passed. */
        SecurityMarketDataQuery m_query;

        /** The queue writer for BookQuote. */
        Beam::ScopedQueueWriter<BookQuote> m_queue;
      };

      /** Records a call to query(...) for SequencedTimeAndSale. */
      struct QuerySequencedTimeAndSaleOperation {

        /** The SecurityMarketDataQuery passed. */
        SecurityMarketDataQuery m_query;

        /** The queue writer for SequencedTimeAndSale. */
        Beam::ScopedQueueWriter<SequencedTimeAndSale> m_queue;
      };

      /** Records a call to query(...) for TimeAndSale. */
      struct QueryTimeAndSaleOperation {

        /** The SecurityMarketDataQuery passed. */
        SecurityMarketDataQuery m_query;

        /** The queue writer for TimeAndSale. */
        Beam::ScopedQueueWriter<TimeAndSale> m_queue;
      };

      /** Records a call to query(...) for SecurityInfoQuery. */
      struct SecurityInfoQueryOperation {

        /** The SecurityInfoQuery passed. */
        SecurityInfoQuery m_query;

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<std::vector<SecurityInfo>>
          m_result;
      };

      /** Records a call to load_snapshot(...). */
      struct LoadSecuritySnapshotOperation {

        /** The Security passed. */
        Security m_security;

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<SecuritySnapshot> m_result;
      };

      /** Records a call to load_technicals(...). */
      struct LoadSecurityTechnicalsOperation {

        /** The Security passed. */
        Security m_security;

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<SecurityTechnicals> m_result;
      };

      /** Records a call to load_security_info_from_prefix(...). */
      struct LoadSecurityInfoFromPrefixOperation {

        /** The prefix string passed. */
        std::string m_prefix;

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<std::vector<SecurityInfo>>
          m_result;
      };

      /** A variant covering all possible TestMarketDataClient operations. */
      using Operation = std::variant<QuerySequencedOrderImbalanceOperation,
        QueryOrderImbalanceOperation, QuerySequencedBboQuoteOperation,
        QueryBboQuoteOperation, QuerySequencedBookQuoteOperation,
        QueryBookQuoteOperation, QuerySequencedTimeAndSaleOperation,
        QueryTimeAndSaleOperation, SecurityInfoQueryOperation,
        LoadSecuritySnapshotOperation, LoadSecurityTechnicalsOperation,
        LoadSecurityInfoFromPrefixOperation>;

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
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);
      std::vector<SecurityInfo> query(const SecurityInfoQuery& query);
      SecuritySnapshot load_snapshot(const Security& security);
      SecurityTechnicals load_technicals(const Security& security);
      std::vector<SecurityInfo> load_security_info_from_prefix(
        const std::string& prefix);
      void close();

    private:
      Beam::ScopedQueueWriter<std::shared_ptr<Operation>> m_operations;
      Beam::SynchronizedVector<std::weak_ptr<Beam::BaseQueue>> m_queues;
      Beam::SynchronizedUnorderedSet<Beam::Services::Tests::BaseServiceResult*>
        m_pending_results;
      Beam::IO::OpenState m_open_state;

      TestMarketDataClient(const TestMarketDataClient&) = delete;
      TestMarketDataClient& operator =(const TestMarketDataClient&) = delete;
      template<typename T>
      void append_queue(std::shared_ptr<Operation> operation);
      template<typename T, typename R, typename... Args>
      R append_result(Args&&... args);
  };

  inline TestMarketDataClient::TestMarketDataClient(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_operations(std::move(operations)) {}

  inline TestMarketDataClient::~TestMarketDataClient() {
    close();
  }

  inline void TestMarketDataClient::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    append_queue<QuerySequencedOrderImbalanceOperation>(
      std::make_shared<Operation>(QuerySequencedOrderImbalanceOperation(
        query, std::move(queue))));
  }

  inline void TestMarketDataClient::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    append_queue<QueryOrderImbalanceOperation>(std::make_shared<Operation>(
      QueryOrderImbalanceOperation(query, std::move(queue))));
  }

  inline void TestMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    append_queue<QuerySequencedBboQuoteOperation>(std::make_shared<Operation>(
      QuerySequencedBboQuoteOperation(query, std::move(queue))));
  }

  inline void TestMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    append_queue<QueryBboQuoteOperation>(std::make_shared<Operation>(
      QueryBboQuoteOperation(query, std::move(queue))));
  }

  inline void TestMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    append_queue<QuerySequencedBookQuoteOperation>(std::make_shared<Operation>(
      QuerySequencedBookQuoteOperation(query, std::move(queue))));
  }

  inline void TestMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    append_queue<QueryBookQuoteOperation>(std::make_shared<Operation>(
      QueryBookQuoteOperation(query, std::move(queue))));
  }

  inline void TestMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    append_queue<QuerySequencedTimeAndSaleOperation>(
      std::make_shared<Operation>(
        QuerySequencedTimeAndSaleOperation(query, std::move(queue))));
  }

  inline void TestMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    append_queue<QueryTimeAndSaleOperation>(std::make_shared<Operation>(
      QueryTimeAndSaleOperation(query, std::move(queue))));
  }

  inline std::vector<SecurityInfo> TestMarketDataClient::query(
      const SecurityInfoQuery& query) {
    return append_result<SecurityInfoQueryOperation, std::vector<SecurityInfo>>(
      query);
  }

  inline SecuritySnapshot TestMarketDataClient::load_snapshot(
      const Security& security) {
    return append_result<LoadSecuritySnapshotOperation, SecuritySnapshot>(
      security);
  }

  inline SecurityTechnicals TestMarketDataClient::load_technicals(
      const Security& security) {
    return append_result<LoadSecurityTechnicalsOperation, SecurityTechnicals>(
      security);
  }

  inline std::vector<SecurityInfo> TestMarketDataClient::
      load_security_info_from_prefix(const std::string& prefix) {
    return append_result<
      LoadSecurityInfoFromPrefixOperation, std::vector<SecurityInfo>>(prefix);
  }

  inline void TestMarketDataClient::close() {
    if(m_open_state.SetClosing()) {
      m_queues.ForEach([] (const auto& q) {
        if(auto queue = q.lock()) {
          queue->Break();
        }
      });
      m_queues.Clear();
      m_pending_results.With([] (auto& results) {
        for(auto& result : results) {
          result->set(std::make_exception_ptr(Beam::IO::EndOfFileException()));
        }
      });
      m_pending_results.Clear();
    }
    m_open_state.Close();
  }

  template<typename T>
  void TestMarketDataClient::append_queue(
      std::shared_ptr<Operation> operation) {
    auto queue = std::shared_ptr<Beam::BaseQueue>(
      operation, &std::get<T>(*operation).m_queue);
    m_queues.PushBack(queue);
    if(!m_open_state.IsOpen()) {
      m_queues.RemoveIf([&] (const auto& weak_queue) {
        auto q = weak_queue.lock();
        return !q || q == queue;
      });
      queue->Break();
      return;
    }
    m_operations.Push(operation);
  }

  template<typename T, typename R, typename... Args>
  R TestMarketDataClient::append_result(Args&&... args) {
    auto async = Beam::Routines::Async<R>();
    auto operation = std::make_shared<Operation>(
      std::in_place_type<T>, std::forward<Args>(args)..., async.GetEval());
    m_pending_results.Insert(&std::get<T>(*operation).m_result);
    if(!m_open_state.IsOpen()) {
      m_pending_results.Erase(&std::get<T>(*operation).m_result);
      BOOST_THROW_EXCEPTION(Beam::IO::EndOfFileException());
    }
    m_operations.Push(operation);
    auto result = std::move(async.Get());
    m_pending_results.Erase(&std::get<T>(*operation).m_result);
    return result;
  }
}

#endif
