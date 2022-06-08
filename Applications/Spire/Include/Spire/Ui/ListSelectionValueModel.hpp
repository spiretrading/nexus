#ifndef SPIRE_LIST_SELECTION_VALUE_MODEL_HPP
#define SPIRE_LIST_SELECTION_VALUE_MODEL_HPP
#include <memory>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Lists the values corresponding to a list of selected indexes.
   * @param <T> The type of model being selected from.
   */
  template<typename T>
  class ListSelectionValueModel : public ListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;

      using OperationSignal = typename ListModel<T>::OperationSignal;

      using AddOperation = typename ListModel<T>::AddOperation;

      using MoveOperation = typename ListModel<T>::MoveOperation;

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

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

      using ListModel<T>::transact;

    protected:
      void transact(const std::function<void ()>& f) override;

    private:
      std::shared_ptr<ListModel<Type>> m_values;
      std::shared_ptr<ListModel<int>> m_selection;
      std::vector<Type> m_selection_values;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_connection;

      void on_operation(const typename ListModel<int>::Operation& operation);
  };

  template<typename T>
  ListSelectionValueModel<T>::ListSelectionValueModel(
      std::shared_ptr<ListModel<Type>> values,
      std::shared_ptr<ListModel<int>> selection)
      : m_values(std::move(values)),
        m_selection(std::move(selection)),
        m_connection(m_selection->connect_operation_signal(
          std::bind_front(&ListSelectionValueModel::on_operation, this))) {
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
  boost::signals2::connection
      ListSelectionValueModel<T>::connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void ListSelectionValueModel<T>::transact(const std::function<void ()>& f) {
    f();
  }

  template<typename T>
  void ListSelectionValueModel<T>::on_operation(
      const typename ListModel<int>::Operation& operation) {
    m_transaction.transact([&] {
      visit(operation,
        [&] (const typename ListModel<int>::AddOperation& operation) {
          m_selection_values.insert(
            m_selection_values.begin() + operation.get_value(),
            get(operation.get_value()));
          m_transaction.push(
            AddOperation(operation.m_index, get(operation.get_value())));
        },
        [&] (const typename ListModel<int>::RemoveOperation& operation) {
          m_transaction.push(RemoveOperation(
            operation.m_index, m_selection_values[operation.get_value()]));
          m_selection_values.erase(
            m_selection_values.begin() + operation.get_value());
        },
        [&] (const typename ListModel<int>::MoveOperation& operation) {
          if(operation.m_source < operation.m_destination) {
            std::rotate(m_selection_values.rend() - operation.m_source - 1,
              m_selection_values.rend() - operation.m_source,
              m_selection_values.rend() - operation.m_destination);
          } else {
            std::rotate(m_selection_values.begin() + operation.m_source,
              m_selection_values.begin() + operation.m_source + 1,
              m_selection_values.begin() + operation.m_destination + 1);
          }
          m_transaction.push(operation);
        },
        [&] (const typename ListModel<int>::UpdateOperation& operation) {
          m_transaction.push(UpdateOperation(operation.m_index,
            m_selection_values[operation.m_index], get(operation.m_index)));
          m_selection_values[operation.m_index] = get(operation.m_index);
        });
    });
  }
}

#endif
