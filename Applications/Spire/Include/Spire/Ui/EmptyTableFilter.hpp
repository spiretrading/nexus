#ifndef SPIRE_EMPTY_TABLE_FILTER_HPP
#define SPIRE_EMPTY_TABLE_FILTER_HPP
#include "Spire/Ui/TableFilter.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a TableFilter where no rows are ever filtered. */
  class EmptyTableFilter : public TableFilter {
    public:

      /** Constructs an EmptyTableFilter. */
      EmptyTableFilter() = default;

      Filter get_filter(int column) override;

      QWidget* make_filter_widget(int column, QWidget& parent) override;

      bool is_filtered(const TableModel& model, int row) const override;

      boost::signals2::connection connect_filter_signal(
        const FilterSignal::slot_type& slot) const override;
  };
}

#endif
