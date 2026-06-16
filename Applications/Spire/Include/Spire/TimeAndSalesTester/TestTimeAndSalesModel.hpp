#ifndef SPIRE_TEST_TIME_AND_SALES_MODEL_HPP
#define SPIRE_TEST_TIME_AND_SALES_MODEL_HPP
#include <deque>
#include "Spire/Async/QtFuture.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /** Implements a TimeAndSalesModel for testing purposes. */
  class TestTimeAndSalesModel : public TimeAndSalesModel {
    public:

      /** Stores the arguments and return value to a query_until call. */
      struct QueryRequest {

        /** The sequence the query stops at. */
        Beam::Sequence m_sequence;

        /** The maximum number of entries requested. */
        int m_max_count;

        /** Resolves the promise returned to the caller of query_until. */
        QtFuture<std::vector<Entry>> m_result;
      };

      /**
       * Publishes a time and sale to the update signal's subscribers.
       * @param entry The entry to publish.
       */
      void publish(const Entry& entry);

      /** Returns the query_until calls still awaiting a result. */
      const std::deque<QueryRequest>& get_query_requests() const;

      /** Removes and returns the oldest query_until call awaiting a result. */
      QueryRequest pop_query_request();

      QtPromise<std::vector<Entry>> query_until(
        Beam::Sequence sequence, int max_count) override;
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      std::deque<QueryRequest> m_query_requests;
  };
}

#endif
