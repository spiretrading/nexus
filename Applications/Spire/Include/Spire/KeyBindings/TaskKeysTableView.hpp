#ifndef SPIRE_TASK_KEYS_TABLE_VIEW_HPP
#define SPIRE_TASK_KEYS_TABLE_VIEW_HPP
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** Enumerates the columns of the order task table. */
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

  /**
   * Returns a new EditableTableView for task keys.
   * @param order_task_table The table model of order task arguments.
   * @param region_query_model The model used to query region matches.
   * @param destinations The destination database to use.
   * @param markets The market database to use.
   * @param additional_tags Specifies the definition of all available additional
   *        tags.
   * @param parent The parent widget.
   */
  TableView* make_task_keys_table_view(
    std::shared_ptr<TableModel> order_task_table,
    std::shared_ptr<ComboBox::QueryModel> region_query_model,
    Nexus::DestinationDatabase destinations, Nexus::MarketDatabase markets,
    AdditionalTagDatabase additional_tags, QWidget* parent = nullptr);
}

#endif
