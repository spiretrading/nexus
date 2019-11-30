#ifndef SPIRE_ORDER_IMBALANCE_CHART_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_CHART_MODEL_HPP
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/Spire/SubscriptionResult.hpp"

namespace Spire {

  //! Model used to publish order imbalances for a specific security.
  class OrderImbalanceChartModel : private boost::noncopyable {
    public:

      //! Signals an update to the security's price.
      /*!
        \param price The security's current price.
      */
      using PriceSignal = Signal<void (Nexus::Money price)>;

      //! Signals an order imbalance.
      /*!
        \param imbalance The published order imbalance.
      */
      using OrderImbalanceSignal = Signal<
        void (const Nexus::OrderImbalance imbalance)>;

      virtual ~OrderImbalanceChartModel() = default;

      //! Subscribes to the security's price feed.
      /*!
        \param slot The slot to call when a new price is published.
      */
      virtual SubscriptionResult<boost::optional<Nexus::Money>> subscribe_price(
        const PriceSignal::slot_type& slot) = 0;

    protected:
      OrderImbalanceChartModel() = default;
  };
}

#endif
