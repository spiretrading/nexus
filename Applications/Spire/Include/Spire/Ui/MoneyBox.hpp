#ifndef SPIRE_MONEY_BOX_HPP
#define SPIRE_MONEY_BOX_HPP
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ScalarValueModel over Money values. */
  using MoneyModel = ScalarValueModel<Nexus::Money>;

  /** A ScalarValueModel over optional Money values. */
  using OptionalMoneyModel = ScalarValueModel<boost::optional<Nexus::Money>>;

  /** A LocalScalarValueModel over Money values. */
  using LocalMoneyModel = LocalScalarValueModel<Nexus::Money>;

  /** A LocalScalarValueModel over optional Money values. */
  using LocalOptionalMoneyModel =
    LocalScalarValueModel<boost::optional<Nexus::Money>>;

  /** Represents a widget for inputting Money values. */
  class MoneyBox : public QWidget {
    public:

      /**
       * Signals a submission.
       * @param value The submitted value.
       */
      using SubmitSignal = Signal<void (boost::optional<Nexus::Money> value)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The value that was rejected.
       */
      using RejectSignal = Signal<void (boost::optional<Nexus::Money> value)>;

      /**
       * Constructs a MoneyBox with a LocalValueModel.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      explicit MoneyBox(QHash<Qt::KeyboardModifier, Nexus::Money> modifiers,
        QWidget* parent = nullptr);

      /**
       * Constructs a MoneyBox.
       * @param model The model used for the current value.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      MoneyBox(std::shared_ptr<OptionalMoneyModel> model,
        QHash<Qt::KeyboardModifier, Nexus::Money> modifiers,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<OptionalMoneyModel>& get_model() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& value);

      /** Returns <code>true</code> iff this box is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the MoneyBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the value submission signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

    private:
      struct MoneyToDecimalModel;
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<OptionalMoneyModel> m_model;
      std::shared_ptr<MoneyToDecimalModel> m_adaptor_model;
      boost::optional<Nexus::Money> m_submission;
      DecimalBox* m_decimal_box;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_reject_connection;

      void on_submit(const boost::optional<Decimal>& submission);
      void on_reject(const boost::optional<Decimal>& value);
  };
}

#endif
