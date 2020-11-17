#ifndef SPIRE_TOGGLE_BUTTON_TEST_WIDGET_HPP
#define SPIRE_TOGGLE_BUTTON_TEST_WIDGET_HPP
#include <QWidget>

namespace Spire {

  //! Represents a widget for testing a ToggleButton.
  class ToggleButtonTestWidget : public QWidget {
    public:

      //! Constructs a ToggleButtonTestwidget.
      /*!
        \param parent The parent widget.
      */
      explicit ToggleButtonTestWidget(QWidget* parent = nullptr);
  };
}

#endif
