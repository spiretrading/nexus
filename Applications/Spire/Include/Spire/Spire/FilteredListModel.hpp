#ifndef SPIRE_FILTERED_LIST_MODEL_HPP
#define SPIRE_FILTERED_LIST_MODEL_HPP
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"

namespace Spire {

  /** Implements a ListModel that filters values out of a source list. */
  template<typename T>
  class FilteredListModel : public ListModel<T> {
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
       * The type of callable used to filter elements.
       * @param list The list being filtered.
       * @param index The index of the element within the <i>list</i> to test.
       * @return <code>true</code> iff the <i>index</i> should be excluded.
       */
      using Filter =
        std::function<bool (const ListModel<Type>& list, int index)>;

      /**
       * Constructs a FilteredListModel from a ListModel and a filter function.
       * @param source The list to filter.
       * @param filter The filter applied to every element of the <i>source</i>.
       */
      FilteredListModel(std::shared_ptr<ListModel<Type>> source, Filter filter);

      /** Applies a new filter to this list. */
      void set_filter(const Filter& filter);

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
      Filter m_filter;
      std::vector<int> m_filtered_data;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      std::tuple<bool, std::vector<int>::iterator> find(int index);
      void on_operation(const Operation& operation);
  };

  template<typename T>
  FilteredListModel(
    std::shared_ptr<T>, typename FilteredListModel<typename T::Type>::Filter) ->
      FilteredListModel<typename T::Type>;
}

#endif
