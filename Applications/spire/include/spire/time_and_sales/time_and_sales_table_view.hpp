#ifndef SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#define SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#include <QAbstractItemModel>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>
#include "spire/time_and_sales/time_and_sales_properties.hpp"

namespace spire {

  //! Displays a table with horizontal header and loading widget.
  class time_and_sales_table_view : public QScrollArea {
    public:

      //! Constructs a time_and_sales_table_view
      /*
        \param parent The parent to this widget.
      */
      time_and_sales_table_view(QWidget* parent = nullptr);

      //! Sets the model to get the table data from.
      /*
        \param model The model.
      */
      void set_model(QAbstractItemModel* model);

      //! Sets the time_and_sales_properties of the table.
      /*
        \param properties The properties the table will be updated to have.
      */
      void set_properties(const time_and_sales_properties& properties);

      //! Hides the loading widget.
      void hide_loading_widget();

      //! Displays the loading widget.
      void show_loading_widget();

    protected:
      bool event(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

    private:
      QVBoxLayout* m_layout;
      QHeaderView* m_header;
      QWidget* m_header_padding;
      QTableView* m_table;
      QTimer m_h_scroll_bar_timer;
      QTimer m_v_scroll_bar_timer;
      snapshot_loading_widget* m_loading_widget;

      void fade_out_horizontal_scroll_bar();
      void fade_out_vertical_scroll_bar();
      int table_height_with_additional_row();
      bool within_horizontal_scroll_bar(const QPoint& pos);
      bool within_vertical_scroll_bar(const QPoint& pos);
      void on_header_resize(int index, int old_size, int new_size);
      void on_header_move(int logical_index, int old_index, int new_index);
      void on_horizontal_slider_value_changed(int new_value);
      void on_rows_about_to_be_inserted();
  };
}

#endif
