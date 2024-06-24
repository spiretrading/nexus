#ifndef SPIRE_SORTED_LIST_MODEL_HPP
#define SPIRE_SORTED_LIST_MODEL_HPP
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <boost/iterator/counting_iterator.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Spire/TranslatedListModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  /**
   * Implements a ListModel that provides a view over a source list by sorting
   * its elements.
   */
  template<typename T>
  class SortedListModel : public ListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;

      using Operation = typename ListModel<T>::Operation;

      using OperationSignal = typename ListModel<T>::OperationSignal;

      using AddOperation = typename ListModel<T>::AddOperation;

      using MoveOperation = typename ListModel<T>::MoveOperation;

      using PreRemoveOperation = typename ListModel<T>::PreRemoveOperation;

      using RemoveOperation = typename ListModel<T>::RemoveOperation;

      using UpdateOperation = typename ListModel<T>::UpdateOperation;

      using StartTransaction = typename ListModel<T>::StartTransaction;

      using EndTransaction = typename ListModel<T>::EndTransaction;

      /**
       * Comparison function that accepts two elements along with their sorted
       * index and returns whether the first element is less than the second.
       * @return Returns <code>true</code> iff the first argument is less than
       *         the second.
       */
      using Comparator = std::function<bool (
        const Type& left, int left_index, const Type& right, int right_index)>;

      /**
       * Binary function that accepts two elements and returns whether the first
       * argument is less than the second.
       * @return Returns <code>true</code> iff the first argument is less than
       *         the second.
       */
      using ValueComparator =
        std::function<bool (const Type& left, const Type& right)>;

      /** 
       * Constructs a SortedListModel from a ListModel.
       * @param source The list to sort.
       */
      explicit SortedListModel(std::shared_ptr<ListModel<Type>> source);

      /**
       * Constructs a SortedListModel from a ListModel and a comparator.
       * @param source The list to sort.
       * @param compartor A comparison function.
       */
      SortedListModel(
        std::shared_ptr<ListModel<Type>> source, ValueComparator comparator);

      /**
       * Constructs a SortedListModel from a ListModel and a comparator.
       * @param source The list to sort.
       * @param compartor A comparison function.
       */
      SortedListModel(
        std::shared_ptr<ListModel<Type>> source, Comparator comparator);

      /** Returns the comparator used to rank elements. */
      const Comparator& get_comparator() const;

      int get_size() const override;

      const Type& get(int index) const override;

      QValidator::State set(int index, const Type& value) override;

      QValidator::State push(const Type& value) override;

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
      TranslatedListModel<Type> m_translation;
      Comparator m_comparator;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      bool comparator(int lhs, int rhs) const;
      void sort();
      int find_sorted_index(int index, int size) const;
      void on_operation(const Operation& operation);
  };

  template<typename T>
  SortedListModel(std::shared_ptr<T>) -> SortedListModel<typename T::Type>;

  template<typename T>
  SortedListModel(std::shared_ptr<T>,
    typename SortedListModel<typename T::Type>::ValueComparator) ->
      SortedListModel<typename T::Type>;

  template<typename T>
  SortedListModel(std::shared_ptr<T>,
    typename SortedListModel<typename T::Type>::Comparator) ->
      SortedListModel<typename T::Type>;

  template<typename T>
  SortedListModel<T>::SortedListModel(std::shared_ptr<ListModel<Type>> source)
    : SortedListModel(std::move(source), std::less<Type>()) {}

  template<typename T>
  SortedListModel<T>::SortedListModel(
    std::shared_ptr<ListModel<Type>> source, ValueComparator comparator)
    : SortedListModel(std::move(source),
        [comparator = std::move(comparator)] (const Type& left, int left_index,
            const Type& right, int right_index) {
          return comparator(left, right);
        }) {}

  template<typename T>
  SortedListModel<T>::SortedListModel(
      std::shared_ptr<ListModel<Type>> source, Comparator comparator)
      : m_source(std::move(source)),
        m_translation(m_source),
        m_comparator(std::move(comparator)) {
    sort();
    m_source_connection = m_translation.connect_operation_signal(
      std::bind_front(&SortedListModel::on_operation, this));
  }

  template<typename T>
  const typename SortedListModel<T>::Comparator&
      SortedListModel<T>::get_comparator() const {
    return m_comparator;
  }

  template<typename T>
  int SortedListModel<T>::get_size() const {
    return m_translation.get_size();
  }

  template<typename T>
  const typename SortedListModel<T>::Type&
      SortedListModel<T>::get(int index) const {
    return m_translation.get(index);
  }

  template<typename T>
  QValidator::State SortedListModel<T>::set(int index, const Type& value) {
    return m_translation.set(index, value);
  }

  template<typename T>
  QValidator::State SortedListModel<T>::push(const Type& value) {
    return m_translation.push(value);
  }

  template<typename T>
  QValidator::State SortedListModel<T>::insert(const Type& value, int index) {
    return m_translation.insert(value, index);
  }

  template<typename T>
  QValidator::State SortedListModel<T>::move(int source, int destination) {
    if(source == destination) {
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  template<typename T>
  QValidator::State SortedListModel<T>::remove(int index) {
    return m_translation.remove(index);
  }

  template<typename T>
  boost::signals2::connection SortedListModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void SortedListModel<T>::transact(const std::function<void ()>& transaction) {
    m_transaction.transact(transaction);
  }

  template<typename T>
  bool SortedListModel<T>::comparator(int lhs, int rhs) const {
    return m_comparator(
      m_translation.get(lhs), lhs, m_translation.get(rhs), rhs);
  }

  template<typename T>
  void SortedListModel<T>::sort() {
    m_transaction.transact([&] {
      m_translation.transact([&] {
        for(auto i = 1; i < get_size(); ++i) {
          auto index = find_sorted_index(i, i + 1);
          if(index != i) {
            m_translation.move(i, index);
            m_transaction.push(MoveOperation(i, index));
          }
        }
      });
    });
  }

  template<typename T>
  int SortedListModel<T>::find_sorted_index(int index, int size) const {
    if(index != 0 && comparator(index, index - 1)) {
      return *std::lower_bound(boost::iterators::make_counting_iterator(0),
        boost::iterators::make_counting_iterator(index), index,
        [&] (auto lhs, auto rhs) { return comparator(lhs, rhs); });
    } else if(index != size - 1 && comparator(index + 1, index)) {
      return *std::lower_bound(
        boost::iterators::make_counting_iterator(index + 1),
        boost::iterators::make_counting_iterator(size), index,
        [&] (auto lhs, auto rhs) { return comparator(lhs, rhs); }) - 1;
    }
    return index;
  }

  template<typename T>
  void SortedListModel<T>::on_operation(const Operation& operation) {
    visit(operation,
      [&] (const StartTransaction&) {
        m_transaction.start();
      },
      [&] (const EndTransaction&) {
        m_transaction.end();
      },
      [&] (const AddOperation& operation) {
        auto index = find_sorted_index(operation.m_index, get_size());
        m_translation.move(operation.m_index, index);
        m_transaction.push(AddOperation(index));
      },
      [&] (const UpdateOperation& operation) {
        auto index = find_sorted_index(operation.m_index, get_size());
        auto update = UpdateOperation(
          index, operation.get_previous(), operation.get_value());
        if(operation.m_index != index) {
          m_translation.move(operation.m_index, index);
          m_transaction.transact([&] {
            m_transaction.push(MoveOperation(operation.m_index, index));
            m_transaction.push(std::move(update));
          });
        } else {
          m_transaction.push(std::move(update));
        }
      },
      [&] (const PreRemoveOperation& operation) {
        m_transaction.push(operation);
      },
      [&] (const RemoveOperation& operation) {
        m_transaction.push(operation);
      });
  }
}

#endif
