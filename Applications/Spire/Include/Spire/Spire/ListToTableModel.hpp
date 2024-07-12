#ifndef SPIRE_LIST_TO_TABLE_MODEL_HPP
#define SPIRE_LIST_TO_TABLE_MODEL_HPP
#include <functional>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /**
   * Provides a table view over a ListModel.
   * @param <T> The type of list elements being viewed.
   */
  template<typename T>
  class ListToTableModel : public TableModel {
    public:

      /** The type of list elements being viewed. */
      using Type = T;

      /**
       * The function used to translate list values into table columns.
       * @param value The value to translate into columns.
       * @param column The index of the column to translate from the
       *        <i>value</i>.
       * @return A reference to the column to view.
       */
      using Accessor = std::function<AnyRef (Type& value, int column)>;

      /**
       * Constructs a ListToTableModel.
       * @param list The list to view.
       * @param columns The number of columns to represent.
       * @param accessor The function used to translate list values into table
       *        rows.
       */
      ListToTableModel(
        std::shared_ptr<ListModel<Type>> list, int columns, Accessor accessor);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      QValidator::State
        set(int row, int column, const std::any& value) override;

      QValidator::State remove(int row) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<ListModel<Type>> m_list;
      boost::signals2::scoped_connection m_connection;
      int m_columns;
      Accessor m_accessor;
      TableModelTransactionLog m_transaction;

      void on_operation(const typename ListModel<Type>::Operation& operation);
  };

  template<typename T>
  ListToTableModel(T, int,
    typename ListToTableModel<typename T::element_type::Type>::Accessor) ->
      ListToTableModel<typename T::element_type::Type>;

  template<typename T>
  ListToTableModel<T>::ListToTableModel(
    std::shared_ptr<ListModel<Type>> list, int columns, Accessor accessor)
    : m_list(std::move(list)),
      m_columns(columns),
      m_accessor(std::move(accessor)),
      m_connection(m_list->connect_operation_signal(
        std::bind_front(&ListToTableModel::on_operation, this))) {}

  template<typename T>
  int ListToTableModel<T>::get_row_size() const {
    return m_list->get_size();
  }

  template<typename T>
  int ListToTableModel<T>::get_column_size() const {
    return m_columns;
  }

  template<typename T>
  AnyRef ListToTableModel<T>::at(int row, int column) const {
    if(column < 0 || column >= get_column_size()) {
      throw std::out_of_range("The column is out of range.");
    }
    auto& element = m_list->get(row);
    return m_accessor(const_cast<Type&>(element), column);
  }

  template<typename T>
  QValidator::State
      ListToTableModel<T>::set(int row, int column, const std::any& value) {
    if(column < 0 || column >= get_column_size()) {
      throw std::out_of_range("The column is out of range.");
    }
    auto element = m_list->get(row);
    m_accessor(element, column).assign(value);
    return m_list->set(row, element);
  }

  template<typename T>
  QValidator::State ListToTableModel<T>::remove(int row) {
    return m_list->remove(row);
  }

  template<typename T>
  boost::signals2::connection ListToTableModel<T>::connect_operation_signal(
      const OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void ListToTableModel<T>::on_operation(
      const typename ListModel<Type>::Operation& operation) {
    visit(operation,
      [&] (const typename ListModel<Type>::StartTransaction&) {
        m_transaction.start();
      },
      [&] (const typename ListModel<Type>::EndTransaction&) {
        m_transaction.end();
      },
      [&] (const typename ListModel<Type>::AddOperation& operation) {
        m_transaction.push(TableModel::AddOperation(operation.m_index));
      },
      [&] (const typename ListModel<Type>::MoveOperation& operation) {
        m_transaction.push(TableModel::MoveOperation(
          operation.m_source, operation.m_destination));
      },
      [&] (const typename ListModel<Type>::PreRemoveOperation& operation) {
        m_transaction.push(TableModel::PreRemoveOperation(operation.m_index));
      },
      [&] (const typename ListModel<Type>::RemoveOperation& operation) {
        m_transaction.push(TableModel::RemoveOperation(operation.m_index));
      },
      [&] (const typename ListModel<Type>::UpdateOperation& operation) {
        m_transaction.transact([&] {
          for(auto i = 0; i < get_column_size(); ++i) {
            m_transaction.push(TableModel::UpdateOperation(operation.m_index, i,
              to_any(
                m_accessor(const_cast<Type&>(operation.get_previous()), i)),
              to_any(m_accessor(const_cast<Type&>(operation.get_value()), i))));
          }
        });
    });
  }
}

#endif
