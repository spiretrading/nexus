#ifndef SPIRE_REVERSED_LIST_MODEL_HPP
#define SPIRE_REVERSED_LIST_MODEL_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements a ListModel that wraps an existing ListModel and exposes its
   * rows in reversed order.
   */
  template<typename T>
  class ReversedListModel : public ListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;

      using OperationSignal = typename ListModel<T>::OperationSignal;

      using AddOperation = typename ListModel<T>::AddOperation;

      using MoveOperation = typename ListModel<T>::MoveOperation;

      using PreRemoveOperation = typename ListModel<T>::PreRemoveOperation;

      using RemoveOperation = typename ListModel<T>::RemoveOperation;

      using UpdateOperation = typename ListModel<T>::UpdateOperation;

      using StartTransaction = typename ListModel<T>::StartTransaction;

      using EndTransaction = typename ListModel<T>::EndTransaction;

      using Operation = typename ListModel<T>::Operation;

      /**
       * Constructs a ReversedListModel.
       * @param source The ListModel to view in reversed order.
       */
      explicit ReversedListModel(std::shared_ptr<ListModel<Type>> source);

      /** Returns the source being reversed. */
      const std::shared_ptr<ListModel<Type>>& get_source() const;

      /**
       * Maps a row index from this table into the source table.
       * @param index An index into a row of this table.
       * @return The corresponding row index into the source table or
       *         <code>-1</code> iff the row index is not valid.
       */
      int index_to_source(int index) const;

      /**
       * Maps a row index from the source table to this table.
       * @param index An index into a row of the source table.
       * @return The corresponding row index into this table, or <code>-1</code>
       *         iff the row index is not valid.
       */
      int index_from_source(int index) const;

      int get_size() const override;

      const Type& get(int index) const override;

      QValidator::State set(int index, const Type& value) override;

      QValidator::State insert(const Type& value, int index) override;

      QValidator::State move(int source, int destination) override;

      QValidator::State remove(int index) override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

      using ListModel<T>::insert;
      using ListModel<T>::remove;
      using ListModel<T>::transact;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      std::shared_ptr<ListModel<Type>> m_source;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_connection;

      void on_operation(const Operation& operation);
  };

  template<typename T>
  ReversedListModel(std::shared_ptr<T>) -> ReversedListModel<typename T::Type>;

  template<typename T>
  ReversedListModel<T>::ReversedListModel(
      std::shared_ptr<ListModel<Type>> source)
      : m_source(std::move(source)) {
    m_connection = m_source->connect_operation_signal(
      std::bind_front(&ReversedListModel::on_operation, this));
  }

  template<typename T>
  const std::shared_ptr<ListModel<typename ReversedListModel<T>::Type>>&
      ReversedListModel<T>::get_source() const {
    return m_source;
  }

  template<typename T>
  int ReversedListModel<T>::index_to_source(int index) const {
    return get_size() - index - 1;
  }

  template<typename T>
  int ReversedListModel<T>::index_from_source(int index) const {
    return index_to_source(index);
  }

  template<typename T>
  int ReversedListModel<T>::get_size() const {
    return m_source->get_size();
  }

  template<typename T>
  const typename ReversedListModel<T>::Type&
      ReversedListModel<T>::get(int index) const {
    return m_source->get(index_to_source(index));
  }

  template<typename T>
  QValidator::State ReversedListModel<T>::set(int index, const Type& value) {
    return m_source->set(index_to_source(index), value);
  }

  template<typename T>
  QValidator::State ReversedListModel<T>::insert(const Type& value, int index) {
    return m_source->insert(value, index_to_source(index) + 1);
  }

  template<typename T>
  QValidator::State ReversedListModel<T>::move(int source, int destination) {
    return m_source->move(
      index_to_source(source), index_to_source(destination));
  }

  template<typename T>
  QValidator::State ReversedListModel<T>::remove(int index) {
    return m_source->remove(index_to_source(index));
  }

  template<typename T>
  boost::signals2::connection ReversedListModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void ReversedListModel<T>::transact(
      const std::function<void ()>& transaction) {
    m_transaction.transact([&] {
      transaction();
    });
  }

  template<typename T>
  void ReversedListModel<T>::on_operation(const Operation& operation) {
    visit(operation,
      [&] (StartTransaction) {
        m_transaction.start();
      },
      [&] (EndTransaction) {
        m_transaction.end();
      },
      [&] (const AddOperation& operation) {
        m_transaction.push(AddOperation(index_from_source(operation.m_index)));
      },
      [&] (const PreRemoveOperation& operation) {
        m_transaction.push(
          PreRemoveOperation(index_from_source(operation.m_index)));
      },
      [&] (const RemoveOperation& operation) {
        m_transaction.push(
          RemoveOperation(index_from_source(operation.m_index)));
      },
      [&] (const MoveOperation& operation) {
        m_transaction.push(MoveOperation(index_from_source(operation.m_source),
          index_from_source(operation.m_destination)));
      },
      [&] (const UpdateOperation& operation) {
        m_transaction.push(UpdateOperation(index_from_source(operation.m_index),
          operation.get_previous(), operation.get_value()));
      });
  }
}

#endif
