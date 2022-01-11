#ifndef SPIRE_TABLE_VIEW_HPP
#define SPIRE_TABLE_VIEW_HPP
#include <memory>
#include <QWidget>
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableHeader.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a table of values represented by TableItems arranged along a grid.
   */
  class TableView : public QWidget {
    public:

      /** The model representing the header. */
      using HeaderModel = ListModel<TableHeaderItem::Model>;

      using ViewBuilder = TableBody::ViewBuilder;

      using Index = TableBody::Index;

      using CurrentModel = TableBody::CurrentModel;

      using SortSignal = TableHeader::SortSignal;

      /**
       * The default view builder which uses a label styled TextBox to display
       * the text representation of its value.
       */
      static QWidget* default_view_builder(
        const std::shared_ptr<TableModel>& table, int row, int column);

      /**
       * Constructs a TableView.
       * @param table The model of values to display.
       * @param header The model used to display the header.
       * @param filter The filter to apply.
       * @param current The current value.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      TableView(std::shared_ptr<TableModel> table,
        std::shared_ptr<HeaderModel> header,
        std::shared_ptr<TableFilter> filter,
        std::shared_ptr<CurrentModel> current, ViewBuilder view_builder,
        QWidget* parent = nullptr);

      /** Returns the table of values displayed. */
      const std::shared_ptr<TableModel>& get_table() const;

      /** Returns the current value. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Connects a slot to the sort signal. */
      boost::signals2::connection connect_sort_signal(
        const SortSignal::slot_type& slot) const;

    private:
      mutable SortSignal m_sort_signal;
      std::shared_ptr<TableModel> m_table;
      std::shared_ptr<FilteredTableModel> m_filtered_table;
      std::shared_ptr<SortedTableModel> m_sorted_table;
      std::shared_ptr<HeaderModel> m_header;
      std::shared_ptr<TableFilter> m_filter;
      TableBody* m_body;
      boost::signals2::scoped_connection m_filter_connection;

      bool is_filtered(const TableModel& model, int row);
      void on_order_update(int index, TableHeaderItem::Order order);
      void on_filter_clicked(int index);
      void on_filter(int column, TableFilter::Filter filter);
  };

  /**
   * A builder for a TableView, using local models as default values as well as
   * a standard filter model.
   */
  class TableViewBuilder {
    public:

      /**
       * Constructs a TableViewBuilder. The initial builder uses the supplied
       * table as the model, an empty header, no columns can be filtered,
       * all columns are sorted along the default sort order, a local model is
       * used for the current value and the default view builder is used.
       * @param table The model of values to display.
       * @param parent The parent widget.
       */
      explicit TableViewBuilder(
        std::shared_ptr<TableModel> table, QWidget* parent = nullptr);

      /** Sets the model of the header to display. */
      TableViewBuilder& set_header(
        const std::shared_ptr<TableView::HeaderModel>& header);

      /** Sets the filter to apply. */
      TableViewBuilder& set_filter(const std::shared_ptr<TableFilter>& filter);

      /** Applies the standard filter. */
      TableViewBuilder& set_standard_filter();

      /** Sets the current value. */
      TableViewBuilder& set_current(
        const std::shared_ptr<TableView::CurrentModel>& current);

      /** Sets the ViewBuilder to use. */
      TableViewBuilder& set_view_builder(
        const TableView::ViewBuilder& view_builder);

      /** Makes a new TableView using the current state of this builder. */
      TableView* make() const;

    private:
      std::shared_ptr<TableModel> m_table;
      QWidget* m_parent;
      std::shared_ptr<TableView::HeaderModel> m_header;
      std::shared_ptr<TableFilter> m_filter;
      std::shared_ptr<TableView::CurrentModel> m_current;
      TableView::ViewBuilder m_view_builder;
  };
}

#endif
