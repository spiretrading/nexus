#ifndef SPIRE_MULTI_SELECTION_MODEL_HPP
#define SPIRE_MULTI_SELECTION_MODEL_HPP
#include <unordered_set>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TableIndex.hpp"

namespace Spire {

  /**
   * Implements a selection model used when multiple items can be selected.
   * Ensures that no duplicate items are selected.
   * @param <T> The type of index to select.
   */
  template<typename T>
  class MultiSelectionModel : public ListModel<T> {
    public:

      /** The type of index to select. */
      using Index = T;

      using Type = typename ListModel<T>::Type;
      using OperationSignal = typename ListModel<T>::OperationSignal;
      using AddOperation = typename ListModel<T>::AddOperation;
      using MoveOperation = typename ListModel<T>::MoveOperation;
      using PreRemoveOperation = typename ListModel<T>::PreRemoveOperation;
      using RemoveOperation = typename ListModel<T>::RemoveOperation;
      using UpdateOperation = typename ListModel<T>::UpdateOperation;
      using StartTransaction = typename ListModel<T>::StartTransaction;
      using EndTransaction = typename ListModel<T>::EndTransaction;

      /** Constructs an empty MultiSelectionModel. */
      MultiSelectionModel() = default;

      int get_size() const override;
      const Type& get(int index) const override;
      QValidator::State set(int index, const Type& value) override;
      QValidator::State insert(const Type& value, int index) override;
      QValidator::State move(int source, int destination) override;
      QValidator::State remove(int index) override;
      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;
      using ListModel<T>::transact;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      std::unordered_set<Type> m_selection;
      ArrayListModel<Type> m_list;
  };

  /** The type of multi selection model used for ListViews. */
  using ListMultiSelectionModel = MultiSelectionModel<int>;

  /** The type of multi selection model used for TableViews. */
  using TableMultiSelectionModel = MultiSelectionModel<TableIndex>;

  template<typename T>
  int MultiSelectionModel<T>::get_size() const {
    return m_list.get_size();
  }

  template<typename T>
  const typename MultiSelectionModel<T>::Type&
      MultiSelectionModel<T>::get(int index) const {
    return m_list.get(index);
  }

  template<typename T>
  QValidator::State MultiSelectionModel<T>::set(int index, const Type& value) {
    auto previous = m_list.get(index);
    if(previous == value) {
      return QValidator::State::Acceptable;
    }
    if(m_selection.insert(value).second) {
      m_selection.erase(previous);
      return m_list.set(index, value);
    }
    return m_list.remove(index);
  }

  template<typename T>
  QValidator::State MultiSelectionModel<T>::insert(
      const Type& value, int index) {
    if(m_selection.insert(value).second) {
      return m_list.insert(value, index);
    }
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State MultiSelectionModel<T>::move(int source, int destination) {
    return m_list.move(source, destination);
  }

  template<typename T>
  QValidator::State MultiSelectionModel<T>::remove(int index) {
    m_selection.erase(m_list.get(index));
    return m_list.remove(index);
  }

  template<typename T>
  boost::signals2::connection MultiSelectionModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return m_list.connect_operation_signal(slot);
  }

  template<typename T>
  void MultiSelectionModel<T>::transact(
      const std::function<void ()>& transaction) {
    m_list.transact([&] { transaction(); });
  }
}

#endif
