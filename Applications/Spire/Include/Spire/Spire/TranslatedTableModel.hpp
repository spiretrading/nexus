#ifndef SPIRE_TRANSLATED_TABLE_MODEL_HPP
#define SPIRE_TRANSLATED_TABLE_MODEL_HPP
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /**
   * Implements a TableModel that provides a view over a source model by
   * translating its rows. Translation in this sense refers to the Euclidean
   * notion of translation, where points are moved about an origin.
   */
  class TranslatedTableModel : public TableModel {
    public:

      /** 
       * Constructs a TranslatedTableModel from a TableModel.
       * @param source The table to translate.
       */
      explicit TranslatedTableModel(std::shared_ptr<TableModel> source);

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

      int get_row_size() const override;
      int get_column_size() const override;
      AnyRef at(int row, int column) const override;
      QValidator::State set(
        int row, int column, const std::any& value) override;
      QValidator::State remove(int row) override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      std::vector<int> m_translation;
      std::vector<int> m_reverse_translation;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      void translate(int direction, int row);
      void on_operation(const Operation& operation);
  };

  template<typename F>
  decltype(auto) TranslatedTableModel::transact(F&& transaction) {
    return m_transaction.transact(std::forward<F>(transaction));
  }
}

#endif
