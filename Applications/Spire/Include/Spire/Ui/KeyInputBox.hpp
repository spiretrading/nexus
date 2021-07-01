#ifndef SPIRE_KEY_INPUT_BOX_HPP
#define SPIRE_KEY_INPUT_BOX_HPP
#include <QKeySequence>
#include <QWidget>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ValueModel over a QKeySequence. */
  using KeySequenceValueModel = ValueModel<QKeySequence>;

  /** A LocalValueModel over a QKeySequence. */
  using LocalKeySequenceValueModel = LocalValueModel<QKeySequence>;

  /** Displays an input box representing a QKeySequence. */
  class KeyInputBox : public QWidget {
    public:

      /**
       * Signals a user submission.
       * @param submission The value being submitted.
       */
      using SubmitSignal = Signal<void (const QKeySequence& submission)>;

      /**
       * Constructs a KeyInputBox.
       * @param current The model representing the current value.
       * @param parent The parent widget.
       */
      explicit KeyInputBox(std::shared_ptr<KeySequenceValueModel> current,
        QWidget* parent = nullptr);

      /**
       * Constructs a KeyInputBox using a LocalKeySequenceValueModel to
       * represent the current value.
       * @param parent The parent widget.
       */
      explicit KeyInputBox(QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<KeySequenceValueModel>& get_current() const;

      /**
       * Connects a slot to the SubmitSignal.
       * @param slot The slot to connect to the SubmitSignal.
       * @return The connection to the SubmitSignal.
       */
      boost::signals2::connection
        connect_submit_signal(const SubmitSignal::slot_type& slot) const;
  };
}

#endif
