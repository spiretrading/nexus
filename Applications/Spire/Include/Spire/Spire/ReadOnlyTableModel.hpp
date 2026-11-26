#ifndef SPIRE_READ_ONLY_TABLE_MODEL_HPP
#define SPIRE_READ_ONLY_TABLE_MODEL_HPP
#include <memory>
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /** A read-only view of a source table that forwards its operations. */
  class ReadOnlyTableModel : public TableModel {
    public:

      /**
       * Constructs a read-only view.
       * @param source The table that supplies cell values and signals changes.
       */
      explicit ReadOnlyTableModel(
        std::shared_ptr<const TableModel> source);

      int get_row_size() const override;
      int get_column_size() const override;
      AnyRef at(int row, int column) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;
      void start_transaction() override;
      void end_transaction() override;

    private:
      mutable OperationSignal m_operation_signal;
      std::shared_ptr<const TableModel> m_source;
      boost::signals2::scoped_connection m_source_connection;
  };
}

#endif
