#ifndef BOOK_VIEW_HIGHLIGHT_PROPERTIES_WIDGET_HPP
#define BOOK_VIEW_HIGHLIGHT_PROPERTIES_WIDGET_HPP
#include <QListWidget>
#include <QWidget>
#include "spire/book_view/book_view.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays/modifies the properties used to highlight a book view's quotes.
  class book_view_highlight_properties_widget : public QWidget {
    public:

      //! Constructs a book_view_highlight_properties_widget.
      /*!
        \param properties The properties to display.
        \param parent The parent widget.
      */
      book_view_highlight_properties_widget(
        const book_view_properties& properties, QWidget* parent = nullptr);

      //! Applies the properties represented by this widget to an instance.
      /*!
        \param properties The instance to apply the properties to.
      */
      void apply(book_view_properties& properties) const;

    protected:
      void showEvent(QShowEvent* event) override;

    private:
      QListWidget* m_markets_list_widget;
      check_box* m_highlight_none_check_box;
      check_box* m_highlight_top_level_check_box;
      check_box* m_highlight_all_levels_check_box;
      flat_button* m_market_highlight_color_button;
      check_box* m_hide_orders_check_box;
      check_box* m_display_orders_check_box;
      check_box* m_highlight_orders_check_box;
      flat_button* m_order_highlight_color_button;

      void update_color_button_stylesheet(flat_button* button,
        const QColor& color);
      void update_market_widgets(int selected_item_index);
      void update_market_list_stylesheet(int selected_item_index);
      void on_market_highlight_color_button_clicked();
      void on_order_highlight_color_button_clicked();
      void on_highlight_none_check_box_checked(int state);
      void on_highlight_top_level_check_box_checked(int state);
      void on_highlight_all_levels_check_box_checked(int state);
  };
}

#endif
