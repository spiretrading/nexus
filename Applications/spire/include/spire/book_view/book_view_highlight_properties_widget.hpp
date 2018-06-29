#ifndef BOOK_VIEW_HIGHLIGHT_PROPERTIES_WIDGET_HPP
#define BOOK_VIEW_HIGHLIGHT_PROPERTIES_WIDGET_HPP
#include <QWidget>
#include "spire/book_view/book_view.hpp"

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
  };
}

#endif
