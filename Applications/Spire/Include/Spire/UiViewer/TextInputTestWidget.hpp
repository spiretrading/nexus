#ifndef SPIRE_TEXT_INPUT_TEST_WIDGET_HPP
#define SPIRE_TEXT_INPUT_TEST_WIDGET_HPP
#include <functional>
#include <QGridLayout>
#include <QLabel>
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Represents a widget for testing a TextInputWidget.
  class TextInputTestWidget : public QWidget {
    public:

      //! Constructs a TextInputTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit TextInputTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      TextInputWidget* m_text_input;
      QLabel* m_text_label;
      TextInputWidget* m_set_text_input;
      CheckBox* m_disable_check_box;
      std::function<void()> m_reset_tab_order;

      void on_reset_button();
      void on_set_text_button();
  };
}

#endif
