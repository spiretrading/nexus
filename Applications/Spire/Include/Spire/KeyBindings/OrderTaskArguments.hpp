#ifndef SPIRE_ORDER_TASK_ARGUMENTS_HPP
#define SPIRE_ORDER_TASK_ARGUMENTS_HPP
#include <vector>
#include <boost/optional/optional.hpp>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <QKeySequence>
#include <QString>
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Tag.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"

namespace Spire {

  enum class QuantitySetting {

    /** The order quantity is fixed to the default. */
    DEFAULT,

    /** The order quantity is set at submission time. */
    ADJUSTABLE
  };

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
    QuantitySetting m_quantity;

    /** The order's time in force. */
    Nexus::TimeInForce m_time_in_force;

    /** The list of additional tags to apply to the order. */
    std::vector<AdditionalTag> m_additional_tags;

    /** The order task's key binding. */
    QKeySequence m_key;
  };

  /** The type of model used for a list of OrderTaskArguments. */
  using OrderTaskArgumentsListModel = ListModel<OrderTaskArguments>;

  /**
   * Finds the <i>OrderTaskArguments</i> bound to a given <i>key</i> for a
   * specified <i>region</i>.
   * @param arguments The list of OrderTaskArguments to search.
   * @param region The region that the binding belongs to.
   * @param key The key binding to find.
   * @return The <i>OrderTaskArguments</i> with the specified <i>key</i> whose
   *         region field is a subset of the specified <i>region</i>.
   */
  boost::optional<const OrderTaskArguments&> find_order_task_arguments(
    const OrderTaskArgumentsListModel& arguments, const Nexus::Region& region,
    const QKeySequence& key);

  /**
   * Constructs a <i>CanvasNode</i> representing an order task with a specified
   * set of arguments.
   * @param arguments The arguments used to build the <i>CanvasNode<i>.
   * @param additional_tags The database of additional tags.
   */
  std::unique_ptr<CanvasNode>
    make_canvas_node(const OrderTaskArguments& arguments,
      const AdditionalTagDatabase& additional_tags);

  /**
   * Converts an OrderTaskNode into an OrderTaskArguments record.
   * @param node The node to convert, should be a SingleOrderTaskNode.
   * @param markets The database of markets used to set specify the region.
   * @param destinations The database of destinations used to identify the
   *        market.
   * @return The OrderTaskArguments represented by the <i>node</i>.
   */
  OrderTaskArguments to_order_task_arguments(const CanvasNode& node,
    const Nexus::MarketDatabase& markets,
    const Nexus::DestinationDatabase& destinations);

  /** Returns the text representation of a QuantitySetting. */
  const QString& to_text(QuantitySetting setting);
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Spire::OrderTaskArguments> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Spire::OrderTaskArguments& value,
        unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("region", value.m_region);
      shuttle.Shuttle("destination", value.m_destination);
      shuttle.Shuttle("order_type", value.m_order_type);
      shuttle.Shuttle("side", value.m_side);
      shuttle.Shuttle("quantity", value.m_quantity);
      shuttle.Shuttle("time_in_force", value.m_time_in_force);
      shuttle.Shuttle("additional_tags", value.m_additional_tags);
      shuttle.Shuttle("key", value.m_key);
    }
  };
}

#endif
