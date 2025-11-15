#ifndef SPIRE_PERCENT_BOX_HPP
#define SPIRE_PERCENT_BOX_HPP
#include "Spire/Ui/DecimalBox.hpp"

namespace Spire {

  /** Represents a widget for inputting percentage. */
  class PercentBox : public QWidget {
    public:

      /** Signals a submission. */
      using SubmitSignal = DecimalBox::SubmitSignal;

      /** Signals that the current value was rejected as a submission. */
      using RejectSignal = DecimalBox::RejectSignal;

      /**
       * Constructs a PercentBox with a LocalOptionalValueModel and default
       * modifiers.
       * @param parent The parent widget.
       */
      explicit PercentBox(QWidget* parent = nullptr);

      /**
       * Constructs a PercentBox with a LocalOptionalValueModel.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      explicit PercentBox(QHash<Qt::KeyboardModifier, Decimal>
        modifiers, QWidget* parent = nullptr);

      /**
       * Constructs a PercentBox with default modifiers.
       * @param current The current value model.
       * @param parent The parent widget.
       */
      explicit PercentBox(std::shared_ptr<OptionalDecimalModel> current,
        QWidget* parent = nullptr);

      /**
       * Constructs a PercentBox.
       * @param current The current value model.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      PercentBox(std::shared_ptr<OptionalDecimalModel> current,
        QHash<Qt::KeyboardModifier, Decimal> modifiers,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<OptionalDecimalModel>& get_current() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& value);

      /** Returns <code>true</code> iff this box is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the PercentBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the value submission signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

    private:
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      DecimalBox* m_decimal_box;
      std::shared_ptr<OptionalDecimalModel> m_model;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_reject_connection;

      void on_submit(const boost::optional<Decimal>& submission);
      void on_reject(const boost::optional<Decimal>& value);
  };
}

#endif
