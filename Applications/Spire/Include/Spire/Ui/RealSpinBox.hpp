#ifndef SPIRE_REAL_SPIN_BOX_HPP
#define SPIRE_REAL_SPIN_BOX_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/optional.hpp>
#include <QAbstractSpinBox>
#include <QLocale>
#include <QRegularExpression>
#include <QStyle>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents a widget for entering numeric values.
  class RealSpinBox : public QAbstractSpinBox {
    public:

      //! The maximum number of decimal places that can be input.
      static const auto MAXIMUM_DECIMAL_PLACES = 6;

      //! The precision of the spin box's floating point type.
      static constexpr auto PRECISION = 15;

      //! Numeric type used by the widget.
      using Real = boost::multiprecision::cpp_dec_float<PRECISION>;

      //! Signal type for value changes.
      /*!
        \param value The updated value.
      */
      using ChangeSignal = Signal<void (Real value)>;

      //! Constructs a RealSpinBox with a default step of 1.
      /*!
        \param value The initial value to display.
        \param parent The parent widget.
      */
      explicit RealSpinBox(std::shared_ptr<RealSpinBoxModel> model,
        QWidget* parent = nullptr);

      //! Sets the number of decimal places that can be input, up to
      //! MAXIMUM_DECIMAL_PLACES.
      /*!
        \param decimals The number of accepted decimal places.
      */
      void set_decimal_places(int decimals);

      //! Sets the minimum number of displayed decimals to display
      //! when a value is committed.
      /*!
        \param decimals The minimum number of decimals to display.
      */
      void set_minimum_decimal_places(int decimals);

      //! Returns the last submitted value.
      Real get_value() const;

      //! Sets the current displayed value iff the given value is a valid
      //! number.
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
      StepEnabled stepEnabled() const override;

    private:
      mutable ChangeSignal m_change_signal;
      std::shared_ptr<RealSpinBoxModel> m_model;
      QRegularExpression m_real_regex;
      QLocale m_locale;
      int m_decimals;
      int m_minimum_decimals;
      Real m_last_valid_value;
      QString m_last_valid_text;
      bool m_has_first_click;
      int m_up_arrow_timer_id;
      int m_down_arrow_timer_id;

      void add_step(int step);
      void add_step(int step, Qt::KeyboardModifiers modifiers);
      void assign_value(Real& variable, Real value);
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
