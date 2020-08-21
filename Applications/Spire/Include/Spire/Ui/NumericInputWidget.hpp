#ifndef SPIRE_NUMERIC_INPUT_WIDGET_HPP
#define SPIRE_NUMERIC_INPUT_WIDGET_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/optional.hpp>
#include <QAbstractSpinBox>
#include <QLocale>
#include <QStyle>
#include <QRegularExpression>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents a widget for entering numeric values.
  class NumericInputWidget : public QAbstractSpinBox {
    public:

      //! The maximum number of accepted decimal places.
      static const auto MAX_DECIMAL_PLACES = 15;

      //! Numeric type used by the widget.
      using Real = boost::multiprecision::cpp_dec_float<MAX_DECIMAL_PLACES>;

      //! Signal type for value changes.
      /*!
        \param value The updated value.
      */
      using ChangeSignal = Signal<void (Real value)>;

      //! Constructs a NumericInputWidget with a default step of 1.
      /*!
        \param parent The parent widget.
      */
      NumericInputWidget(Real value, QWidget* parent = nullptr);

      //! Sets the number of decimal places that can be input, up to
      //! MAX_DECIMAL_PLACES.
      /*!
        \param decimals The number of accepted decimal places.
      */
      void set_decimals(int decimals);

      //! Sets the minimum number of displayed decimals to display
      //! when a value is committed.
      /*!
        \param decimals The minimum number of decimals to display.
      */
      void set_minimum_decimals(int decimals);

      //! Sets the minimum accepted value.
      /*!
        \param minimum The minimum value.
      */
      void set_minimum(Real minimum);

      //! Sets the maximum accepted value.
      /*!
        \param maximum The maximum value.
      */
      void set_maximum(Real maximum);

      //! Sets the value to increment/decrement by when stepping up or down.
      /*!
        \param step The increment/decrement value.
      */
      void set_step(Real step);

      //! Returns the last submitted value.
      Real get_value() const;

      //! Sets the current displayed value.
      /*!
        \param value The current value.
      */
      void set_value(Real value);

      //! Increments the value by the given number of steps.
      /*!
        \param step The number of steps to increment/decrement the value by.
      */
      void stepBy(int step) override;

      //! Connects a slot to the value change signal.
      boost::signals2::connection connect_change_signal(
        const ChangeSignal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void timerEvent(QTimerEvent* event) override;
      QAbstractSpinBox::StepEnabled stepEnabled() const override;

    private:
      mutable ChangeSignal m_change_signal;
      QRegularExpression m_real_regex;
      QLocale m_locale;
      Real m_minimum;
      Real m_maximum;
      int m_decimals;
      int m_minimum_decimals;
      Real m_step;
      Real m_last_valid_value;
      QString m_last_valid_text;
      bool m_has_first_click;
      int m_up_arrow_timer_id;
      int m_down_arrow_timer_id;

      void add_step(int step);
      void add_step(int step, Qt::KeyboardModifiers modifiers);
      QString display_string(Real value);
      QStyle::SubControl get_current_control(const QPoint& mouse_pos);
      boost::optional<Real> get_value(const QString& text) const;
      bool is_valid(const QString& string);
      void set_stylesheet(bool is_up_disabled, bool is_down_disabled);
      void stop_timer(int& timer_id);
      void update_stylesheet();
      void on_editing_finished();
      void on_text_changed(const QString& text);
  };
}

#endif
