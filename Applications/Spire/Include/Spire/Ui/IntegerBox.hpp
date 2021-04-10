#ifndef SPIRE_INTEGER_BOX_HPP
#define SPIRE_INTEGER_BOX_HPP
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/ScalarValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a widget for inputting integer values.
  class IntegerBox : public QWidget {
    public:

      //! Signals that submission value has changed.
      /*!
        \param value The submission value.
      */
      using SubmitSignal = Signal<void (int value)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The value that was rejected.
       */
      using RejectSignal = Signal<void (int value)>;

      //! Constructs an IntegerBox with a LocalValueModel.
      /*!
        \param modifiers The initial keyboard modifier increments.
        \param parent The parent widget.
      */
      explicit IntegerBox(QHash<Qt::KeyboardModifier, int> modifiers,
        QWidget* parent = nullptr);

      //! Constructs an IntegerBox.
      /*!
        \param model The model used for the current value.
        \param modifiers The initial keyboard modifier increments.
        \param parent The parent widget.
      */
      IntegerBox(std::shared_ptr<IntegerModel> model,
        QHash<Qt::KeyboardModifier, int> modifiers, QWidget* parent = nullptr);

      //! Returns the current value model.
      const std::shared_ptr<IntegerModel>& get_model() const;

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

    private:
      struct IntegerToDecimalModel;
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<IntegerModel> m_model;
      std::shared_ptr<IntegerToDecimalModel> m_adaptor_model;
      int m_submission;
      DecimalBox* m_decimal_box;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_reject_connection;

      void on_submit(const DecimalBox::Decimal& submission);
      void on_reject(const DecimalBox::Decimal& value);
  };
}

#endif
