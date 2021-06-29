#ifndef SPIRE_KEY_INPUT_BOX_HPP
#define SPIRE_KEY_INPUT_BOX_HPP
#include <QHBoxLayout>
#include <QKeySequence>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ValueModel over a QKeySequence. */
  using KeySequenceModel = ValueModel<QKeySequence>;

  /** A LocalValueModel over a QKeySequence. */
  using LocalKeySequenceModel = LocalValueModel<QKeySequence>;

  /** Represents an input for entering key sequences. */
  class KeyInputBox : public QWidget {
    public:

      /**
       * Signals that the current key sequence is being submitted.
       * @param submission The submitted key sequence.
       */
      using SubmitSignal = Signal<void (const QKeySequence& submission)>;

      /**
       * Constructs a KeyInputBox.
       * @param model The current key sequence's model.
       * @param parent The parent widget.
       */
      KeyInputBox(std::shared_ptr<KeySequenceModel> model,
        QWidget* parent = nullptr);

      /** Returns the last submitted key sequence. */
      const QKeySequence& get_submission() const;

      /** Returns the model. */
      const std::shared_ptr<KeySequenceModel>& get_model() const;

      QSize sizeHint() const override;

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      enum class Status {
        NONE,
        PROMPT
      };

      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<KeySequenceModel> m_model;
      boost::signals2::scoped_connection m_current_connection;
      QKeySequence m_submission;
      QKeySequence m_previous_current;
      LayeredWidget* m_layers;
      TextBox* m_text_box;
      QHBoxLayout* m_key_layout;
      QWidget* m_key_spacer;
      int m_text_box_right_margin;

      void set_status(Status status);
      void submit_current();
      void on_current_sequence(const QKeySequence& sequence);
      void on_text_box_style();
  };
}

#endif
