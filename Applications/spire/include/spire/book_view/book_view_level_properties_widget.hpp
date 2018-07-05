#ifndef BOOK_VIEW_LEVEL_PROPERTIES_WIDGET_HPP
#define BOOK_VIEW_LEVEL_PROPERTIES_WIDGET_HPP
#include <QWidget>
#include "spire/book_view/book_view.hpp"
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

    private:
      void set_color_button_stylesheet(flat_button* button,
        const QColor& color);
  };
}

#endif
