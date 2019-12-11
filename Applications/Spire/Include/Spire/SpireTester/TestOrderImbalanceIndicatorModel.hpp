#ifndef SPIRE_TEST_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_TEST_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include <mutex>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  //! Represents an OrderImbalanceIndicatorModel for testing, allowing for the
  //! observation of load requests sent to the model and simulation
  //! of asynchronous loads.
  class TestOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      //! Stores a request to load from an OrderImbalanceModel.
      class LoadEntry {
        public:

          //! Constructs a LoadEntry.
          /*
            \param interval The time interval to load.
          */
          LoadEntry(const TimeInterval& interval);

          //! Returns the load entry's requested time interval.
          const TimeInterval& get_interval() const;

          //! Sets the result of the load operation.
          /*
            \param result The list of imbalances that the promise loading the
                    specified range should evalute to.
          */
          void set_result(std::vector<Nexus::OrderImbalance> result);

          //! Returns the order imbalances to load.
          std::vector<Nexus::OrderImbalance>& get_result();

        private:
          friend class TestOrderImbalanceIndicatorModel;
          mutable Beam::Threading::Mutex m_mutex;
          TimeInterval m_interval;
          bool m_is_loaded;
          std::vector<Nexus::OrderImbalance> m_result;
          Beam::Threading::ConditionVariable m_load_condition;
      };

      QtPromise<std::vector<Nexus::OrderImbalance>> load(
        const TimeInterval& interval) override;

      QtPromise<std::vector<Nexus::OrderImbalance>> load(
        const Nexus::Security& security,
        const TimeInterval& interval) override;

      SubscriptionResult<boost::optional<Nexus::OrderImbalance>>
        subscribe(const OrderImbalanceSignal::slot_type& slot) override;

      std::shared_ptr<OrderImbalanceChartModel> get_chart_model(
        const Nexus::Security& security) override;

      //! Pops the oldest load request from this model's load
      //! operation stack.
      QtPromise<std::shared_ptr<LoadEntry>> pop_load();

      //! Returns the number of pending load entries.
      int get_load_entry_count() const;

    private:
      Beam::Threading::Mutex m_mutex;
      Beam::Threading::ConditionVariable m_load_condition;
      std::deque<std::shared_ptr<LoadEntry>> m_load_entries;
  };
}

#endif
