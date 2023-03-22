#ifndef SPIRE_FREEZING_UPDATE_TABLE_MODEL_HPP
#define SPIRE_FREEZING_UPDATE_TABLE_MODEL_HPP
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /**
   * Implements a TableModel that prevents the table from signalling the update
   * when it is frozen. It could be used to prevent a column from being sorted
   * when a cell is being edited.
   */
  class FreezingUpdateTableModel : public TableModel {
    public:

      /**
       * Constructs a FreezingUpdateTableModel.
       * @param source The source model.
       */
      explicit FreezingUpdateTableModel(std::shared_ptr<TableModel> source);

      bool is_frozen() const;

      void set_frozen(bool is_frozen);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      QValidator::State set(
        int row, int column, const std::any& value) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
        std::shared_ptr<TableModel> m_source;
        bool m_is_frozen;
        boost::optional<UpdateOperation> m_frozen_update;
        TableModelTransactionLog m_transaction;
        boost::signals2::scoped_connection m_source_connection;

        void on_operation(const TableModel::Operation& operation);
  };
}

#endif
