#ifndef SPIRE_SUBSCRIPTION_RESULT_HPP
#define SPIRE_SUBSCRIPTION_RESULT_HPP
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Represents a data subscription, storing the subscription's connection and
   * a snapshot used to initialize the data feed.
   */
  template<typename T>
  struct SubscriptionResult {

    /** The connection to the subscription. */
    boost::signals2::connection m_connection;

    /** The data feed's initial snapshot. */
    QtPromise<T> m_snapshot;
  };
}

#endif
