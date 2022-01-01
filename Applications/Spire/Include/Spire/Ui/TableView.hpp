#ifndef SPIRE_TABLE_VIEW_HPP
#define SPIRE_TABLE_VIEW_HPP
#include <QWidget>
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
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
      TableBody* m_body;
  };
}

#endif
