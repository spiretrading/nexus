#ifndef SPIRE_ORDER_TASK_ARGUMENTS_HPP
#define SPIRE_ORDER_TASK_ARGUMENTS_HPP
#include <vector>
#include <boost/optional/optional.hpp>
#include <QKeySequence>
#include <QString>
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Tag.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /** Stores the arguments used to submit an order task. */
  struct OrderTaskArguments {

    /** The name of the order task. */
    QString m_name;

    /** The region this task applies to. */
    Nexus::Region m_region;

    /** The destination to submit the order to. */
    Nexus::Destination m_destination;

    /** The order type. */
    Nexus::OrderType m_order_type;

    /** A order's side. */
    Nexus::Side m_side;

    /** The order's quantity. */
    boost::optional<Nexus::Quantity> m_quantity;

    /** The order's time in force. */
    Nexus::TimeInForce m_time_in_force;

    /** The list of additional tags to apply to the order. */
    std::vector<Nexus::Tag> m_additional_tags;

    /** The order task's key binding. */
    QKeySequence m_key;
  };

  /** The type of model used for a list of OrderTaskArguments. */
  using OrderTaskArgumentsListModel = ListModel<OrderTaskArguments>;

  /**
   * Finds the <i>OrderTaskArguments</i> bound to a given <i>key</i> for a
   * specified <i>region</i>.
   * @param order_task_arguments The list of OrderTaskArguments to search.
   * @param region The region that the binding belongs to.
   * @param key The key binding to find.
   * @return The <i>OrderTaskArguments</i> with the specified <i>key</i> whose
   *         region field is a subset of the specified <i>region</i>.
   */
  boost::optional<const OrderTaskArguments&> find_order_task_arguments(
    const OrderTaskArgumentsListModel& order_task_arguments,
    const Nexus::Region& region, const QKeySequence& key);
}

#endif
