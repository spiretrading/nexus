#ifndef SPIRE_EDITABLE_TABLE_VIEW_HPP
#define SPIRE_EDITABLE_TABLE_VIEW_HPP
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a TableView that supports deleting rows and editing table cells.
   */
  class EditableTableView : public TableView {
    public:

      /**
       * Constructs an EditableTableView.
       * @param table The model of values to display.
       * @param header The model used to display the header.
       * @param table_filter The filter to apply to a column.
       * @param current The current value.
       * @param selection The selection.
       * @param item_builder The TableViewItemBuilder to use.
       * @param comparator The comparison function.
       * @param parent The parent widget.
       */
      EditableTableView(std::shared_ptr<TableModel> table,
        std::shared_ptr<HeaderModel> header,
        std::shared_ptr<TableFilter> table_filter,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection,
        TableViewItemBuilder item_builder, Comparator comparator,
        QWidget* parent = nullptr);

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      bool focusNextPrevChild(bool next) override;

    private:
      struct ItemBuilder;
      bool m_is_processing_key;

      void delete_row(const TableRowIndexTracker& row);
      bool navigate_next();
      bool navigate_previous();
  };

  /**
   * A builder for an EditableTableView, using local models as default values as
   * well as a standard filter model.
   */
  class EditableTableViewBuilder {
    public:

      /**
       * Constructs an EditableTableViewBuilder. The initial builder uses the
       * supplied table as the model, an empty header, no columns can be
       * filtered, all columns are sorted along the default sort order, a local
       * model is used for the current value and the default view builder is
       * used.
       * @param table The model of values to display.
       * @param parent The parent widget.
       */
      explicit EditableTableViewBuilder(
        std::shared_ptr<TableModel> table, QWidget* parent = nullptr);

      /** Sets the model of the header to display. */
      EditableTableViewBuilder& set_header(
        const std::shared_ptr<TableView::HeaderModel>& header);

      /**
       * Adds a header item.
       * @param name The name of the column.
       */
      EditableTableViewBuilder& add_header_item(QString name);

      /**
       * Adds a header item.
       * @param name The name of the column.
       * @param short_name The short form name of the column.
       */
      EditableTableViewBuilder& add_header_item(
        QString name, QString short_name);

      /**
       * Adds a header item.
       * @param name The name of the column.
       * @param short_name The short form name of the column.
       * @param filter How the column is filtered.
       */
      EditableTableViewBuilder& add_header_item(
        QString name, QString short_name, TableFilter::Filter filter);

      /**
       * Adds a header item.
       * @param name The name of the column.
       * @param filter How the column is filtered.
       */
      EditableTableViewBuilder& add_header_item(
        QString name, TableFilter::Filter filter);

      /** Sets the filter to apply. */
      EditableTableViewBuilder& set_filter(
        const std::shared_ptr<TableFilter>& filter);

      /** Applies the standard filter. */
      EditableTableViewBuilder& set_standard_filter();

      /** Sets the current index. */
      EditableTableViewBuilder& set_current(
        const std::shared_ptr<TableView::CurrentModel>& current);

      /** Sets the selection. */
      EditableTableViewBuilder& set_selection(
        const std::shared_ptr<TableView::SelectionModel>& selection);

      /** Sets the TableViewItemBuilder to use. */
      EditableTableViewBuilder& set_item_builder(
        const TableViewItemBuilder& item_builder);

      /** Sets the Comparator to use. */
      EditableTableViewBuilder& set_comparator(
        TableView::Comparator comparator);

      /** Makes a new TableView using the current state of this builder. */
      EditableTableView* make() const;

    private:
      std::shared_ptr<TableModel> m_table;
      QWidget* m_parent;
      std::shared_ptr<TableView::HeaderModel> m_header;
      std::shared_ptr<TableFilter> m_filter;
      std::shared_ptr<TableView::CurrentModel> m_current;
      std::shared_ptr<TableView::SelectionModel> m_selection;
      TableViewItemBuilder m_item_builder;
      TableView::Comparator m_comparator;
  };
}

#endif
