#ifndef SPIRE_KEY_SEQUENCE_TEST_WIDGET_HPP
#define SPIRE_KEY_SEQUENCE_TEST_WIDGET_HPP
#include <functional>
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
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
      KeySequenceInputField* m_input;
      TextInputWidget* m_reset_input;
      TextInputWidget* m_set_input;
      std::function<void()> m_reset_tab_order;

      QKeySequence parse_key_sequence(const QString& text);
      void on_reset_button();
      void on_set_button();
  };
}

#endif
