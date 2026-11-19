#ifndef SPIRE_LIST_SELECTION_VALUE_MODEL_HPP
#define SPIRE_LIST_SELECTION_VALUE_MODEL_HPP
#include <memory>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TransactionLogModel.hpp"

namespace Spire {

  /**
   * Lists the values corresponding to a list of selected indexes.
   * @param <T> The type of model being selected from.
   */
  template<typename T>
  class ListSelectionValueModel : public TransactionLogModel<ListModel<T>> {
    public:
      using Type = typename ListModel<T>::Type;
      using OperationSignal = typename ListModel<T>::OperationSignal;
      using AddOperation = typename ListModel<T>::AddOperation;
      using MoveOperation = typename ListModel<T>::MoveOperation;
      using PreRemoveOperation = typename ListModel<T>::PreRemoveOperation;
      using RemoveOperation = typename ListModel<T>::RemoveOperation;
      using UpdateOperation = typename ListModel<T>::UpdateOperation;

      /**
       * Constructs a ListSelectionValueModel.
       * @param values The list of values available for selection.
       * @param selection The list of selected indexes.
       */
      ListSelectionValueModel(std::shared_ptr<ListModel<Type>> values,
        std::shared_ptr<ListModel<int>> selection);

      int get_size() const override;

      const Type& get(int index) const override;

    private:
      std::shared_ptr<ListModel<Type>> m_values;
      std::shared_ptr<ListModel<int>> m_selection;
      std::vector<Type> m_selection_values;
      boost::signals2::scoped_connection m_values_connection;
      boost::signals2::scoped_connection m_selection_connection;

      void update(int index, const Type& value);
      void on_values_operation(
        const typename ListModel<Type>::Operation& operation);
      void on_selection_operation(const ListModel<int>::Operation& operation);
  };

  template<typename T>
  ListSelectionValueModel<T>::ListSelectionValueModel(
      std::shared_ptr<ListModel<Type>> values,
      std::shared_ptr<ListModel<int>> selection)
      : m_values(std::move(values)),
        m_selection(std::move(selection)) {
    this->m_transaction.set_size(get_size());
    m_values_connection = m_values->connect_operation_signal(
      std::bind_front(&ListSelectionValueModel::on_values_operation, this));
    m_selection_connection = m_selection->connect_operation_signal(
      std::bind_front(&ListSelectionValueModel::on_selection_operation, this));
    for(auto i = 0; i != get_size(); ++i) {
      m_selection_values.push_back(get(i));
    }
  }

  template<typename T>
  int ListSelectionValueModel<T>::get_size() const {
    return m_selection->get_size();
  }

  template<typename T>
  const typename ListSelectionValueModel<T>::Type&
      ListSelectionValueModel<T>::get(int index) const {
    return m_values->get(m_selection->get(index));
  }

  template<typename T>
  void ListSelectionValueModel<T>::update(int index, const Type& value) {
    auto update = UpdateOperation(index, m_selection_values[index], value);
    m_selection_values[index] = update.get_value();
    this->m_transaction.push(update);
  }

  template<typename T>
  void ListSelectionValueModel<T>::on_values_operation(
      const typename ListModel<Type>::Operation& operation) {
    visit(operation,
      [&] (const UpdateOperation& operation) {
        for(auto i = 0; i != get_size(); ++i) {
          if(m_selection->get(i) == operation.m_index) {
            update(i, operation.get_value());
          }
        }
      },
      [&] (const AnyListModel::StartTransaction&) {
        this->m_transaction.start();
      },
      [&] (const AnyListModel::EndTransaction&) {
        this->m_transaction.end();
      });
  }

  template<typename T>
  void ListSelectionValueModel<T>::on_selection_operation(
      const ListModel<int>::Operation& operation) {
    visit(operation,
      [&] (const AddOperation& operation) {
        m_selection_values.insert(
          m_selection_values.begin() + operation.m_index,
          get(operation.m_index));
        this->m_transaction.push(operation);
      },
      [&] (const RemoveOperation& operation) {
        m_selection_values.erase(
          m_selection_values.begin() + operation.m_index);
        this->m_transaction.push(operation);
      },
      [&] (const MoveOperation& operation) {
        move_element(
          m_selection_values, operation.m_source, operation.m_destination);
        this->m_transaction.push(operation);
      },
      [&] (const ListModel<int>::UpdateOperation& operation) {
        update(operation.m_index, get(operation.m_index));
      },
      [&] (const std::convertible_to<typename ListModel<Type>::Operation>
          auto& operation) {
        this->m_transaction.push(operation);
      });
  }
}

#endif
