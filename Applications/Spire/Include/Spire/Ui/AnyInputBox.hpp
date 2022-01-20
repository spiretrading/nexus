#ifndef SPIRE_ANY_INPUT_BOX_HPP
#define SPIRE_ANY_INPUT_BOX_HPP
#include <QWidget>
#include "Spire/Spire/AnyValueModel.hpp"
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Encapsulates a generic input box. */
  class AnyInputBox : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const AnyRef& submission)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The rejected value.
       */
      using RejectSignal = Signal<void (const AnyRef& value)>;

      /**
       * Constructs an AnyInputBox.
       * @param input_box The input box to encapsulate.
       * @param parent The parent widget.
       */
      template<typename T>
      explicit AnyInputBox(T& input_box, QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<AnyValueModel>& get_current() const;

      /** Returns the last submission. */
      const AnyRef& get_submission() const;

      /** Returns <code>true</code> iff this input box is read-only. */
      bool is_read_only() const;

      /** Sets whether this input box is read-only. */
      void set_read_only(bool read_only);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectedSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;
  };

  template<typename T>
  AnyInputBox::AnyInputBox(T& input_box, QWidget* parent) {}
}

#endif
