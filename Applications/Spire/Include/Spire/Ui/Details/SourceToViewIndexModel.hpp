#ifndef SPIRE_SOURCE_TO_VIEW_INDEX_MODEL_HPP
#define SPIRE_SOURCE_TO_VIEW_INDEX_MODEL_HPP
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {
  class TableView::SourceToViewIndexModel : public TableView::CurrentModel {
    public:
      SourceToViewIndexModel(std::shared_ptr<SortedTableModel> sorted_table,
        std::shared_ptr<FilteredTableModel> filtered_table,
        std::shared_ptr<TableView::CurrentModel> current);

      const Type& get() const override;
      QValidator::State test(const Type& value) const override;
      QValidator::State set(const Type& value) override;
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<SortedTableModel> m_sorted_table;
      std::shared_ptr<FilteredTableModel> m_filtered_table;
      std::shared_ptr<TableView::CurrentModel> m_current;
      LocalValueModel<Type> m_index;
      mutable Type m_transaction_index;
      int m_source_size;
      bool m_is_source_transacting;
      bool m_is_view_transacting;
      boost::signals2::scoped_connection m_source_connection;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_table_connection;

      Type from_source(const Type& index) const;
      Type to_source(const Type& index) const;
      void on_current(const Type& index);
      void on_source_operation(const TableModel::Operation& operation);
      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
