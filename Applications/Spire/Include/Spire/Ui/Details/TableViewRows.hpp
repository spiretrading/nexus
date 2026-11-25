#ifndef SPIRE_TABLE_VIEW_ROWS_HPP
#define SPIRE_TABLE_VIEW_ROWS_HPP
#include "Spire/Spire/FilteredTableModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {
  class TableView::Rows {
    public:
      Rows() = default;
      Rows(std::shared_ptr<FilteredTableModel> filtered_table,
        std::shared_ptr<SortedTableModel> sorted_table) noexcept;

      const std::shared_ptr<FilteredTableModel>& get_filtered_table() const;
      std::shared_ptr<TableModel> get_table() const;
      int index_from_source(int row) const;
      int index_to_source(int row) const;

    private:
      std::shared_ptr<FilteredTableModel> m_filtered_table;
      std::shared_ptr<SortedTableModel> m_sorted_table;
  };
}

#endif
