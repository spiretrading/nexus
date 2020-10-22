#ifndef SPIRE_ICON_BUTTON_TEST_WIDGET_HPP
#define SPIRE_ICON_BUTTON_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QTimer>

namespace Spire {

  //! Represents a widget for testing an IconButton.
  class IconButtonTestWidget : public QWidget {
    public:

      //! Constructs an IconButtonTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit IconButtonTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      QLabel* m_pressed_label;
      QTimer* m_pressed_timer;

      void on_button_pressed(int row);
  };
}

#endif
