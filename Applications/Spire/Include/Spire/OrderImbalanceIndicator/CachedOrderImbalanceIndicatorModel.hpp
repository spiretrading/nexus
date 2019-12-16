#ifndef SPIRE_CACHED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_CACHED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include <boost/icl/interval_set.hpp>
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  //! Represents an OrderImbalanceModel that caches loaded order imbalances
  //! from a source model.
  class CachedOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      //! Constructs a CachedOrderImbalanceIndicatorModel that loads from
      //! the given source model.
      /*
        \param source The model supplying data to the cached model.
      */
      CachedOrderImbalanceIndicatorModel(
        std::shared_ptr<OrderImbalanceIndicatorModel> source);

      QtPromise<std::vector<Nexus::OrderImbalance>> load(
        const TimeInterval& interval) override;

      QtPromise<std::vector<Nexus::OrderImbalance>> load(
        const Nexus::Security& security,
        const TimeInterval& interval) override;

      SubscriptionResult<boost::optional<Nexus::OrderImbalance>>
        subscribe(const OrderImbalanceSignal::slot_type& slot) override;

    private:
      std::shared_ptr<OrderImbalanceIndicatorModel> m_source_model;
      LocalOrderImbalanceIndicatorModel m_cache;
      boost::icl::interval_set<boost::posix_time::ptime> m_intervals;
      std::map<Nexus::Security,
        boost::icl::interval_set<boost::posix_time::ptime>>
        m_security_intervals;
      boost::signals2::scoped_connection m_subscription_connection;
      QtPromise<boost::optional<Nexus::OrderImbalance>> m_subscription_promise;

      QtPromise<void> load_from_model(const TimeInterval& interval);
      QtPromise<void> load_from_model(const Nexus::Security& security,
        const TimeInterval& interval);
      void on_imbalance_published(const Nexus::OrderImbalance& imbalance);
      void on_imbalances_loaded(const TimeInterval& interval,
        const std::vector<Nexus::OrderImbalance>& imbalances);
      void on_imbalances_loaded(const Nexus::Security& security,
        const TimeInterval& interval,
        const std::vector<Nexus::OrderImbalance>& imbalances);
  };
}

#endif
