#ifndef SPIRE_MODEL_TRANSACTION_LOG_HPP
#define SPIRE_MODEL_TRANSACTION_LOG_HPP
#include <functional>
#include <utility>
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Details {
  struct ScopeExit {
    std::function<void()> m_f;
    ScopeExit(std::function<void()> f);
    ~ScopeExit();
  };
}

  /** Logs a model transaction and signals it upon completion. */
  template<typename T>
  class ModelTransactionLog {
    public:

      /** The type of the model. */
      using Type = T;

      /** Constructs an empty log. */
      ModelTransactionLog();

      /**
       * Pushes an operation to a model transaction.
       * @param operation The operation to push.
       */
      void push(typename Type::Operation&& operation);

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
        const typename Type::OperationSignal::slot_type& slot) const;

    private:
      mutable typename Type::OperationSignal m_operation_signal;
      typename Type::Transaction m_transaction;
      int m_level;

      ModelTransactionLog(const ModelTransactionLog&) = delete;
      ModelTransactionLog& operator =(const ModelTransactionLog&) = delete;
  };

  template<typename T>
  ModelTransactionLog<T>::ModelTransactionLog()
    : m_level(0) {}

  template<typename T>
  void ModelTransactionLog<T>::push(typename T::Operation&& operation) {
    if(m_level > 0) {
      m_transaction.push_back(std::move(operation));
    } else {
      m_operation_signal(operation);
    }
  }

  template<typename T>
  template<typename F>
  decltype(auto) ModelTransactionLog<T>::transact(F&& transaction) {
    ++m_level;
    auto on_exit = Details::ScopeExit([&] {
      --m_level;
      if(m_level != 0) {
        return;
      }
      auto transaction = std::move(m_transaction);
      if(transaction.empty()) {
        return;
      } else if(transaction.size() == 1) {
        m_operation_signal(transaction.front());
      } else {
        m_operation_signal(transaction);
      }
    });
    return std::forward<F>(transaction)();
  }

  template<typename T>
  boost::signals2::connection ModelTransactionLog<T>::connect_operation_signal(
    const typename T::OperationSignal::slot_type& slot) const {
    return m_operation_signal.connect(slot);
  }
}

#endif
