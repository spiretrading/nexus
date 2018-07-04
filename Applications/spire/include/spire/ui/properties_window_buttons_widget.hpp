#ifndef PROPERTIES_WINDOW_BUTTONS_WIDGET_HPP
#define PROPERTIES_WINDOW_BUTTONS_WIDGET_HPP
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays the button group for a properties window.
  class properties_window_buttons_widget : public QWidget {
    public:

      properties_window_buttons_widget(QWidget* parent = nullptr);
  };
}

#endif
