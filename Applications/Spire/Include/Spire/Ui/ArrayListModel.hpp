#ifndef SPIRE_ARRAY_LIST_MODEL_HPP
#define SPIRE_ARRAY_LIST_MODEL_HPP
#include "Spire/Ui/ListModel.hpp"
#include "Spire/Ui/ModelTransactionLog.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a ListModel using an array as its backing data structure. */
  class ArrayListModel : public ListModel {
    public:

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
      void push(const std::any& value);

      /**
       * Inserts a value at a specified index.
       * @param value The value to insert.
       * @param index The index to insert the value at.
       * @throws <code>std::out_of_range</code> -
       *         <code>index < 0 or index > get_size()</code>.
       */
      void insert(const std::any& value, int index);

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

      const std::any& at(int index) const override;

      QValidator::State set(int index, const std::any& value) override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    private:
      std::vector<std::any> m_data;
      ModelTransactionLog<ListModel> m_transaction;
  };

  template<typename F>
  decltype(auto) ArrayListModel::transact(F&& transaction) {
    return m_transaction.transact(std::forward<F>(transaction));
  }
}

#endif
