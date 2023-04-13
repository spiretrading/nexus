#ifndef SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#define SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#include <QWidget>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindowProperties.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** Display the TableView to represent time and sales. */
  class TimeAndSalesTableView : public QWidget {
    public:

      using Index = TableView::Index;
      using Column = TimeAndSalesTableModel::Column;

      /**
       * Constructs a TimeAndSalesTableView.
       * @param table The time and sale to represent.
       * @param parent The parent widget.
       */
      explicit TimeAndSalesTableView(std::shared_ptr<TimeAndSalesTableModel> table,
        QWidget* parent = nullptr);

      /* Returns the table. */
      const std::shared_ptr<TimeAndSalesTableModel>& get_table() const;

      /** Returns the TableItem at a specified index. */
      const TableItem* get_item(Index index) const;

      /** Returns the TableItem at a specified index. */
      TableItem* get_item(Index index);

    private:
      std::shared_ptr<TimeAndSalesTableModel> m_table;
      TableHeader* m_table_header;
      TableBody* m_table_body;

      QWidget* table_view_builder(const std::shared_ptr<TableModel>& table, int row, int column);
      void align_header_item_right(Column column);
      void customize_table_header();
  };
}

#endif
