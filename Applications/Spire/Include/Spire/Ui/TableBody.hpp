#ifndef SPIRE_TABLE_BODY_HPP
#define SPIRE_TABLE_BODY_HPP
#include <deque>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Styles/BasicProperty.hpp"
#include "Spire/Styles/CompositeProperty.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/HoverObserver.hpp"
#include "Spire/Ui/KeyObserver.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/TableCurrentController.hpp"
#include "Spire/Ui/TableSelectionController.hpp"
#include "Spire/Ui/TableViewItemBuilder.hpp"
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

  /** Selects the current row. */
  using CurrentRow = StateSelector<void, struct CurrentRowTag>;

  /** Selects all columns. */
  using Column = StateSelector<void, struct ColumnTag>;

  /** Selects the current column. */
  using CurrentColumn = StateSelector<void, struct CurrentColumnTag>;

  /** Selects the hover item . */
  using HoverItem = StateSelector<void, struct HoverItemTag>;
}

  /** Displays the body of a TableView. */
  class TableBody : public QWidget {
    public:
      using CurrentModel = TableCurrentController::CurrentModel;

      using SelectionModel = TableSelectionController::SelectionModel;

      using Index = TableIndex;

      /**
       * The default item builder which uses a label styled TextBox to display
       * the text representation of its value.
       */
      static QWidget* default_item_builder(
        const std::shared_ptr<TableModel>& table, int row, int column);

      /**
       * Constructs a TableBody.
       * @param table The model of values to display.
       * @param current The current value.
       * @param selection The selection.
       * @param widths The widths of each column.
       * @param item_builder The TableViewItemBuilder to use.
       * @param parent The parent widget.
       */
      TableBody(std::shared_ptr<TableModel> table,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection,
        std::shared_ptr<ListModel<int>> widths,
        TableViewItemBuilder item_builder, QWidget* parent = nullptr);

      /** Returns the table of values displayed. */
      const std::shared_ptr<TableModel>& get_table() const;

      /** Returns the current value. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the selection. */
      const std::shared_ptr<SelectionModel>& get_selection() const;

      /** Returns the TableItem at a given index. */
      TableItem* find_item(const Index& index);

      /**
       * Returns an estimate of how many pixels need to be scrolled for a single
       * row.
       */
      int estimate_scroll_line_height() const;

      /** Show the given column. */
      void show_column(int column);

      /** Hide the given column. */
      void hide_column(int column);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      bool focusNextPrevChild(bool next) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void moveEvent(QMoveEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      struct Styles {
        QColor m_background_color;
        int m_horizontal_spacing;
        int m_vertical_spacing;
        QMargins m_padding;
        QColor m_horizontal_grid_color;
        QColor m_vertical_grid_color;
      };
      struct Cover;
      struct ColumnCover;
      struct RowCover;
      struct Layout;
      struct Painter;
      std::shared_ptr<TableModel> m_table;
      std::unordered_set<Qt::Key> m_keys;
      TableCurrentController m_current_controller;
      TableSelectionController m_selection_controller;
      std::shared_ptr<ListModel<int>> m_widths;
      TableViewItemBuilder m_item_builder;
      std::deque<RowCover*> m_recycled_rows;
      std::vector<ColumnCover*> m_column_covers;
      RowCover* m_current_row;
      boost::optional<Index> m_current_index;
      Styles m_styles;
      bool m_is_transaction;
      int m_operation_counter;
      int m_resize_guard;
      KeyObserver m_key_observer;
      std::unordered_map<TableItem*, HoverObserver> m_hover_observers;
      boost::optional<Index> m_hover_index;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_row_style_connection;
      boost::signals2::scoped_connection m_table_connection;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_selection_connection;
      boost::signals2::scoped_connection m_widths_connection;

      const Layout& get_layout() const;
      Layout& get_layout();
      RowCover* find_row(int index);
      TableItem* find_item(const boost::optional<Index>& index);
      boost::optional<int> get_current_row_index() const;
      Index get_index(const TableItem& item) const;
      int get_column_size() const;
      int estimate_row_height() const;
      int get_left_spacing(int index) const;
      int get_top_spacing(int index) const;
      void add_column_cover(int index, const QRect& geometry);
      void increment_operation_counter();
      void add_row(int index);
      void pre_remove_row(int index);
      void remove_row(int index);
      void move_row(int source, int destination);
      void update_parent();
      RowCover* mount_row(int index);
      RowCover* make_row_cover();
      void destroy(RowCover* row);
      void remove(RowCover& row);
      void mount_visible_rows();
      void unmount_hidden_rows();
      void initialize_visible_region();
      void reset_visible_region();
      void update_visible_region();
      void update_column_covers();
      void update_column_widths();
      bool navigate_next();
      bool navigate_previous();
      void on_item_activated(TableItem& item);
      void on_current(const boost::optional<Index>& current);
      void on_row_selection(const ListModel<int>::Operation& operation);
      void on_hover(TableItem& item, HoverObserver::State state);
      void on_style();
      void on_cover_style(Cover& cover);
      void on_table_operation(const TableModel::Operation& operation);
      void on_widths_update(const ListModel<int>::Operation& operation);
      bool on_key_press(QWidget& target, QKeyEvent& event);
  };
}

#endif
