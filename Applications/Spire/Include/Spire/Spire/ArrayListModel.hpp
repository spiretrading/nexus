#ifndef SPIRE_ARRAY_LIST_MODEL_HPP
#define SPIRE_ARRAY_LIST_MODEL_HPP
#include <stdexcept>
#include <vector>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/ListModelTransactionLog.hpp"

namespace Spire {

  /** Implements a ListModel using an array as its backing data structure. */
  template<typename T>
  class ArrayListModel : public ListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;

      using OperationSignal = ListModel<T>::OperationSignal;

      using ListModel<T>::AddOperation;

      using ListModel<T>::RemoveOperation;

      using ListModel<T>::MoveOperation;

      using ListModel<T>::UpdateOperation;

      /** Constructs an empty ArrayListModel. */
      ArrayListModel() = default;

      /**
       * Takes a callable function and invokes it. All operations performed on
       * this model during the transaction get appended to a
       * <code>Transaction</code> that is signalled at the end of the
       * transaction. If a transaction is already being invoked, then all
       * operations are appened into the parent transaction.
       * @param transaction The transaction to perform.
       * @return The result of the transaction.
       */
      template<typename F>
      decltype(auto) transact(F&& transaction);

      /**
       * Appends a value.
       * @param value The value to append to this model.
       */
      void push(const Type& value);

      /**
       * Inserts a value at a specified index.
       * @param value The value to insert.
       * @param index The index to insert the value at.
       * @throws <code>std::out_of_range</code> -
       *         <code>index < 0 or index > get_size()</code>.
       */
      void insert(const Type& value, int index);

      /**
       * Moves a value.
       * @param source - The index of the value to move.
       * @param destination - The index to move the value to.
       * @throws <code>std::out_of_range</code> - The source or destination are
       *         not within this table's range.
       */
      void move(int source, int destination);

      /**
       * Removes a value from the table.
       * @param index - The index of the value to remove.
       * @throws <code>std::out_of_range</code> - The index is not within this
       *         table's range.
       */
      void remove(int index);

      int get_size() const override;

      const Type& get(int index) const override;

      QValidator::State set(int index, const Type& value) override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    private:
      std::vector<Type> m_data;
      ListModelTransactionLog<Type> m_transaction;
  };

  template<typename T>
  template<typename F>
  decltype(auto) ArrayListModel<T>::transact(F&& transaction) {
    return m_transaction.transact(std::forward<F>(transaction));
  }

  template<typename T>
  void ArrayListModel<T>::push(const Type& value) {
    insert(value, get_size());
  }

  template<typename T>
  void ArrayListModel<T>::insert(const Type& value, int index) {
    if(index < 0 || index > get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    m_data.insert(std::next(m_data.begin(), index), value);
    m_transaction.push(AddOperation(index));
  }

  template<typename T>
  void ArrayListModel<T>::move(int source, int destination) {
    if(source < 0 || source >= get_size() || destination < 0 ||
        destination >= get_size()) {
      throw std::out_of_range("The source or destination is out of range.");
    }
    if(source == destination) {
      return;
    }
    auto source_row = std::move(m_data[source]);
    if(source < destination) {
      std::move(std::next(m_data.begin(), source + 1),
        std::next(m_data.begin(), destination + 1),
        std::next(m_data.begin(), source));
    } else {
      std::move_backward(std::next(m_data.begin(), destination),
        std::next(m_data.begin(), source),
        std::next(m_data.begin(), source + 1));
    }
    m_data[destination] = std::move(source_row);
    m_transaction.push(MoveOperation(source, destination));
  }

  template<typename T>
  void ArrayListModel<T>::remove(int index) {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    m_data.erase(std::next(m_data.begin(), index));
    m_transaction.push(RemoveOperation(index));
  }

  template<typename T>
  int ArrayListModel<T>::get_size() const {
    return static_cast<int>(m_data.size());
  }

  template<typename T>
  const typename ArrayListModel<T>::Type& ArrayListModel<T>::get(int index)
      const {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_data[index];
  }

  template<typename T>
  QValidator::State ArrayListModel<T>::set(int index, const Type& value) {
    if(index < 0 || index >= get_size()) {
      return QValidator::State::Invalid;
    }
    m_data[index] = value;
    m_transaction.push(UpdateOperation(index));
    return QValidator::State::Acceptable;
  }

  template<typename T>
  boost::signals2::connection ArrayListModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }
}

#endif