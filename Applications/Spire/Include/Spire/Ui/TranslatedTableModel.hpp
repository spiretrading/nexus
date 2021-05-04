#ifndef SPIRE_TRANSLATED_TABLE_MODEL_HPP
#define SPIRE_TRANSLATED_TABLE_MODEL_HPP
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a TableModel performing operations without modifying the original model. */
  class TranslatedTableModel : public TableModel {
    public:

      /** 
       * Constructs a TranslatedTableModel from a TableModel.
       * @param source The original model.
      */
      TranslatedTableModel(std::shared_ptr<TableModel> source);

      /**
       * Performs a transaction that consists of a series of operations.
       * @param transaction The transaction to perform.
       * @return The result of the transaction.
       */
      template<typename F>
      decltype(auto) transact(F&& transaction);

      /**
       * Moves a row by performing a translation.
       * @param source - The index of the row to move.
       * @param destination - The index to move the row to.
       * @throws <code>std::out_of_range</code> - The source or destination are
       *         not within this table's range.
       */
      void move(int source, int destination);

      int get_row_size() const override;

      int get_column_size() const override;

      const std::any& at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    private:
      struct ScopeExit {
        std::function<void ()> m_f;
        ScopeExit(std::function<void()> f);
        ~ScopeExit();
      };
      mutable OperationSignal m_operation_signal;
      std::shared_ptr<TableModel> m_source;
      std::vector<int> m_index_mapping;
      boost::signals2::scoped_connection m_source_operation_connection;
      Transaction m_transaction;
      int m_transaction_level;

      void push(Operation&& operation);
  };

  template<typename F>
  decltype(auto) TranslatedTableModel::transact(F&& transaction) {
    ++m_transaction_level;
    auto on_exit = ScopeExit([&] {
      --m_transaction_level;
      if(m_transaction_level != 0) {
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
