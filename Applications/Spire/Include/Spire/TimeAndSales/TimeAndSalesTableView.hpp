#ifndef SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#define SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {
namespace Styles {

  /** Selects components with the UNKNOWN indicator. */
  using UnknownIndicator =
    StateSelector<void, struct UnknownIndicatorSeletorTag>;

  /** Selects components with the ABOVE_ASK indicator. */
  using AboveAskIndicator =
    StateSelector<void, struct AboveAskIndicatorSeletorTag>;

  /** Selects components with the AT_ASK indicator. */
  using AtAskIndicator = StateSelector<void, struct AtAskIndicatorSeletorTag>;

  /** Selects components with the INSIDE indicator. */
  using InsideIndicator = StateSelector<void, struct InsideIndicatorSeletorTag>;

  /** Selects components with the AT_BID indicator. */
  using AtBidIndicator = StateSelector<void, struct AtBidIndicatorSeletorTag>;

  /** Selects components with the BELOW_BID indicator. */
  using BelowBidIndicator =
    StateSelector<void, struct BelowBidIndicatorSeletorTag>;
}

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
