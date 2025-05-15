#ifndef SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#define SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /**
   * Returns a new TableView for the time and sales.
   * @param table The table model of time and sales.
   * @param properties The table's display properties.
   * @param parent The parent widget.
   */
  TableView* make_time_and_sales_table_view(
    std::shared_ptr<TimeAndSalesTableModel> table,
    std::shared_ptr<TimeAndSalesPropertiesModel> properties,
    QWidget* parent = nullptr);
}

#endif
