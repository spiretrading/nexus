#ifndef SPIRE_EXECUTION_REPORTS_TO_TABLE_MODEL_HPP
#define SPIRE_EXECUTION_REPORTS_TO_TABLE_MODEL_HPP
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /**
   * Flattens out the fields of a list of execution reports into a TableModel.
   */
  class ExecutionReportsToTableModel : public TableModel {
    public:

      /** Lists out the columns of the table. */
      enum Column {

        /** The timestamp field. */
        TIME,

        /** The order id the report belongs to. */
        ID,

        /** The side of the order the report belongs to. */
        SIDE,

        /** The security of the order the report belongs to. */
        SECURITY,

        /** The report's order status field. */
        ORDER_STATUS,

        /** The quantity of the order the report belongs to. */
        QUANTITY,

        /** The report's last quantity field. */
        LAST_QUANTITY,

        /** The price of the order the report belongs to. */
        PRICE,

        /** The report's last price field. */
        LAST_PRICE,

        /** The report's market field. */
        MARKET,

        /** The report's liquidity flag field. */
        LIQUIDITY_FLAG,

        /** The report's execution fee field. */
        EXECUTION_FEE,

        /** The report's processing fee field. */
        PROCESSING_FEE,

        /** The report's miscellaneous fee field. */
        MISCELLANEOUS_FEE,

        /** The report's message field. */
        MESSAGE
      };

      /** The number of columns in this table. */
      static const auto COLUMN_SIZE = 15;

      /**
       * Constructs an ExecutionReportsToTableModel populated by a list of
       * execution reports.
       * @param reports The list of execution reports used to populate this
       *        table.
       */
      explicit ExecutionReportsToTableModel(
        std::shared_ptr<ExecutionReportListModel> reports);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<ExecutionReportListModel> m_reports;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_connection;

      void on_operation(const ExecutionReportListModel::Operation& operation);
  };
}

#endif
