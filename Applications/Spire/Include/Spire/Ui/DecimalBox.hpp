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
      using SubmitSignal = Signal<void (const Decimal& value)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The value that was rejected.
       */
      using RejectSignal = Signal<void (const Decimal& value)>;

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

      //! Sets the placeholder value.
      void set_placeholder(const QString& value);

      //! Returns <code>true</code> iff this box is read-only.
      bool is_read_only() const;

      //! Sets the read-only state.
      /*!
        \param is_read_only True iff the DecimalBox should be read-only.
      */
      void set_read_only(bool is_read_only);

      //! Returns whether a warning is displayed when a submission is rejected.
      bool is_warning_displayed() const;

      //! Sets whether a warning is displayed when a submission is rejected.
      void set_warning_displayed(bool is_displayed);

      //! Connects a slot to the value submission signal.
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      //! Connects a slot to the RejectSignal.
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

      bool test_selector(const Styles::Selector& element,
        const Styles::Selector& selector) const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<DecimalModel> m_model;
      Decimal m_submission;
      QHash<Qt::KeyboardModifier, Decimal> m_modifiers;
      TextBox* m_text_box;
      QRegExp m_validator;
      Button* m_up_button;
      Button* m_down_button;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_reject_connection;

      void decrement();
      void increment();
      Decimal get_increment() const;
      void step_by(const Decimal& value);
      void update_button_positions();
      void on_current(const Decimal& current);
      void on_submit(const QString& submission);
      void on_reject(const QString& value);
  };
}

#endif
