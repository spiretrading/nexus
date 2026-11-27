#ifndef SPIRE_TABLE_GROUPING_BAR_HPP
#define SPIRE_TABLE_GROUPING_BAR_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/GroupedTableModel.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/TextBox.hpp"

class QHBoxLayout;

namespace Spire {
  class ContextMenu;

  /** Selects grouping columns and displays them from outermost to innermost. */
  class TableGroupingBar : public QWidget {
    public:

      /**
       * Constructs a grouping bar for a table.
       * @param table The table whose grouping is edited.
       * @param columns The column headers in source order.
       */
      TableGroupingBar(std::shared_ptr<GroupedTableModel> table,
        const std::shared_ptr<ListModel<TableHeaderItem::Model>>& columns,
        QWidget* parent = nullptr);

    private:
      using Header = ListModel<TableHeaderItem::Model>;

      struct Column {
        std::shared_ptr<TextModel> m_name;
        std::shared_ptr<BooleanModel> m_selected;
        boost::signals2::scoped_connection m_connection;
      };
      std::shared_ptr<GroupedTableModel> m_table;
      std::vector<Column> m_columns;
      QHBoxLayout* m_tags;
      boost::signals2::scoped_connection m_columns_connection;
      boost::signals2::scoped_connection m_grouping_connection;

      void populate_menu(ContextMenu& menu);
      void on_columns_operation(ContextMenu& menu,
        const Header::Operation& operation);
      void on_select(int column, bool selected);
      void on_grouping(const std::vector<int>& grouping);
  };
}

#endif
