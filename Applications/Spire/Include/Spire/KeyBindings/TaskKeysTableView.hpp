#ifndef SPIRE_TASK_KEYS_TABLE_VIEW_HPP
#define SPIRE_TASK_KEYS_TABLE_VIEW_HPP
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Ui/ComboBox.hpp"

namespace Spire {

  /**
   * Returns a new EditableTableView for task keys.
   * @param order_task_arguments The list of order task arguments.
   * @param region_query_model The model used to query region matches.
   * @param destinations The destination database to use.
   * @param markets The market database to use.
   * @param parent The parent widget.
   */
  TableView* make_task_keys_table_view(
    std::shared_ptr<OrderTaskArgumentsListModel> order_task_arguments,
    std::shared_ptr<ComboBox::QueryModel> region_query_model,
    Nexus::DestinationDatabase destinations, Nexus::MarketDatabase markets,
    QWidget* parent = nullptr);
}

#endif
