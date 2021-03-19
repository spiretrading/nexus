#ifndef SPIRE_DECIMAL_BOX_HPP
#define SPIRE_DECIMAL_BOX_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <QHash>
#include <QRegularExpression>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/StyledWidget.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a widget for inputting decimal values.
  class DecimalBox : public Styles::StyledWidget {
    public:

      //! The maximum precision of the Decimal type.
      static constexpr auto PRECISION = 15;

      //! Represents the floating point type used by the DecimalBox.
      using Decimal = boost::multiprecision::cpp_dec_float<PRECISION>;

      //! Signals that the current value has changed.
      /*!
        \param value The current value.
      */
      using CurrentSignal = Signal<void (Decimal value)>;

      //! Signals that submission value has changed.
      /*!
        \param value The submission value.
      */
      using SubmitSignal = Signal<void (Decimal value)>;

      //! Constructs a DecimalBox with 6 decimal places and no trailing zeros.
      /*!
        \param current The current value to display.
        \param minimum The minimum acceptable value.
        \param maximum The maximum acceptable value.
        \param modifiers The initial keyboard modifier increments.
        \param parent The parent widget.
      */
      DecimalBox(Decimal current, Decimal minimum, Decimal maximum,
        QHash<Qt::KeyboardModifier, Decimal> modifiers,
        QWidget* parent = nullptr);

      //! Return the current value.
      Decimal get_current() const;

      //! Sets the current value.
      /*!
        \param current The current value.
      */
      void set_current(Decimal current);

      //! Returns the minimum value.
      Decimal get_minimum() const;

      //! Sets the minimum value.
      /*!
        \param minimum The minimum value.
      */
      void set_minimum(Decimal minimum);

      //! Returns the maximum value.
      Decimal get_maximum() const;

      //! Sets the maximum value.
      /*!
        \param maximum The maximum value.
      */
      void set_maximum(Decimal maximum);

      //! Returns the increment for the given keyboard modifier.
      /*!
        \param modifier The keyboard modifier.
      */
      Decimal get_increment(Qt::KeyboardModifier modifier) const;

      //! Sets the increment for the given keyboard modifier.
      /*!
        \param modifier The keyboard modifier.
        \param increment The increment value.
      */
      void set_increment(Qt::KeyboardModifier modifier, Decimal increment);

      //! Returns the maximum accepted decimal places.
      int get_decimal_places() const;

      //! Sets the maximum accepted decimal places.
      /*!
        \param decimal_places The maximum decimal places.
      */
      void set_decimal_places(int decimal_places);

      //! Sets the number of leading zeros that are prepended to the current
      //! value on submission.
      /*!
        \param leading_zeros The number of leading zeros.
      */
      void set_leading_zeros(int leading_zeros);

      //! Sets the number of trailing zeros that are appended to the current
      //! value on submission.
      /*!
        \param trailing_zeros The number of trailing zeros.
      */
      void set_trailing_zeros(int trailing_zeros);

      //! Sets the read-only state.
      /*!
        \param is_read_only True iff the DecimalBox should be read-only.
      */
      void set_read_only(bool is_read_only);

      //! Sets if the warning style should be applied.
      /*!
        \param has_warning True iff the warning style should be applied.
      */
      void set_suppress_warnings(bool has_warning);

      //! Connets a slot to the current changed signal.
      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const;

      //! Connects a slot to the value submission signal.
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      bool test_selector(const Styles::Selector& element,
        const Styles::Selector& selector) const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

    private:
      mutable CurrentSignal m_current_signal;
      mutable SubmitSignal m_submit_signal;
      QString m_current;
      Decimal m_submission;
      Decimal m_minimum;
      Decimal m_maximum;
      QHash<Qt::KeyboardModifier, Decimal> m_modifiers;
      int m_decimal_places;
      int m_leading_zeros;
      int m_trailing_zeros;
      TextBox* m_text_box;
      QRegExp m_validator;
      QRegExp m_trailing_zero_regex;
      Button* m_up_button;
      Button* m_down_button;
      bool m_has_suppressed_warning;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_submit_connection;

      void decrement();
      void increment();
      Decimal get_increment() const;
      void step_by(Decimal value);
      void update_button_positions();
      void update_padded_zeros();
      void on_current(const QString& current);
      void on_submit(const QString& submission);
  };
}

#endif
