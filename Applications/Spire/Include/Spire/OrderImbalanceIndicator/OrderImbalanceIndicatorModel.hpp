#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/Spire/Intervals.hpp"
#include "Spire/Spire/SubscriptionResult.hpp"

namespace Spire {

  //! Model used to publish order imbalances.
  class OrderImbalanceIndicatorModel : private boost::noncopyable {
    public:

      //! Signals that an order imbalance has been published.
      /*!
        \param imbalance The published imbalance.
      */
      using OrderImbalanceSignal =
        Signal<void (const Nexus::OrderImbalance& imbalance)>;

      virtual ~OrderImbalanceIndicatorModel() = default;

      //! Loads a list of historical order imbalances.
      /*
        \param interval The time interval to load.
        \return The list of order imbalances within the specified interval.
      */
      virtual QtPromise<std::vector<Nexus::OrderImbalance>> load(
        const TimeInterval& interval) = 0;

      //! Subscribes to real time order imbalances.
      /*!
        \param slot The slot receiving new order imbalances.
        \return A SubscriptionResult containing the last published imbalance.
      */
      virtual SubscriptionResult<boost::optional<Nexus::OrderImbalance>>
        subscribe(const OrderImbalanceSignal::slot_type& slot) = 0;

      //! Subscribes to all order imbalances published on or after a start time.
      /*!
        \param start The start time to load order imbalances.
        \param slot The slot receiving new order imbalances.
        \return A SubscriptionResult containing the last published imbalance.
      */
      virtual SubscriptionResult<std::vector<Nexus::OrderImbalance>> subscribe(
        const TimeBound& start, OrderImbalanceSignal::slot_type& slot);

    protected:
      OrderImbalanceIndicatorModel() = default;
  };
}

#endif
