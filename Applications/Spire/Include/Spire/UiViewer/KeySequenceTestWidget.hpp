#ifndef SPIRE_KEY_SEQUENCE_TEST_WIDGET_HPP
#define SPIRE_KEY_SEQUENCE_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/KeySequenceInputField.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Represents a widget for testing a KeySequenceInputField.
  class KeySequenceTestWidget : public QWidget {
    public:

      //! Constructs a KeySequenceTestWidget.
      /*!
        \param parent The parent widget.
      */
      KeySequenceTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      QLabel* m_status_label;
      CheckBox* m_disable_check_box;
      KeySequenceInputField* m_input;
      TextInputWidget* m_set_input;
      FlatButton* m_set_button;
      TextInputWidget* m_reset_input;
      FlatButton* m_reset_button;

      QKeySequence parse_key_sequence(const QString& text);
      void on_reset_button();
      void on_set_button();
  };
}

#endif
