#ifndef SPIRE_STANDARD_TABLE_FILTER_HPP
#define SPIRE_STANDARD_TABLE_FILTER_HPP
#include <memory>
#include <typeindex>
#include <vector>
#include "Spire/Ui/ItemFilter.hpp"
#include "Spire/Ui/TableFilter.hpp"

namespace Spire {

  /**
   * Combines filters applied to individual columns.
   */
  class StandardTableFilter : public TableFilter {
    public:

      /**
       * Constructs a filter from column filters.
       * @param filters Filters in source-column order; null entries disable
       *        filtering. Remaining columns are unfiltered.
       */
      explicit StandardTableFilter(
        std::vector<std::unique_ptr<ItemFilter>> filters);

      /**
       * Constructs a StandardTableFilter.
       * @param types The list of types represented by the table.
       */
      explicit StandardTableFilter(std::vector<std::type_index> types);

      ~StandardTableFilter();

      Filter get_filter(int column) override;
      QWidget* make_filter_widget(int column, QWidget& parent) override;
      bool is_filtered(const TableModel& model, int row) const override;
      boost::signals2::connection connect_filter_signal(
        const FilterSignal::slot_type& slot) const override;

    private:
      struct ColumnFilter;
      struct DateColumnFilter;
      struct EmptyColumnFilter;
      struct ItemColumnFilter;
      mutable FilterSignal m_filter_signal;
      std::vector<std::unique_ptr<ColumnFilter>> m_column_filters;

      void on_filter(int column, Filter filter);
  };
}

#endif
