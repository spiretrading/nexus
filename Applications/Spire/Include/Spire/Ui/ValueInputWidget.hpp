#ifndef SPIRE_VALUE_INPUT_WIDGET_HPP
#define SPIRE_VALUE_INPUT_WIDGET_HPP
#include <QSpinBox>

namespace Spire {

  //! Represents a widget for inputting whole numbers.
  class ValueInputWidget : public QSpinBox {
    public:

      //! Constructs a ValueInputWidget.
      /*
        \param min_value The lowest value that can be entered.
        \param max_value The highest value that can be entered.
        \param parent The parent widget.
      */
      ValueInputWidget(int min_value, int max_value,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      int m_min_value;
      int m_max_value;
  };
}

#endif
