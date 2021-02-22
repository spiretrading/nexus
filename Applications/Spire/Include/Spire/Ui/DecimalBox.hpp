#ifndef SPIRE_DECIMAL_BOX_HPP
#define SPIRE_DECIMAL_BOX_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <QHash>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a widget for inputting decimal values.
  class DecimalBox : public QWidget {
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
        \param initial The initial value to display.
        \param minimum The minimum acceptable value.
        \param maximum The maximum acceptable value.
        \param modifiers The initial keyboard modifier increments.
        \param parent The parent widget.
      */
      DecimalBox(Decimal initial, Decimal minimum, Decimal maximum,
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

      //! Returns true iff the DecimalBox appends trailing zeros to the input,
      //! up to the number of maximum decimal places.
      bool has_trailing_zeros() const;

      //! Sets if the DecimalBox should append trailing zeros.
      /*!
        \param has_trailing_zeros True iff the DecimalBox appends trailing
                                  zeros.
      */
      void set_trailing_zeros(bool has_trailing_zeros);

      //! Sets the read-only state.
      /*!
        \param is_read_only True iff the DecimalBox should be read-only.
      */
      void set_read_only(bool is_read_only);

      //! Connets a slot to the current changed signal.
      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const;

      //! Connects a slot to the value submission signal.
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      mutable CurrentSignal m_current_signal;
      mutable SubmitSignal m_submit_signal;
      Decimal m_minimum;
      Decimal m_maximum;
      QHash<Qt::KeyboardModifier, Decimal> m_modifiers;
      int m_decimal_places;
      bool m_has_trailing_zeros;
      TextBox* m_text_box;
  };
}

#endif
