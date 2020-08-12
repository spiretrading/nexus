#ifndef SPIRE_DECIMAL_INPUT_WIDGET_HPP
#define SPIRE_DECIMAL_INPUT_WIDGET_HPP
#include <QDoubleSpinBox>

namespace Spire {

  //! Represents a widget for displaying and modifying decimal numbers.
  class DecimalInputWidget : public QDoubleSpinBox {
    public:

      //! Constructs a DecimalInputWidget.
      /*!
        \param value The initial value to display.
        \param parent The parent widget.
      */
      explicit DecimalInputWidget(double value, QWidget* parent = nullptr);

      QString textFromValue(double value) const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      double m_last_valid_value;
      int m_last_cursor_pos;
      bool m_has_first_click;

      void add_step(int step, Qt::KeyboardModifiers modifiers);
      void revert_cursor();
      void set_stylesheet(bool is_up_disabled, bool is_down_disabled);
      void on_text_edited(const QString& text);
      void on_value_changed(double value);
  };
}

#endif
