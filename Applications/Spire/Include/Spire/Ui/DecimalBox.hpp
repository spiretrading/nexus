#ifndef SPIRE_DECIMAL_BOX_HPP
#define SPIRE_DECIMAL_BOX_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <QHash>
#include <QRegularExpression>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/StyledWidget.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/ScalarValueModel.hpp"

namespace Spire {

  //! Represents a widget for inputting decimal values.
  class DecimalBox : public Styles::StyledWidget {
    public:

      //! The maximum precision of the Decimal type.
      static constexpr auto PRECISION = 15;

      //! Represents the floating point type used by the DecimalBox.
      using Decimal = boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<PRECISION>>;

      /** Type of model used by the DecimalBox. */
      using DecimalModel = ScalarValueModel<Decimal>;

      //! Signals that submission value has changed.
      /*!
        \param value The submission value.
      */
      using SubmitSignal = Signal<void (Decimal value)>;

      //! Constructs a DecimalBox with a LocalValueModel.
      /*!
        \param modifiers The initial keyboard modifier increments.
        \param parent The parent widget.
      */
      explicit DecimalBox(QHash<Qt::KeyboardModifier, Decimal> modifiers,
        QWidget* parent = nullptr);

      //! Constructs a DecimalBox with 6 decimal places and no trailing zeros.
      /*!
        \param model The model used for the current value.
        \param modifiers The initial keyboard modifier increments.
        \param parent The parent widget.
      */
      DecimalBox(std::shared_ptr<DecimalModel> model,
        QHash<Qt::KeyboardModifier, Decimal> modifiers,
        QWidget* parent = nullptr);

      //! Returns the current value model.
      const std::shared_ptr<DecimalModel>& get_model() const;

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

      //! Connects a slot to the value submission signal.
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      bool test_selector(const Styles::Selector& element,
        const Styles::Selector& selector) const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<DecimalModel> m_model;
      Decimal m_submission;
      QHash<Qt::KeyboardModifier, Decimal> m_modifiers;
      bool m_has_trailing_zeros;
      TextBox* m_text_box;
      QRegExp m_validator;
      QRegExp m_trailing_zero_regex;
      Button* m_up_button;
      Button* m_down_button;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_submit_connection;

      void decrement();
      void increment();
      Decimal get_increment() const;
      void step_by(Decimal value);
      void update_button_positions();
      void update_trailing_zeros();
      void on_current(const Decimal& current);
      void on_submit(const QString& submission);
  };
}

#endif
