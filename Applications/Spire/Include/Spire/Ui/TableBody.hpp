#ifndef SPIRE_TABLE_BODY_HPP
#define SPIRE_TABLE_BODY_HPP
#include <functional>
#include <memory>
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Styles/BasicProperty.hpp"
#include "Spire/Styles/CompositeProperty.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** The horizontal spacing between items. */
  using HorizontalSpacing = BasicProperty<int, struct HorizontalSpacingTag>;

  /** The vertical spacing between items. */
  using VerticalSpacing = BasicProperty<int, struct VerticalSpacingTag>;

  /** The spacing, both vertical and horizontal, between items. */
  using Spacing = CompositeProperty<HorizontalSpacing, VerticalSpacing>;

  /** Sets the spacing between items. */
  Spacing spacing(int spacing);

  /** The color used to paint the horizontal grid lines. */
  using HorizontalGridColor =
    BasicProperty<QColor, struct HorizontalGridColorTag>;

  /** The color used to paint the vertical grid lines. */
  using VerticalGridColor = BasicProperty<QColor, struct VerticalGridColorTag>;

  /** The color used to paint the grid lines. */
  using GridColor = CompositeProperty<HorizontalGridColor, VerticalGridColor>;

  /** Sets the color used for the grid lines. */
  GridColor grid_color(QColor color);

  /** Selects all rows. */
  using Row = StateSelector<void, struct RowTag>;
}

  /** Displays the body of a TableView. */
  class TableBody : public QWidget {
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

      /** Stores an index to a value. */
      struct Index {

        /** The row being indexed. */
        int m_row;

        /** The column being indexed. */
        int m_column;
      };

      /** The type of model to the index of the current value. */
      using CurrentModel = ValueModel<boost::optional<Index>>;

      /**
       * The default view builder which uses a label styled TextBox to display
       * the text representation of its value.
       */
      static QWidget* default_view_builder(
        const std::shared_ptr<TableModel>& table, int row, int column);

      /**
       * Constructs a TableBody using default local models and a default view
       * builder.
       * @param table The model of values to display.
       * @param widths The widths of each column.
       * @param parent The parent widget.
       */
      explicit TableBody(std::shared_ptr<TableModel> table,
        std::shared_ptr<ListModel<int>> widths, QWidget* parent = nullptr);

      /**
       * Constructs a TableBody using a default view builder.
       * @param table The model of values to display.
       * @param current The current value.
       * @param widths The widths of each column.
       * @param parent The parent widget.
       */
      explicit TableBody(std::shared_ptr<TableModel> table,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<ListModel<int>> widths, QWidget* parent = nullptr);

      /**
       * Constructs a TableBody using default local models.
       * @param table The model of values to display.
       * @param widths The widths of each column.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      TableBody(std::shared_ptr<TableModel> table,
        std::shared_ptr<ListModel<int>> widths, ViewBuilder view_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a TableBody.
       * @param table The model of values to display.
       * @param current The current value.
       * @param widths The widths of each column.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      TableBody(std::shared_ptr<TableModel> table,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<ListModel<int>> widths, ViewBuilder view_builder,
        QWidget* parent = nullptr);

      /** Returns the table of values displayed. */
      const std::shared_ptr<TableModel>& get_table() const;

      /** Returns the current value. */
      const std::shared_ptr<CurrentModel>& get_current() const;

    protected:
      bool event(QEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      struct Styles {
        int m_horizontal_spacing;
        int m_vertical_spacing;
        QColor m_horizontal_grid_color;
        QColor m_vertical_grid_color;
      };
      struct RowCover;
      struct BoxStyles {
        QColor m_background_color;
      };
      std::shared_ptr<TableModel> m_table;
      std::shared_ptr<CurrentModel> m_current;
      std::shared_ptr<ListModel<int>> m_widths;
      ViewBuilder m_view_builder;
      std::vector<RowCover*> m_row_covers;
      Styles m_styles;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_row_style_connection;
      boost::signals2::scoped_connection m_table_connection;
      boost::signals2::scoped_connection m_widths_connection;

      void on_style();
      void on_row_cover_style(RowCover& row_cover);
      void on_table_operation(const TableModel::Operation& operation);
      void on_widths_update(const ListModel<int>::Operation& operation);
  };
}

#endif
