#ifndef SPIRE_COLUMN_VIEW_LIST_MODEL_HPP
#define SPIRE_COLUMN_VIEW_LIST_MODEL_HPP
#include <functional>
#include <memory>
#include <stdexcept>
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /**
   * Implements a ListModel that provides a view into a single column of
   * a source table model.
   */
  template<typename T>
  class ColumnViewListModel : public ListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;

      using OperationSignal = ListModel<T>::OperationSignal;

      using AddOperation = typename ListModel<T>::AddOperation;

      using MoveOperation = typename ListModel<T>::MoveOperation;

      using RemoveOperation = typename ListModel<T>::RemoveOperation;

      using UpdateOperation = typename ListModel<T>::UpdateOperation;

      /**
       * Constructs a ColumnViewListModel from a specified column of
       * the table model.
       * @param source The table model.
       * @param column The index of the column to be viewed. 
       */
      ColumnViewListModel(std::shared_ptr<TableModel> source, int column);

      int get_size() const override;

      const Type& get(int index) const override;

      QValidator::State set(int index, const Type& value) override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

      using ListModel<T>::transact;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      std::shared_ptr<TableModel> m_source;
      int m_column;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      void on_operation(const TableModel::Operation& operation);
  };

  template<typename T>
  ColumnViewListModel<T>::ColumnViewListModel(
      std::shared_ptr<TableModel> source, int column)
      : m_source(std::move(source)) {
    if(column < 0 || column >= m_source->get_column_size()) {
      m_column = -1;
      m_source = nullptr;
    } else {
      m_column = column;
      m_source_connection = m_source->connect_operation_signal(
        std::bind_front(&ColumnViewListModel::on_operation, this));
    }
  }

  template<typename T>
  int ColumnViewListModel<T>::get_size() const {
    if(m_column == -1) {
      return 0;
    }
    return m_source->get_row_size();
  }

  template<typename T>
  const typename ColumnViewListModel<T>::Type& ColumnViewListModel<T>::get(
      int index) const {
    if(m_column == -1) {
      throw std::out_of_range("The column is out of range.");
    }
    return m_source->get<Type>(index, m_column);
  }

  template<typename T>
  QValidator::State ColumnViewListModel<T>::set(int index, const Type& value) {
    if(m_column == -1) {
      return QValidator::State::Invalid;
    }
    return m_source->set(index, m_column, value);
  }

  template<typename T>
  boost::signals2::connection ColumnViewListModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void ColumnViewListModel<T>::transact(
      const std::function<void ()>& transaction) {
    m_transaction.transact(transaction);
  }

  template<typename T>
  void ColumnViewListModel<T>::on_operation(
      const TableModel::Operation& operation) {
    m_transaction.transact([&] {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          m_transaction.push(AddOperation(operation.m_index,
            std::any_cast<const Type&>(operation.m_row->get(m_column))));
        },
        [&] (const TableModel::MoveOperation& operation) {
          m_transaction.push(
            MoveOperation(operation.m_source, operation.m_destination));
        },
        [&] (const TableModel::RemoveOperation& operation) {
          m_transaction.push(RemoveOperation(operation.m_index,
            std::any_cast<const Type&>(operation.m_row->get(m_column))));
        },
        [&] (const TableModel::UpdateOperation& operation) {
          if(m_column == operation.m_column) {
            m_transaction.push(UpdateOperation(operation.m_row,
              std::any_cast<const Type&>(operation.m_previous),
              std::any_cast<const Type&>(operation.m_value)));
          }
        });
      });
  }
}

#endif
