#ifndef SPIRE_EDITABLE_TABLE_VIEW_HPP
#define SPIRE_EDITABLE_TABLE_VIEW_HPP
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/FilteredTableModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /**
   * Customizes the PopupBox to propagate mouse events to TableItem to
   * trigger the current signal for TableView.
   */
  class TransparentMouseEventsPopupBox : public QWidget {
    public:

      /** Constructs a TransparentMouseEventsPopupBox. */
      explicit TransparentMouseEventsPopupBox(QWidget& body,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;
      void showEvent(QShowEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      PopupBox* m_popup_box;
      QWidget* m_tip_window;
      bool m_has_sent_event;
  };

  /** Represents an editable TableView. */
  class EditableTableView : public TableView {
    public:

      /**
       * Constructs an EditableTableView.
       * @param table The model of values to display.
       * @param header The model used to display the header.
       * @param table_filter The filter to apply to a column.
       * @param current The current value.
       * @param selection The selection.
       * @param view_builder The ViewBuilder to use.
       * @param comparator The comparison function.
       * @param parent The parent widget.
       */
      EditableTableView(std::shared_ptr<TableModel> table,
        std::shared_ptr<HeaderModel> header,
        std::shared_ptr<TableFilter> table_filter,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection, ViewBuilder view_builder,
        Comparator comparator, QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool focusNextPrevChild(bool next) override;

    private:
      class EditableTableRow;
      TableBody* m_table_body;
      std::unordered_map<QWidget*, std::unique_ptr<EditableTableRow>> m_rows;
      bool m_has_sent_event;
      boost::signals2::scoped_connection m_current_connection;

      QWidget* view_builder(ViewBuilder source_view_builder,
        const std::shared_ptr<TableModel>& table, int row, int column);
      void set_column_cover_mouse_events_transparent();
      void navigate_next();
      void navigate_previous();
      void on_current(const boost::optional<Index>& index);
  };
}

#endif
