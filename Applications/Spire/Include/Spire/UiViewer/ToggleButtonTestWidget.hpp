#ifndef SPIRE_TOGGLE_BUTTON_TEST_WIDGET_HPP
#define SPIRE_TOGGLE_BUTTON_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include <QWidget>

namespace Spire {

  //! Represents a widget for testing a ToggleButton.
  class ToggleButtonTestWidget : public QWidget {
    public:

      //! Constructs a ToggleButtonTestwidget.
      /*!
        \param parent The parent widget.
      */
      ToggleButtonTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      QLabel* m_pressed_label;
      QTimer* m_pressed_timer;

      void on_button_pressed(int row);
  };
}

#endif
