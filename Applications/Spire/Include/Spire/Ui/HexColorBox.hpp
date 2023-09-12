#ifndef SPIRE_HEX_COLOR_BOX_HPP
#define SPIRE_HEX_COLOR_BOX_HPP
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a color in hex rgb format #RRGGBB. */
  class HexColorBox : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const QString& submission)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The value that was rejected.
       */
      using RejectSignal = Signal<void (const QString& value)>;

      /**
       * Constructs a HexColorBox with a local model.
       * @param parent The parent widget.
       */
      explicit HexColorBox(QWidget* parent = nullptr);

      /**
       * Constructs a HexColorBox using a initial current value.
       * @param current The initial current value.
       * @param parent The parent widget.
       */
      explicit HexColorBox(QString current, QWidget* parent = nullptr);

      /**
       * Constructs a HexColorBox.
       * @param current The current value model.
       * @param parent The parent widget.
       */
      explicit HexColorBox(std::shared_ptr<TextModel> current,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<TextModel>& get_current() const;

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectedSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<TextModel> m_current;
      QString m_submission;
      TextBox* m_text_box;
      bool m_is_rejected;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_text_box_current_connection;
      boost::signals2::scoped_connection m_text_box_submit_connection;

      void on_current(const QString& current);
      void on_text_box_current(const QString& current);
      void on_text_box_submission(const QString& submission);
  };
}

#endif
