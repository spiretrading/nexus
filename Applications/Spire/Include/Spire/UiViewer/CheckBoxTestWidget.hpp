#ifndef SPIRE_CHECK_BOX_TEST_WIDGET_HPP
#define SPIRE_CHECK_BOX_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Represents a widget for testing a CheckBox.
  class CheckBoxTestWidget : public QWidget {
    public:

      //! Constructs a CheckBoxTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit CheckBoxTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      CheckBox* m_check_box;
      QLabel* m_status_label;
      TextInputWidget* m_label_input;
      FlatButton* m_reset_button;

      void on_reset_button();
  };
}

#endif
