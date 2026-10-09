#ifndef SPIRE_TEST_TIME_AND_SALES_MODEL_HPP
#define SPIRE_TEST_TIME_AND_SALES_MODEL_HPP
#include <deque>
#include <vector>
#include "Spire/Async/QtFuture.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /** Implements a TimeAndSalesModel for testing purposes. */
  class TestTimeAndSalesModel : public TimeAndSalesModel {
    public:

      /** Stores the arguments and return value to a load_older call. */
      struct LoadRequest {

        /** The maximum number of entries requested. */
        int m_max_count;

        /**
         * Resolves the load with the entries to insert at the front, ordered
         * from oldest to newest.
         */
        QtFuture<std::vector<Details::TimeAndSalesEntry>> m_result;
      };

      /**
       * Appends an entry.
       * @param entry The entry to append.
       */
      void publish(const Details::TimeAndSalesEntry& entry);

      /** Returns the load_older calls still awaiting a result. */
      const std::deque<LoadRequest>& get_requests() const;

      /** Removes and returns the oldest load_older call awaiting a result. */
      LoadRequest pop_request();

      QtPromise<void> load_older(int max_count) override;
      int get_size() const override;
      const Type& get(int index) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      ArrayListModel<Details::TimeAndSalesEntry> m_entries;
      std::deque<LoadRequest> m_requests;
  };
}

#endif
