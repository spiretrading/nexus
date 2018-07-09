#ifndef BOOK_VIEW_LEVEL_PROPERTIES_WIDGET_HPP
#define BOOK_VIEW_LEVEL_PROPERTIES_WIDGET_HPP
#include <QListWidget>
#include <QWidget>
#include "spire/book_view/book_view.hpp"
#include "spire/ui/flat_button.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays/modifies the properties used to display a book view's price
  //! levels.
  class book_view_level_properties_widget : public QWidget {
    public:

      //! Constructs a book_view_level_properties_widget.
      /*!
        \param properties The properties to display.
        \param parent The parent widget.
      */
      book_view_level_properties_widget(const book_view_properties& properties,
        QWidget* parent = nullptr);

      //! Applies the properties represented by this widget to an instance.
      /*!
        \param properties The instance to apply the properties to.
      */
      void apply(book_view_properties& properties) const;

    protected:
      void showEvent(QShowEvent* event) override;

    private:
      QListWidget* m_band_list_widget;
      QString m_band_list_stylesheet;
      flat_button* m_band_color_button;
      flat_button* m_gradient_start_button;
      flat_button* m_gradient_end_button;

      void set_color_button_stylesheet(flat_button* button,
        const QColor& color);
      void populate_band_list(int num_items);
      void update_band_list_font(const QFont& font);
      void update_band_list_gradient();
      void update_band_list_stylesheet(int index);
      void on_band_color_button_clicked();
      void on_change_font_button_clicked();
      void on_gradient_apply_button_clicked();
      void on_gradient_end_button_clicked();
      void on_gradient_start_button_clicked();
  };
}

#endif
