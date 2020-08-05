#ifndef SPIRE_DECIMAL_INPUT_WIDGET_HPP
#define SPIRE_DECIMAL_INPUT_WIDGET_HPP
#include <QDoubleSpinBox>

namespace Spire {

  class DecimalInputWidget : public QDoubleSpinBox {
    public:

      explicit DecimalInputWidget(double value, QWidget* parent = nullptr);

      QString textFromValue(double value) const override;

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      double m_value;
      int m_last_cursor_pos;

      void on_cursor_position_changed(int old_pos, int new_pos);
      void on_editing_finished();
      void on_text_edited(const QString& text);
      void on_value_changed(double value);
  };
}

#endif
