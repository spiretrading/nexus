#ifndef SPIRE_STANDARD_TABLE_FILTER_HPP
#define SPIRE_STANDARD_TABLE_FILTER_HPP
#include <typeindex>
#include <vector>
#include "Spire/Ui/TableFilter.hpp"
#include "Spire/Ui/Ui.hpp"

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

      Filter get_filter(int column) override;

      QWidget* make_filter_widget(int column, QWidget& parent) override;

      bool is_filtered(const TableModel& model, int row) const override;

    private:
      std::vector<std::type_index> m_types;
  };
}

#endif
