#ifndef SPIRE_TABLE_MODEL_TRANSACTION_LOG_HPP
#define SPIRE_TABLE_MODEL_TRANSACTION_LOG_HPP
#include <functional>
#include <utility>
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Logs a TableModel transaction and signals it upon completion. */
  class TableModelTransactionLog {
    public:

      /** Constructs an empty log. */
      TableModelTransactionLog();

      /**
       * Pushes an operation to a TableModel transaction.
       * @param operation The operation to push.
       */
      void push(TableModel::Operation&& operation);

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
       * Connects a slot to a signal indicating the completion of a transaction.
       */
      boost::signals2::connection connect_operation_signal(
        const TableModel::OperationSignal::slot_type& slot) const;

    private:
      struct ScopeExit {
        std::function<void ()> m_f;
        ScopeExit(std::function<void()> f);
        ~ScopeExit();
      };
      mutable TableModel::OperationSignal m_operation_signal;
      TableModel::Transaction m_transaction;
      int m_level;

      TableModelTransactionLog(const TableModelTransactionLog&) = delete;
      TableModelTransactionLog& operator =(
        const TableModelTransactionLog&) = delete;
  };

  template<typename F>
  decltype(auto) TableModelTransactionLog::transact(F&& transaction) {
    ++m_level;
    auto on_exit = ScopeExit([&] {
      --m_level;
      if(m_level != 0) {
        return;
      }
      auto transaction = std::move(m_transaction);
      if(transaction.m_operations.empty()) {
        return;
      } else if(transaction.m_operations.size() == 1) {
        m_operation_signal(transaction.m_operations.front());
      } else {
        m_operation_signal(transaction);
      }
    });
    return std::forward<F>(transaction)();
  }
}

#endif
