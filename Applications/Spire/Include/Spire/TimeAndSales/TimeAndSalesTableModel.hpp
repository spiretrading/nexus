#ifndef SPIRE_TIME_AND_SALES_TO_TABLE_MODEL_HPP
#define SPIRE_TIME_AND_SALES_TO_TABLE_MODEL_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /**
   * Implements an TimeAndSalesTableModel that converts a list of
   * time and sales to a TableModel.
   */
  class TimeAndSalesTableModel : public TableModel {
    public:

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
       * Constructs an TimeAndSalesTableModel.
       * @param source A list of time and sales.
       */
      explicit TimeAndSalesTableModel(
        std::shared_ptr<ListModel<Nexus::TimeAndSale>> source);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<ListModel<Nexus::TimeAndSale>> m_source;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      AnyRef extract_field(const Nexus::TimeAndSale& time_and_sale, Column column) const;
      void on_operation(const ListModel<Nexus::TimeAndSale>::Operation& operation);
  };
}

#endif
