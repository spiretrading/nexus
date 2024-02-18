#ifndef SPIRE_ORDER_TASKS_PAGE_HPP
#define SPIRE_ORDER_TASKS_PAGE_HPP
#include <QWidget>
#include "Spire/KeyBindings/OrderTask.hpp"
#include "Spire/KeyBindings/OrderTasksRow.hpp"
#include "Spire/KeyBindings/OrderTasksToTableModel.hpp"
#include "Spire/KeyBindings/TableMatchCache.hpp"
#include "Spire/KeyBindings/TableRowDragDrop.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** Displays a page configuring key bindings for order tasks. */
  class OrderTasksPage : public QWidget {
    public:
      
      using Column = OrderTasksRow::Column;

      /**
       * Constructs a OrderTasksPage.
       * @param region_query_model The model used to query region matches.
       * @param order_tasks The list of OrderTasks.
       * @param destinations The destination database to use.
       * @param markets The market database to use.
       * @param parent The parent widget.
       */
      OrderTasksPage(std::shared_ptr<ComboBox::QueryModel> region_query_model,
        std::shared_ptr<ListModel<OrderTask>> order_tasks,
        Nexus::DestinationDatabase destinations, Nexus::MarketDatabase markets,
        QWidget* parent = nullptr);

      /** Returns the model used to query region matches. */
      const std::shared_ptr<ComboBox::QueryModel>&
        get_region_query_model() const;

      /** Returns the list of OrderTasks. */
      const std::shared_ptr<ListModel<OrderTask>>& get_order_tasks() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;
      bool focusNextPrevChild(bool next) override;

    private:
      struct RegionKeyHash {
        std::size_t operator()(
          const std::pair<Nexus::Region, QKeySequence>& region_key) const;
      };
      struct AddedRow {
        int m_source_index;
        bool m_is_filtered;

        AddedRow();
      };
      std::shared_ptr<ComboBox::QueryModel> m_region_query_model;
      std::shared_ptr<ListModel<OrderTask>> m_order_tasks;
      Nexus::DestinationDatabase m_destinations;
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<OrderTasksToTableModel> m_order_tasks_table;
      std::shared_ptr<FilteredTableModel> m_filtered_table;
      TableBody* m_table_body;
      SearchBox* m_search_box;
      ContextMenu* m_table_menu;
      std::shared_ptr<ArrayListModel<std::shared_ptr<TableRow>>> m_rows;
      std::unordered_set<std::pair<Nexus::Region, QKeySequence>, RegionKeyHash>
        m_region_key_set;
      boost::optional<TableView::Index> m_previous_index;
      QWidget* m_previous_row;
      QWidget* m_table_next_focus_widget;
      AddedRow m_added_row;
      std::unique_ptr<TableMatchCache> m_table_match_cache;
      std::unique_ptr<TableRowDragDrop> m_table_row_drag_drop;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_sort_connection;
      boost::signals2::scoped_connection m_list_operation_connection;
      boost::signals2::scoped_connection m_source_table_operation_connection;
      boost::signals2::scoped_connection m_view_operation_connection;

      QWidget* table_view_builder(const std::shared_ptr<TableModel>& table,
        int row, int column);
      TableMatchCache::Matcher table_matcher_builder(
        const std::shared_ptr<TableModel>& table, int row, int column);
      void table_view_navigate_next();
      void table_view_navigate_previous();
      void do_search(const QString& query);
      void do_search_excluding_a_row(QWidget* excluding_row);
      void do_search_on_all_rows();
      void update_key(const std::shared_ptr<TableModel>& table, int row,
        const Nexus::Region& region, const QKeySequence& key);
      void on_current(const boost::optional<TableView::Index>& index);
      void on_delete_order();
      void on_search(const QString& value);
      void on_sort(int column, TableHeaderItem::Order order);
      void on_list_operation(const ListModel<OrderTask>::Operation& operation);
      void on_source_table_operation(const TableModel::Operation& operation);
      void on_view_table_operation(const TableModel::Operation& operation);
      void on_focus_changed(QWidget* old, QWidget* now);
  };
}

#endif
