#ifndef SPIRE_MODEL_TRANSACTION_LOG_HPP
#define SPIRE_MODEL_TRANSACTION_LOG_HPP
#include <utility>
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /** Logs a model transaction and signals it upon completion. */
  template<typename T>
  class ModelTransactionLog {
    public:

      /** The type of the model. */
      using Type = T;

      /** Constructs a log for an empty model. */
      ModelTransactionLog();

      /**
       * Constructs a log for a model with an initial size.
       * @param size The number of elements already in the model.
       */
      explicit ModelTransactionLog(int size);

      /**
       * Synchronizes the size after a model change made outside the log.
       * @param size The current number of elements in the model.
       */
      void set_size(int size);

      /** Starts a transaction. */
      void start();

      /** Ends a transaction. */
      void end();

      /**
       * Pushes an operation to the log.
       * @param operation The operation to push.
       */
      void push(const typename Type::Operation& operation);

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
      int m_size;
      int m_level;
      bool m_is_first;

      ModelTransactionLog(const ModelTransactionLog&) = delete;
      ModelTransactionLog& operator =(const ModelTransactionLog&) = delete;
  };

  template<typename T>
  ModelTransactionLog<T>::ModelTransactionLog()
    : ModelTransactionLog(0) {}

  template<typename T>
  ModelTransactionLog<T>::ModelTransactionLog(int size)
    : m_size(size),
      m_level(0),
      m_is_first(true) {}

  template<typename T>
  void ModelTransactionLog<T>::set_size(int size) {
    m_size = size;
  }

  template<typename T>
  void ModelTransactionLog<T>::start() {
    ++m_level;
  }

  template<typename T>
  void ModelTransactionLog<T>::end() {
    --m_level;
    if(m_level == 0 && !m_is_first) {
      m_is_first = true;
      m_operation_signal(typename Type::EndTransaction());
    }
  }

  template<typename T>
  void ModelTransactionLog<T>::push(const typename Type::Operation& operation) {
    auto publish = [&] (bool shifts_indices) {
      auto notify = [&] {
        if(m_level != 0 && m_is_first) {
          m_is_first = false;
          m_operation_signal(typename Type::StartTransaction());
        }
        m_operation_signal(operation);
      };
      if(shifts_indices && m_level == 0) {
        transact(notify);
      } else {
        notify();
      }
    };
    visit(operation,
      [&] (const typename Type::StartTransaction&) {
        start();
      },
      [&] (const typename Type::EndTransaction&) {
        end();
      },
      [&] (const typename Type::AddOperation& add) {
        auto shifts_indices = add.m_index < m_size;
        ++m_size;
        publish(shifts_indices);
      },
      [&] (const typename Type::MoveOperation&) {
        publish(true);
      },
      [&] (const typename Type::RemoveOperation&) {
        --m_size;
        publish(false);
      },
      [&] (const auto&) {
        publish(false);
      });
  }

  template<typename T>
  template<typename F>
  decltype(auto) ModelTransactionLog<T>::transact(F&& transaction) {
    start();
    try {
      std::forward<F>(transaction)();
    } catch(...) {
      end();
      throw;
    }
    end();
  }

  template<typename T>
  boost::signals2::connection ModelTransactionLog<T>::connect_operation_signal(
      const typename Type::OperationSignal::slot_type& slot) const {
    return m_operation_signal.connect(slot);
  }
}

#endif
