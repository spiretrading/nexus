#ifndef SPIRE_ORDER_TASK_ARGUMENTS_TABLE_MODEL_HPP
#define SPIRE_ORDER_TASK_ARGUMENTS_TABLE_MODEL_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /** Enumerates the columns of the OrderTaskArgumentsListToTableModel. */
  enum class OrderTaskColumns {

    /** The name column. */
    NAME,

    /** The region column. */
    REGION,

    /** The destination column. */
    DESTINATION,

    /** The order type column. */
    ORDER_TYPE,

    /** The side column. */
    SIDE,

    /** The quantity column. */
    QUANTITY,

    /** The time in force column. */
    TIME_IN_FORCE,

    /** The additional tags column. */
    TAGS,

    /** The key binding column. */
    KEY
  };

  /** Makes a TableModel as a view over a ListModel<OrderTaskArguments>. */
  std::shared_ptr<TableModel> make_order_task_arguments_table_model(
    std::shared_ptr<OrderTaskArgumentsListModel> order_task_arguments);
}

#endif
