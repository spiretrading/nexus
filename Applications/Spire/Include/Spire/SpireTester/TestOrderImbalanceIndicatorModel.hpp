#ifndef SPIRE_TEST_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_TEST_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  //! Represents an OrderImbalanceIndicatorModel for testing, allowing for the
  //! observation of subscription requests sent to the model and simulation
  //! of asynchronous loads.
  class TestOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      //! Stores a request to subscribe to an OrderImbalanceModel.
      class SubscriptionEntry {
        public:

          //! Constructs a SubscriptionEntry.
          /*
            \param start The start time to begin loading from.
            \param end The end time to load until.
          */
          SubscriptionEntry(const boost::posix_time::ptime& start,
            const boost::posix_time::ptime& end);

          //! Returns the start time of the range to load.
          const boost::posix_time::ptime& get_start() const;

          //! Returns the end time of the range to load.
          const boost::posix_time::ptime& get_end() const;

          //! Sets the result of the subscribe operation.
          /*
            \param result The list of imbalances that the promise loading the
                   specified range should evalute to.
          */
          void set_result(std::vector<Nexus::OrderImbalance> result);

          //! Returns the order imbalance to load.
          std::vector<Nexus::OrderImbalance>& get_result();

        private:
          friend class TestOrderImbalanceIndicatorModel;
          mutable Beam::Threading::Mutex m_mutex;
          boost::posix_time::ptime m_start;
          boost::posix_time::ptime m_end;
          bool m_is_loaded;
          std::vector<Nexus::OrderImbalance> m_result;
          Beam::Threading::ConditionVariable m_subscription_condition;
      };

      OrderImbalanceIndicatorModel::SubscriptionResult
        subscribe(const boost::posix_time::ptime& start,
        const boost::posix_time::ptime& end,
        const OrderImbalanceSignal::slot_type& slot) override;

      //! Pops the oldest subscribe request from this model's subcribe
      //! operation stack.
      QtPromise<std::shared_ptr<SubscriptionEntry>> pop_subscription();

      //! Returns the number of pending subscribe entries.
      int get_subscription_entry_count() const;

    private:
      Beam::Threading::Mutex m_mutex;
      Beam::Threading::ConditionVariable m_subscription_condition;
      std::deque<std::shared_ptr<SubscriptionEntry>> m_subscription_entries;
  };
}

#endif
