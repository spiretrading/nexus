#ifndef SPIRE_PORTFOLIO_TO_POSITIONS_TABLE_MODEL_HPP
#define SPIRE_PORTFOLIO_TO_POSITIONS_TABLE_MODEL_HPP
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /**
   * Transforms the positions in a PortfolioModel into a TableModel where each
   * position is represented as a row.
   */
  class PortfolioToPositionsTableModel : public TableModel {
    public:

      /** Lists out the columns of the table. */
      enum Column {

        /** The positions security. */
        SECURITY,

        /** The size of the position. */
        QUANTITY,

        /** The position's side. */
        SIDE,

        /** The profit and loss. */
        PROFIT_AND_LOSS,

        /** The average price of the position. */
        AVERAGE_PRICE,

        /** The position's cost basis. */
        COST_BASIS,

        /** The position's currency. */
        CURRENCY
      };

      /** The number of columns in this table. */
      static const auto COLUMN_SIZE = 7;

      /**
       * Constructs a PortfolioToPositionsTableModel.
       * @param portfolio The portfolio whose positions are to be represented by
       *        the table.
       */
      explicit PortfolioToPositionsTableModel(
        std::shared_ptr<PortfolioModel> portfolio);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;
  };
}

#endif
