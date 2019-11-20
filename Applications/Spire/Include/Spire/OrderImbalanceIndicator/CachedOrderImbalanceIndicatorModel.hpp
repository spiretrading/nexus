#ifndef SPIRE_CACHED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_CACHED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include <unordered_set>
#include <boost/icl/continuous_interval.hpp>
#include <boost/icl/interval_set.hpp>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  //! Represents an OrderImbalanceModel that caches loaded order imbalances
  //! from a source model.
  class CachedOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      //! Constructs a CachedOrderImbalanceModel with a given source model.
      /*
        \param source_model The model supplying data to the cached model.
      */
      CachedOrderImbalanceIndicatorModel(
        std::shared_ptr<OrderImbalanceIndicatorModel> source_model);

      SubscriptionResult subscribe(
        const boost::posix_time::ptime& start,
        const boost::posix_time::ptime& end,
        const OrderImbalanceSignal::slot_type& slot) override;

    private:
      struct Subscription {
        OrderImbalanceSignal m_imbalance_signal;
        boost::posix_time::ptime m_start_time;
        boost::posix_time::ptime m_end_time;

        Subscription(const boost::posix_time::ptime& start,
          const boost::posix_time::ptime& end);
      };

      std::shared_ptr<OrderImbalanceIndicatorModel> m_source_model;
      std::vector<Nexus::OrderImbalance> m_imbalances;
      std::vector<Subscription> m_subscriptions;
      boost::icl::interval_set<boost::posix_time::ptime> m_ranges;
      std::vector<boost::signals2::scoped_connection> m_connections;

      OrderImbalanceIndicatorModel::SubscriptionResult
        get_subscription(const boost::posix_time::ptime& start,
        const boost::posix_time::ptime& end,
        const OrderImbalanceSignal::slot_type& slot);
      OrderImbalanceIndicatorModel::SubscriptionResult
        load_imbalances(const boost::posix_time::ptime& start,
        const boost::posix_time::ptime& end,
        const OrderImbalanceSignal::slot_type& slot);
      void on_order_imbalance(const Nexus::OrderImbalance& imbalance);
  };
}

#endif
