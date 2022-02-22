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
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      enum class Status : std::uint8_t {
        UNINITIALIZED,
        NONE,
        PROMPT
      };
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<KeySequenceValueModel> m_current;
      QKeySequence m_submission;
      Status m_status;
      QWidget* m_body;
      Box* m_input_box;
      bool m_is_modified;
      boost::signals2::scoped_connection m_current_connection;

      void layout_key_sequence();
      void transition_status();
      void transition_submission();
      void set_status(Status status);
      void on_current(const QKeySequence& current);
  };
}

#endif
