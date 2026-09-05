#ifndef SPIRE_DEDUPLICATED_TABLE_MODEL_HPP
#define SPIRE_DEDUPLICATED_TABLE_MODEL_HPP
#include <any>
#include <functional>
#include <memory>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /**
   * Implements a TableModel that signals an update only when a cell's value
   * changes.
   */
  class DeduplicatedTableModel : public TableModel {
    public:

      /**
       * The type of function used to test two values of a column for equality.
       * @param previous The previous value.
       * @param value The updated value.
       * @param column The column that both values belong to.
       * @return <code>true</code> iff the values are equal.
       */
      using Comparator = std::function<
        bool (const std::any& previous, const std::any& value, int column)>;

      /**
       * Constructs a DeduplicatedTableModel testing values for equality using
       * <i>is_equal</i>.
       * @param source The table to view.
       */
      explicit DeduplicatedTableModel(std::shared_ptr<TableModel> source);

      /**
       * Constructs a DeduplicatedTableModel.
       * @param source The table to view.
       * @param comparator The function used to test values for equality.
       */
      DeduplicatedTableModel(
        std::shared_ptr<TableModel> source, Comparator comparator);

      int get_row_size() const override;
      int get_column_size() const override;
      AnyRef at(int row, int column) const override;
      QValidator::State
        set(int row, int column, const std::any& value) override;
      QValidator::State remove(int row) override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      Comparator m_comparator;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_connection;

      void on_operation(const Operation& operation);
  };
}

#endif
