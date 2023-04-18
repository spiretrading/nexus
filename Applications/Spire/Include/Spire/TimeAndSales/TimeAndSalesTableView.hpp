#ifndef SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#define SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#include <QWidget>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindowProperties.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {
namespace Styles {

  using PullIndicator = StateSelector<void, struct PullIndicatorSelectorTag>;

  /** Selects a indicator to indicate the pull request is being delayed. */
  using PullDelayed = StateSelector<void, struct PullDelayedSelectorTag>;
}

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

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      //struct TimeAndSalesTableViewModel;
      struct HeaderItemProperties {
        bool m_is_visible;
        Qt::Alignment m_text_align;
        int m_width;
      };
      std::shared_ptr<TimeAndSalesTableModel> m_table;
      //std::shared_ptr<TimeAndSalesTableViewModel> m_table;
      TableHeader* m_table_header;
      TableBody* m_table_body;
      QWidget* m_pull_indicator;
      ScrollBox* m_scroll_box;
      QTimer* m_timer;
      ContextMenu* m_table_columns_menu;
      std::vector<HeaderItemProperties> m_header_item_properties;
      bool m_is_loading;
      int m_resize_index;
      QPoint m_resize_position;
      boost::signals2::scoped_connection m_begin_loading_connection;
      boost::signals2::scoped_connection m_end_loading_connection;

      QWidget* table_view_builder(
        const std::shared_ptr<TableModel>& table, int row, int column);
      void make_header_item_properties();
      void make_table_columns_sub_menu();
      void customize_table_header();
      int get_next_sibling_index(int index);
      std::tuple<int, int> get_next_sibling(int index);
      void resize_column_widths();
      void on_begin_loading();
      void on_end_loading();
      void on_scroll_position(int position);
      void on_timer_expired();
      void on_start_resize(int index);
      void on_end_resize(int index);
  };
}

#endif
