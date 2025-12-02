#ifndef SPIRE_STANDARD_TABLE_FILTER_HPP
#define SPIRE_STANDARD_TABLE_FILTER_HPP
#include <memory>
#include <typeindex>
#include <vector>
#include "Spire/Ui/TableFilter.hpp"

namespace Spire {

  /**
   * Implements a TableFilter that maps column types to a standard set of
   * FilterPanels.
   */
  class StandardTableFilter : public TableFilter {
    public:

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
      struct EmptyColumnFilter;
      template<typename T> struct ScalarColumnFilter;
      mutable FilterSignal m_filter_signal;
      std::vector<std::unique_ptr<ColumnFilter>> m_column_filters;

      void on_filter(int column, Filter filter);
  };
}

#endif
