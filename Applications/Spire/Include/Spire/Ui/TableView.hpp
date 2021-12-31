#ifndef SPIRE_TABLE_VIEW_HPP
#define SPIRE_TABLE_VIEW_HPP
#include <functional>
#include <memory>
#include <QWidget>
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a table of values represented by TableItems arranged along a grid.
   */
  class TableView : public QWidget {
    public:

      /**
       * The type of function used to build a QWidget representing a value.
       * @param table The table of values being displayed.
       * @param row The row of the specific value to be displayed.
       * @param column The column of the specific value to be displayed.
       * @return The QWidget that shall be used to display the value in the
       *         <i>table</i> at the given <i>row</i> and <i>column</i>.
       */
      using ViewBuilder = std::function<QWidget* (
        const std::shared_ptr<TableModel>& table, int row, int column)>;

      /** The model representing the header. */
      using HeaderModel = ListModel<TableHeaderItem::Model>;

      /**
       * The default view builder which uses a label styled TextBox to display
       * the text representation of its value.
       */
      static QWidget* default_view_builder(
        const std::shared_ptr<TableModel>& table, int row, int column);

      /**
       * Constructs a TableView using default local models and a default view
       * builder.
       * @param table The model of values to display.
       * @param header The model used to display the header.
       * @param parent The parent widget.
       */
      explicit TableView(std::shared_ptr<TableModel> table,
        std::shared_ptr<HeaderModel> header, QWidget* parent = nullptr);

      /**
       * Constructs a TableView using default local models.
       * @param table The model of values to display.
       * @param header The model used to display the header.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      TableView(std::shared_ptr<TableModel> table,
        std::shared_ptr<HeaderModel> header, ViewBuilder view_builder,
        QWidget* parent = nullptr);

      /** Returns the table of values displayed. */
      const std::shared_ptr<TableModel>& get_table() const;

    private:
      std::shared_ptr<TableModel> m_table;
      ViewBuilder m_view_builder;
  };
}

#endif
