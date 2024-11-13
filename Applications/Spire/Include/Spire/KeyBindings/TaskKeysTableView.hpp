#ifndef SPIRE_TASK_KEYS_TABLE_VIEW_HPP
#define SPIRE_TASK_KEYS_TABLE_VIEW_HPP
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /**
   * Returns a new EditableTableView for task keys.
   * @param order_task_arguments The list of order task arguments to view.
   * @param regions The Regions available to query.
   * @param destinations The destination database to use.
   * @param markets The market database to use.
   * @param additional_tags Specifies the definition of all available additional
   *        tags.
   * @param parent The parent widget.
   */
  TableView* make_task_keys_table_view(
    std::shared_ptr<OrderTaskArgumentsListModel> order_task_arguments,
    std::shared_ptr<RegionQueryModel> regions,
    Nexus::DestinationDatabase destinations, Nexus::MarketDatabase markets,
    AdditionalTagDatabase additional_tags, QWidget* parent = nullptr);
}

#endif
