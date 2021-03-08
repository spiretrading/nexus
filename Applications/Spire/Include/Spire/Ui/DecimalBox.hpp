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

      //! Returns true iff the DecimalBox appends trailing zeros to the input,
      //! up to the number of maximum decimal places.
      bool has_trailing_zeros() const;

      //! Sets if the DecimalBox should append trailing zeros.
      /*!
        \param has_trailing_zeros True iff the DecimalBox appends trailing
                                  zeros.
      */
      void set_trailing_zeros(bool has_trailing_zeros);

      //! Sets if the increment and decrement buttons are visible.
      /*!
        \param are_visible True iff the buttons are visible.
      */
      void set_buttons_visible(bool are_visible);

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

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable CurrentSignal m_current_signal;
      mutable SubmitSignal m_submit_signal;
      QString m_current;
      Decimal m_submission;
      Decimal m_minimum;
      Decimal m_maximum;
      QHash<Qt::KeyboardModifier, Decimal> m_modifiers;
      int m_decimal_places;
      bool m_has_trailing_zeros;
      TextBox* m_text_box;
      QRegExp m_validator;
      QRegExp m_trailing_zero_regex;
      IconButton* m_up_button;
      IconButton* m_down_button;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_submit_connection;

      void decrement();
      void increment();
      Decimal get_increment() const;
      void step_by(Decimal value);
      void update_button_positions();
      void update_trailing_zeros();
      void on_current(const QString& current);
      void on_submit(const QString& submission);
  };
}

#endif
