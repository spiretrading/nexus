#ifndef SPIRE_TIME_AND_SALES_TO_TABLE_MODEL_HPP
#define SPIRE_TIME_AND_SALES_TO_TABLE_MODEL_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /**
   * Implements an TimeAndSalesToTableModel that converts a list of
   * time and sales to a TableModel.
   */
  class TimeAndSalesToTableModel : public TableModel {
    public:

      using Entry = TimeAndSalesModel::Entry;

      /* The available columns to display. */
      enum class Column {

        /** The time column. */
        TIME,

        /** The price column. */
        PRICE,

        /** The size column. */
        SIZE,

        /** The market column. */
        MARKET,

        /** The sales condition column. */
        CONDITION
      };

      /** The number of columns in this table. */
      static const auto COLUMN_SIZE = 5;

      /**
       * Constructs an TimeAndSalesToTableModel.
       * @param source A list of OrderTask.
       */
      explicit TimeAndSalesToTableModel(
        std::shared_ptr<ListModel<Entry>> source);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<ListModel<Entry>> m_source;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      AnyRef extract_field(const Entry& entry, Column column) const;
      void on_operation(const ListModel<Entry>::Operation& operation);
  };
}

#endif
