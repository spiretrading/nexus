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

          //! Constructs a general LoadEntry.
          /*
            \param interval The time interval to load.
          */
          LoadEntry(const TimeInterval& interval);

          //! Constructs a LoadEntry for a specific Security.
          /*
            \param security The security whose order imbalances will be loaded.
            \param interval The time interval to load.
          */
          LoadEntry(const Nexus::Security security,
            const TimeInterval& interval);

          //! Returns the load entry's requested time interval.
          const TimeInterval& get_interval() const;

          //! Returns the security, if the original request was for a single
          //! security.
          const std::optional<Nexus::Security>& get_security() const;

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
          std::optional<Nexus::Security> m_security;
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

      //! Pops the oldest load request from this model's load
      //! operation stack.
      QtPromise<std::shared_ptr<LoadEntry>> pop_load();

      //! Returns the number of pending load entries.
      int get_load_entry_count() const;

    private:
      Beam::Threading::Mutex m_mutex;
      Beam::Threading::ConditionVariable m_load_condition;
      std::deque<std::shared_ptr<LoadEntry>> m_load_entries;

      QtPromise<std::vector<Nexus::OrderImbalance>> add_load_entry(
        std::shared_ptr<LoadEntry> load_entry);
  };
}

#endif
