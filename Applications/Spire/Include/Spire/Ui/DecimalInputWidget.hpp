#ifndef SPIRE_DECIMAL_INPUT_WIDGET_HPP
#define SPIRE_DECIMAL_INPUT_WIDGET_HPP
#include <QDoubleSpinBox>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents a widget for displaying and modifying decimal numbers.
  class DecimalInputWidget : public QDoubleSpinBox {
    public:

      //! Signals a user interaction with the value.
      using ValueSignal = Signal<void (double value)>;

      //! Constructs a DecimalInputWidget.
      /*!
        \param value The initial value to display.
        \param parent The parent widget.
      */
      explicit DecimalInputWidget(double value, QWidget* parent = nullptr);

      QString textFromValue(double value) const override;

      //! Connects a slot to the value change signal.
      boost::signals2::connection connect_change_signal(
        const ValueSignal::slot_type& slot) const;

      //! Connects a slot to the value submit signal.
      boost::signals2::connection connect_submit_signal(
        const ValueSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      mutable ValueSignal m_change_signal;
      mutable ValueSignal m_submit_signal;
      int m_last_cursor_pos;
      bool m_has_first_click;

      void add_step(int step, Qt::KeyboardModifiers modifiers);
      void revert_cursor();
      void on_text_edited(const QString& text);
      void on_value_changed(double value);
  };
}

#endif
