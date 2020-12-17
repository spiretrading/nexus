#ifndef SPIRE_FLAT_BUTTON_TEST_WIDGET_HPP
#define SPIRE_FLAT_BUTTON_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include <QWidget>
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Displays a widget for testing FlatButtons.
  class FlatButtonTestWidget : public QWidget {
    public:

      //! Constructs a FlatButtonTestWidget.
      /*
        \param parent The parent widget.
      */
      explicit FlatButtonTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      FlatButton* m_button;
      QLabel* m_status_label;
      TextInputWidget* m_label_input;
      FlatButton* m_create_button;
      CheckBox* m_disable_check_box;
      QTimer* m_pressed_timer;

      void set_button(FlatButton* button);
      void on_create_button();
  };
}

#endif
