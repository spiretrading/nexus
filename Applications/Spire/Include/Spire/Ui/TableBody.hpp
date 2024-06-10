#ifndef SPIRE_TABLE_BODY_HPP
#define SPIRE_TABLE_BODY_HPP
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

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void moveEvent(QMoveEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
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
      struct RowCover;
      struct ColumnCover;
      struct Painter;
      struct BoxStyles {
        QColor m_background_color;
      };
      std::shared_ptr<TableModel> m_table;
      std::unordered_set<Qt::Key> m_keys;
      TableCurrentController m_current_controller;
      TableSelectionController m_selection_controller;
      boost::optional<int> m_current_row_index;
      std::shared_ptr<ListModel<int>> m_widths;
      TableViewItemBuilder m_item_builder;
      std::vector<ColumnCover*> m_column_covers;
      int m_top_index;
      QSpacerItem* m_top_spacer;
      QSpacerItem* m_bottom_spacer;
      RowCover* m_current_row;
      Styles m_styles;
      std::unordered_map<TableItem*, HoverObserver> m_hover_observers;
      boost::optional<Index> m_hover_index;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_row_style_connection;
      boost::signals2::scoped_connection m_table_connection;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_widths_connection;

      RowCover* find_row(int index);
      TableItem* find_item(const boost::optional<Index>& index);
      RowCover* get_current_row();
      TableItem* get_current_item();
      int visible_count() const;
      bool is_visible(int index) const;
      Index get_index(const TableItem& item) const;
      int get_column_size() const;
      int estimate_row_height() const;
      int get_left_spacing(int index) const;
      int get_top_spacing(int index) const;
      void add_column_cover(int index, const QRect& geometry);
      void add_row(int index);
      void remove_row(int index);
      void move_row(int source, int destination);
      void update_parent();
      RowCover* mount_row(
        int index, int layout_index, std::vector<RowCover*>& unmounted_rows);
      RowCover* mount_row(int index, int layout_index);
      void remove(RowCover& row);
      void update_spacer(QSpacerItem*& spacer, int hidden_row_count);
      void update_spacers();
      void mount_visible_rows(std::vector<RowCover*>& unmounted_rows);
      std::vector<RowCover*> unmount_hidden_rows();
      void initialize_visible_region();
      void reset_visible_region(
        int total_height, std::vector<RowCover*>& unmounted_rows);
      void update_visible_region();
      void on_item_activated(TableItem& item);
      void on_current(const boost::optional<Index>& previous,
        const boost::optional<Index>& current);
      void on_row_selection(const ListModel<int>::Operation& operation);
      void on_hover(TableItem& item, HoverObserver::State state);
      void on_style();
      void on_cover_style(Cover& cover);
      void on_table_operation(const TableModel::Operation& operation);
      void on_widths_update(const ListModel<int>::Operation& operation);
  };
}

#endif
