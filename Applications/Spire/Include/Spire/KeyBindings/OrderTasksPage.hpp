#ifndef SPIRE_ORDER_TASKS_PAGE_HPP
#define SPIRE_ORDER_TASKS_PAGE_HPP
#include <QWidget>
#include "Spire/KeyBindings/OrderTask.hpp"
#include "Spire/KeyBindings/OrderTasksRow.hpp"
#include "Spire/KeyBindings/OrderTasksToTableModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** Displays a page configuring key bindings for order tasks. */
  class OrderTasksPage : public QWidget {
    public:
      
      using Column = OrderTasksRow::Column;

      /**
       * Constructs a OrderTasksPage.
       * @param region_query_model The model used to query region matches.
       * @param model The list model of OrderTask.
       * @param destinations The destination database to use.
       * @param markets The market database to use.
       * @param parent The parent widget.
       */
      OrderTasksPage(std::shared_ptr<ComboBox::QueryModel> region_query_model,
        std::shared_ptr<ListModel<OrderTask>> model,
        Nexus::DestinationDatabase destinations, Nexus::MarketDatabase markets,
        QWidget* parent = nullptr);

      /** Returns the model used to query region matches. */
      const std::shared_ptr<ComboBox::QueryModel>&
        get_region_query_model() const;

      /** Returns the list model. */
      const std::shared_ptr<ListModel<OrderTask>>& get_model() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
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
      std::shared_ptr<ListModel<OrderTask>> m_model;
      Nexus::DestinationDatabase m_destinations;
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<OrderTasksToTableModel> m_order_tasks_table;
      std::shared_ptr<FilteredTableModel> m_filtered_table;
      TableBody* m_table_body;
      SearchBox* m_search_box;
      ContextMenu* m_table_menu;
      std::unique_ptr<FocusObserver> m_table_body_focus_observer;
      std::vector<std::unique_ptr<OrderTasksRow>> m_rows;
      std::unordered_set<std::pair<Nexus::Region, QKeySequence>, RegionKeyHash>
        m_region_key_set;
      std::vector<std::vector<QString>> m_row_text;
      boost::optional<TableView::Index> m_previous_index;
      AddedRow m_added_row;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_sort_connection;
      boost::signals2::scoped_connection m_source_table_operation_connection;
      boost::signals2::scoped_connection m_view_operation_connection;

      QWidget* table_view_builder(const std::shared_ptr<TableModel>& table,
        int row, int column);
      void build_search_text(const TableModel& table);
      void table_view_navigate_next();
      void table_view_navigate_previous();
      void do_search(const QString& query);
      void do_search_excluding_a_row(int excluding_row);
      void do_search_on_all_rows();
      void update_key(const std::shared_ptr<TableModel>& table, int row,
        const Nexus::Region& region, const QKeySequence& key);
      void on_current(const boost::optional<TableView::Index>& index);
      void on_delete_order();
      void on_search(const QString& value);
      void on_sort(int column, TableHeaderItem::Order order);
      void on_source_table_operation(const TableModel::Operation& operation);
      void on_table_body_focus(FocusObserver::State state);
      void on_view_table_operation(const TableModel::Operation& operation);
  };
}

#endif
