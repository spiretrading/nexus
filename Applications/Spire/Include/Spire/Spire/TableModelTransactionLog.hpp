#ifndef SPIRE_TABLE_MODEL_TRANSACTION_LOG_HPP
#define SPIRE_TABLE_MODEL_TRANSACTION_LOG_HPP
#include "Spire/Spire/TableModel.hpp"

namespace Spire {
  class TableModelTransactionLog {
    public:

      /** Constructs an empty log. */
      TableModelTransactionLog();

      /** Starts a transaction. */
      void start();

      /** Ends a transaction. */
      void end();

      /**
       * Pushes an operation to a model transaction.
       * @param operation The operation to push.
       */
      void push(const TableModel::Operation& operation);

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
      mutable TableModel::OperationSignal m_operation_signal;
      int m_level;
      bool m_is_first;

      TableModelTransactionLog(const TableModelTransactionLog&) = delete;
      TableModelTransactionLog& operator =(
        const TableModelTransactionLog&) = delete;
  };

  inline TableModelTransactionLog::TableModelTransactionLog()
    : m_level(0),
      m_is_first(true) {}

  inline void TableModelTransactionLog::start() {
    ++m_level;
  }

  inline void TableModelTransactionLog::end() {
    --m_level;
    if(m_level == 0 && !m_is_first) {
      m_operation_signal(TableModel::EndTransaction());
      m_is_first = true;
    }
  }

  inline void TableModelTransactionLog::push(
      const TableModel::Operation& operation) {
    if(m_level != 0 && m_is_first) {
      m_is_first = false;
      m_operation_signal(TableModel::StartTransaction());
    }
    m_operation_signal(operation);
  }

  template<typename F>
  decltype(auto) TableModelTransactionLog::transact(F&& transaction) {
    start();
    try {
      std::forward<F>(transaction)();
    } catch(const std::exception&) {
      end();
      throw;
    }
    end();
  }

  inline boost::signals2::connection TableModelTransactionLog::
      connect_operation_signal(
        const TableModel::OperationSignal::slot_type& slot) const {
    return m_operation_signal.connect(slot);
  }
}

#endif
