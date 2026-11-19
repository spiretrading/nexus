#ifndef SPIRE_MODEL_TRANSACTION_LOG_HPP
#define SPIRE_MODEL_TRANSACTION_LOG_HPP
#include <cstdint>
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
       * Connects a slot to model operations. If a transaction is already open,
       * the slot receives its boundaries only if further operations occur.
       */
      boost::signals2::connection connect_operation_signal(
        const typename Type::OperationSignal::slot_type& slot) const;

    private:
      using Operation = typename Type::Operation;
      using NotificationSignal =
        Signal<void (const Operation&, std::uint64_t, bool)>;
      mutable NotificationSignal m_operation_signal;
      int m_size;
      int m_level;
      std::uint64_t m_sequence;
      std::uint64_t m_transaction;

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
      m_sequence(0),
      m_transaction(0) {}

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
    if(m_level == 0 && m_transaction != 0) {
      auto transaction = std::exchange(m_transaction, 0);
      m_operation_signal(typename Type::EndTransaction(), transaction, false);
    }
  }

  template<typename T>
  void ModelTransactionLog<T>::push(const typename Type::Operation& operation) {
    auto publish = [&] (bool shifts_indices) {
      auto notify = [&] {
        if(m_level != 0 && m_transaction == 0) {
          auto transaction = ++m_sequence;
          m_transaction = transaction;
          m_operation_signal(
            typename Type::StartTransaction(), transaction, false);
          m_operation_signal(operation, transaction, true);
        } else {
          m_operation_signal(operation, m_transaction, false);
        }
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
    using Slot = NotificationSignal::slot_type;
    return m_operation_signal.connect(Slot(
      [callback = slot.slot_function(), initial = m_transaction,
          needs_start = true] (const Operation& operation,
          std::uint64_t transaction, bool is_opening) mutable {
        if(initial != 0 && transaction == initial) {
          if(is_opening && !std::holds_alternative<
              typename Type::PreRemoveOperation>(operation)) {
            return;
          }
          if(needs_start) {
            if(std::holds_alternative<typename Type::EndTransaction>(
                operation)) {
              return;
            }
            needs_start = false;
            callback(typename Type::StartTransaction());
          }
        }
        callback(operation);
      }).track(slot));
  }
}

#endif
