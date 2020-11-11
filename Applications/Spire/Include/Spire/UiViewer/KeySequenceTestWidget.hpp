#ifndef SPIRE_KEY_SEQUENCE_TEST_WIDGET_HPP
#define SPIRE_KEY_SEQUENCE_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/KeySequenceInputField.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  class KeySequenceTestWidget : public QWidget {
    public:

      KeySequenceTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      QLabel* m_status_label;
      KeySequenceInputField* m_input;
      TextInputWidget* m_reset_input;
      TextInputWidget* m_set_input;

      QKeySequence parse_key_sequence(const QString& text);
      void on_reset_button();
      void on_set_button();
  };
}

#endif
