#ifndef SPIRE_SINGLE_SELECTION_MODEL_HPP
#define SPIRE_SINGLE_SELECTION_MODEL_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Spire/TableIndex.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a selection model used when only a single item can be selected.
   * @param <T> The type of index to select.
   */
  template<typename T>
  class SingleSelectionModel : public ListModel<T> {
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

      /** Constructs an empty SingleSelectionModel. */
      SingleSelectionModel() = default;

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
      void transact(const std::function<void ()>& f) override;

    private:
      boost::optional<Type> m_value;
      ListModelTransactionLog<Type> m_transaction;
  };

  /** The type of single selection model used for ListViews. */
  using ListSingleSelectionModel = SingleSelectionModel<int>;

  /** The type of single selection model used for TableViews. */
  using TableSingleSelectionModel = SingleSelectionModel<TableIndex>;

  template<typename T>
  int SingleSelectionModel<T>::get_size() const {
    if(m_value) {
      return 1;
    }
    return 0;
  }

  template<typename T>
  const typename SingleSelectionModel<T>::Type&
      SingleSelectionModel<T>::get(int index) const {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return *m_value;
  }

  template<typename T>
  QValidator::State SingleSelectionModel<T>::set(int index, const Type& value) {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    auto previous = *m_value;
    *m_value = value;
    m_transaction.push(UpdateOperation(index, previous, value));
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State SingleSelectionModel<T>::insert(
      const Type& value, int index) {
    if(!m_value && index == 0) {
      m_value = value;
      m_transaction.push(AddOperation(index, value));
      return QValidator::State::Acceptable;
    } else if(m_value && (index == 0 || index == 1)) {
      auto previous = *m_value;
      m_value = value;
      m_transaction.push(UpdateOperation(0, previous, value));
      return QValidator::State::Acceptable;
    }
    throw std::out_of_range("The index is out of range.");
  }

  template<typename T>
  QValidator::State SingleSelectionModel<T>::move(int source, int destination) {
    if(source < 0 || source >= get_size() || destination < 0 ||
        destination >= get_size()) {
      throw std::out_of_range("The source or destination is out of range.");
    }
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State SingleSelectionModel<T>::remove(int index) {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    auto previous = *m_value;
    m_value = boost::none;
    m_transaction.push(RemoveOperation(index, previous));
    return QValidator::State::Acceptable;
  }

  template<typename T>
  boost::signals2::connection SingleSelectionModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void SingleSelectionModel<T>::transact(
      const std::function<void ()>& transaction) {
    m_transaction.transact(transaction);
  }
}

#endif
