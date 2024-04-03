#ifndef SPIRE_TABLE_VIEW_HPP
#define SPIRE_TABLE_VIEW_HPP
#include <memory>
#include <QWidget>
#include "Spire/Spire/SortedTableModel.hpp"
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

      using SelectionModel = TableBody::SelectionModel;

      using SortSignal = TableHeader::SortSignal;

      using Comparator = SortedTableModel::Comparator;

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
       * @param selection The selection.
       * @param view_builder The ViewBuilder to use.
       * @param comparator A comparison function.
       * @param parent The parent widget.
       */
      TableView(std::shared_ptr<TableModel> table,
        std::shared_ptr<HeaderModel> header,
        std::shared_ptr<TableFilter> filter,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection, ViewBuilder view_builder,
        Comparator comparator, QWidget* parent = nullptr);

      /** Returns the table of values displayed. */
      const std::shared_ptr<TableModel>& get_table() const;

      /** Returns the current value. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the selection. */
      const std::shared_ptr<SelectionModel>& get_selection() const;

      /** Returns the table header. */
      TableHeader& get_header();

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
      TableHeader* m_header_view;
      TableBody* m_body;
      int m_horizontal_spacing;
      int m_vertical_spacing;
      ScrollBox* m_scroll_box;
      boost::signals2::scoped_connection m_filter_connection;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_body_style_connection;

      bool is_filtered(const TableModel& model, int row);
      void on_order_update(int index, TableHeaderItem::Order order);
      void on_filter_clicked(int index);
      void on_filter(int column, TableFilter::Filter filter);
      void on_current(const boost::optional<Index>& current);
      void on_body_style();
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

      /**
       * Adds a header item.
       * @param name The name of the column.
       */
      TableViewBuilder& add_header_item(QString name);

      /**
       * Adds a header item.
       * @param name The name of the column.
       * @param short_name The short form name of the column.
       */
      TableViewBuilder& add_header_item(QString name, QString short_name);

      /**
       * Adds a header item.
       * @param name The name of the column.
       * @param short_name The short form name of the column.
       * @param filter How the column is filtered.
       */
      TableViewBuilder& add_header_item(
        QString name, QString short_name, TableFilter::Filter filter);

      /**
       * Adds a header item.
       * @param name The name of the column.
       * @param filter How the column is filtered.
       */
      TableViewBuilder& add_header_item(
        QString name, TableFilter::Filter filter);

      /** Sets the filter to apply. */
      TableViewBuilder& set_filter(const std::shared_ptr<TableFilter>& filter);

      /** Applies the standard filter. */
      TableViewBuilder& set_standard_filter();

      /** Sets the current index. */
      TableViewBuilder& set_current(
        const std::shared_ptr<TableView::CurrentModel>& current);

      /** Sets the selection. */
      TableViewBuilder& set_selection(
        const std::shared_ptr<TableView::SelectionModel>& selection);

      /** Sets the ViewBuilder to use. */
      TableViewBuilder& set_view_builder(
        const TableView::ViewBuilder& view_builder);

      /** Sets the Comparator to use. */
      TableViewBuilder& set_comparator(TableView::Comparator comparator);

      /** Makes a new TableView using the current state of this builder. */
      TableView* make() const;

    private:
      std::shared_ptr<TableModel> m_table;
      QWidget* m_parent;
      std::shared_ptr<TableView::HeaderModel> m_header;
      std::shared_ptr<TableFilter> m_filter;
      std::shared_ptr<TableView::CurrentModel> m_current;
      std::shared_ptr<TableView::SelectionModel> m_selection;
      TableView::ViewBuilder m_view_builder;
      TableView::Comparator m_comparator;
  };
}

#endif
