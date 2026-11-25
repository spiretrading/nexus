#ifndef SPIRE_TABLE_GROUPING_BAR_HPP
#define SPIRE_TABLE_GROUPING_BAR_HPP
#include <QStringList>
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/GroupedTableModel.hpp"

class QHBoxLayout;

namespace Spire {

  /** Selects grouping columns and displays them from outermost to innermost. */
  class TableGroupingBar : public QWidget {
    public:

      /**
       * Constructs a grouping bar for a table.
       * @param table The table whose grouping is edited.
       * @param columns The column names in source order.
       */
      TableGroupingBar(std::shared_ptr<GroupedTableModel> table,
        const QStringList& columns, QWidget* parent = nullptr);

    private:
      struct Column {
        QString m_name;
        std::shared_ptr<BooleanModel> m_selected;
        boost::signals2::scoped_connection m_connection;
      };
      std::shared_ptr<GroupedTableModel> m_table;
      std::vector<Column> m_columns;
      QHBoxLayout* m_tags;
      boost::signals2::scoped_connection m_grouping_connection;

      void on_select(int column, bool selected);
      void on_grouping(const std::vector<int>& grouping);
  };
}

#endif
