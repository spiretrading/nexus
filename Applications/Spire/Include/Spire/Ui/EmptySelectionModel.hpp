#ifndef SPIRE_EMPTY_SELECTION_MODEL_HPP
#define SPIRE_EMPTY_SELECTION_MODEL_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TableIndex.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a selection model used when no items can be selected.
   * @param <T> The type of index to select.
   */
  template<typename T>
  class EmptySelectionModel : public ListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;

      using OperationSignal = typename ListModel<T>::OperationSignal;

      using AddOperation = typename ListModel<T>::AddOperation;

      using MoveOperation = typename ListModel<T>::MoveOperation;

      using RemoveOperation = typename ListModel<T>::RemoveOperation;

      using UpdateOperation = typename ListModel<T>::UpdateOperation;

      using StartTransaction = typename ListModel<T>::StartTransaction;

      using EndTransaction = typename ListModel<T>::EndTransaction;

      /** The type of index to select. */
      using Index = T;

      /** Constructs an EmptySelectionModel. */
      EmptySelectionModel() = default;

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
  };

  /** The type of empty selection model used for ListViews. */
  using ListEmptySelectionModel = EmptySelectionModel<int>;

  /** The type of empty selection model used for TableViews. */
  using TableEmptySelectionModel = EmptySelectionModel<TableIndex>;

  template<typename T>
  int EmptySelectionModel<T>::get_size() const {
    return 0;
  }

  template<typename T>
  const typename EmptySelectionModel<T>::Type&
      EmptySelectionModel<T>::get(int index) const {
    throw std::out_of_range("The index is out of range.");
  }

  template<typename T>
  QValidator::State EmptySelectionModel<T>::set(int index, const Type& value) {
    throw std::out_of_range("The index is out of range.");
  }

  template<typename T>
  QValidator::State EmptySelectionModel<T>::insert(
      const Type& value, int index) {
    if(index == 0) {
      return QValidator::State::Invalid;
    }
    throw std::out_of_range("The index is out of range.");
  }

  template<typename T>
  QValidator::State EmptySelectionModel<T>::move(int source, int destination) {
    throw std::out_of_range("The source or destination is out of range.");
  }

  template<typename T>
  QValidator::State EmptySelectionModel<T>::remove(int index) {
    throw std::out_of_range("The index is out of range.");
  }

  template<typename T>
  boost::signals2::connection EmptySelectionModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return {};
  }

  template<typename T>
  void EmptySelectionModel<T>::transact(
      const std::function<void ()>& transaction) {
    transaction();
  }
}

#endif
