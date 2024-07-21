#ifndef SPIRE_TABLE_TO_LIST_MODEL_HPP
#define SPIRE_TABLE_TO_LIST_MODEL_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /** Provides a view into a TableModel's row. */
  class RowView {
    public:

      /**
       * Returns a const reference to a column in this row.
       * @param column The column to access.
       */
      AnyRef get(int column) const;

      /**
       * Sets a column in this row to a given value.
       * @param column The column to set.
       * @param value The value to set the <i>column</i> to.
       */
      QValidator::State set(int column, const std::any& value) const;

    private:
      friend class TableToListModel;
      TableModel* m_table;
      int m_row;

      RowView(TableModel& table, int row);
  };

  /** Provides a list view over a table model. */
  class TableToListModel : public ListModel<RowView> {
    public:

      /**
       * Constructs a TableToListModel.
       * @param table The table to view as a list.
       */
      explicit TableToListModel(std::shared_ptr<TableModel> table);

      const TableModel::UpdateOperation& get_update() const;

      int get_size() const override;

      const Type& get(int index) const override;

      QValidator::State set(int index, const Type& value) override;

      QValidator::State push(const Type& value) override;

      QValidator::State insert(const Type& value, int index) override;

      QValidator::State move(int source, int destination) override;

      QValidator::State remove(int index) override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      std::shared_ptr<TableModel> m_table;
      boost::signals2::scoped_connection m_connection;
      ListModelTransactionLog<RowView> m_transaction;
      mutable boost::optional<RowView> m_last;
      const TableModel::UpdateOperation* m_update;

      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
