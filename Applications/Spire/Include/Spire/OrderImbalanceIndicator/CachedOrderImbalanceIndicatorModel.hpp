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

      std::tuple<boost::signals2::connection,
        QtPromise<std::vector<Nexus::OrderImbalance>>>
        subscribe(const boost::posix_time::ptime& start,
        const boost::posix_time::ptime& end,
        const OrderImbalanceSignal::slot_type& slot) override;

    private:

      struct OrderImbalanceHash {
        std::size_t operator ()(const Nexus::OrderImbalance& imbalance) const;
      };

      struct OrderImbalanceSignalConnection {
        OrderImbalanceSignal m_imbalance_signal;
        boost::posix_time::ptime m_start_time;
        boost::posix_time::ptime m_end_time;
      };

      std::shared_ptr<OrderImbalanceIndicatorModel> m_source_model;
      std::unordered_set<Nexus::OrderImbalance, OrderImbalanceHash>
        m_imbalances;
      std::vector<OrderImbalanceSignalConnection> m_signals;
      boost::icl::interval_set<boost::posix_time::ptime> m_ranges;
      std::vector<boost::signals2::scoped_connection> m_connections;

      std::tuple<boost::signals2::connection,
      QtPromise<std::vector<Nexus::OrderImbalance>>>
      get_subscription(const boost::posix_time::ptime& start,
        const boost::posix_time::ptime& end,
        const OrderImbalanceSignal::slot_type& slot);
      std::tuple<boost::signals2::connection,
      QtPromise<std::vector<Nexus::OrderImbalance>>>
      load_imbalances(const boost::posix_time::ptime& start,
        const boost::posix_time::ptime& end,
        const OrderImbalanceSignal::slot_type& slot);
      void on_order_imbalance(const Nexus::OrderImbalance& imbalance);
  };
}

#endif
