#ifndef SPIRE_CACHED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_CACHED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include <boost/icl/interval_set.hpp>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  class CachedOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      //! Represents an OrderImbalanceModel that caches loaded order imbalances
      //! from a source model.
      /*
        \param source The model supplying data to the cached model.
      */
      CachedOrderImbalanceIndicatorModel(
        std::shared_ptr<OrderImbalanceIndicatorModel> source);

      QtPromise<std::vector<Nexus::OrderImbalance>> load(
        const TimeInterval& interval) override;

      SubscriptionResult<boost::optional<Nexus::OrderImbalance>>
        subscribe(const OrderImbalanceSignal::slot_type& slot) override;

      std::shared_ptr<OrderImbalanceChartModel> get_chart_model(
        const Nexus::Security& security) override;

    private:
      std::shared_ptr<OrderImbalanceIndicatorModel> m_source_model;
      std::vector<Nexus::OrderImbalance> m_imbalances;
      boost::icl::interval_set<boost::posix_time::ptime> m_ranges;
      boost::signals2::scoped_connection m_subscription_connection;

      QtPromise<std::vector<Nexus::OrderImbalance>> load_from_cache(
        const TimeInterval& interval);
      QtPromise<std::vector<Nexus::OrderImbalance>> load_from_model(
        const TimeInterval& interval);
      void on_imbalance_published(const Nexus::OrderImbalance& imbalance);
  };
}

#endif
