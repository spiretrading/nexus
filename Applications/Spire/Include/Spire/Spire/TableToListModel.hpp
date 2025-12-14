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

  /**
   * Converts a TableModel::StartTransaction into its equivalent
   * ListModel::StartTransaction.
   */
  template<typename T>
  typename ListModel<T>::StartTransaction to_list_operation(
      const TableModel::StartTransaction& operation) {
    return typename ListModel<T>::StartTransaction();
  }

  /**
   * Converts a TableModel::EndTransaction into its equivalent
   * ListModel::EndTransaction.
   */
  template<typename T>
  typename ListModel<T>::EndTransaction to_list_operation(
      const TableModel::EndTransaction& operation) {
    return typename ListModel<T>::EndTransaction();
  }

  /**
   * Converts a TableModel::AddOperation into its equivalent
   * ListModel::AddOperation.
   */
  template<typename T>
  typename ListModel<T>::AddOperation to_list_operation(
      const TableModel::AddOperation& operation) {
    return typename ListModel<T>::AddOperation(operation.m_index);
  }

  /**
   * Converts a TableModel::MoveOperation into its equivalent
   * ListModel::MoveOperation.
   */
  template<typename T>
  typename ListModel<T>::MoveOperation to_list_operation(
      const TableModel::MoveOperation& operation) {
    return typename ListModel<T>::MoveOperation(
      operation.m_source, operation.m_destination);
  }

  /**
   * Converts a TableModel::PreRemoveOperation into its equivalent
   * ListModel::PreRemoveOperation.
   */
  template<typename T>
  typename ListModel<T>::PreRemoveOperation to_list_operation(
      const TableModel::PreRemoveOperation& operation) {
    return typename ListModel<T>::PreRemoveOperation(operation.m_index);
  }

  /**
   * Converts a TableModel::RemoveOperation into its equivalent
   * ListModel::RemoveOperation.
   */
  template<typename T>
  typename ListModel<T>::RemoveOperation to_list_operation(
      const TableModel::RemoveOperation& operation) {
    return typename ListModel<T>::RemoveOperation(operation.m_index);
  }

  /**
   * Converts a TableModel::Operation into its equivalent ListModel::Operation.
   */
  template<typename T>
  typename ListModel<T>::Operation to_list_operation(
      const TableModel::Operation& operation) {
    if(auto start = boost::get<TableModel::StartTransaction>(&operation)) {
      return to_list_operation<T>(*start);
    } else if(auto end = boost::get<TableModel::EndTransaction>(&operation)) {
      return to_list_operation<T>(*end);
    } else if(auto add = boost::get<TableModel::AddOperation>(&operation)) {
      return to_list_operation<T>(*add);
    } else if(auto move = boost::get<TableModel::MoveOperation>(&operation)) {
      return to_list_operation<T>(*move);
    } else if(auto pre_remove =
        boost::get<TableModel::PreRemoveOperation>(&operation)) {
      return to_list_operation<T>(*pre_remove);
    } else if(auto remove =
        boost::get<TableModel::RemoveOperation>(&operation)) {
      return to_list_operation<T>(*remove);
    }
    return typename ListModel<T>::Operation();
  }
}

#endif
