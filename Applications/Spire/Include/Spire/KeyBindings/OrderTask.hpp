#ifndef SPIRE_ORDER_TASK_HPP
#define SPIRE_ORDER_TASK_HPP
#include <QKeySequence>
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"

namespace Spire {

  /** The order task with a keybinding. */
  struct OrderTask {

    /** The name of the order task. */
    QString m_name;

    /** The region this order task is avaialbe on. */
    Nexus::Region m_region;

    /** The destination this order task is avaialbe on. */
    Nexus::Destination m_destination;

    /** The order type. */
    Nexus::OrderType m_order_type;

    /** A bid or an ask for the order task. */
    Nexus::Side m_side;

    /** The quantity of the order task. */
    boost::optional<Nexus::Quantity> m_quantity;

    /** The expire of the order task. */
    Nexus::TimeInForce m_time_in_force;

    /** The keybinding of the order task. */
    QKeySequence m_key;
  };
}

#endif
