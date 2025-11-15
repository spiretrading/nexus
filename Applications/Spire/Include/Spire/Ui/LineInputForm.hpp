#ifndef SPIRE_LINE_INPUT_FORM_HPP
#define SPIRE_LINE_INPUT_FORM_HPP
#include <QWidget>
#include "Spire/Ui/TextBox.hpp"

namespace Spire {
  class Button;
  class OverlayPanel;

  /** Displays a form to input a line of text. */
  class LineInputForm : public QWidget {
    public:

      /**
       * Signals a submission.
       * @param submission The line of text that was submitted.
       */
      using SubmitSignal = Signal<void (const QString& name)>;

      /**
       * Constructs a LineInputForm.
       * @param heading The heading to display in the title bar.
       * @param parent The parent widget.
       */
      LineInputForm(QString heading, QWidget& parent);

      /**
       * Constructs a LineInputForm.
       * @param heading The heading to display in the title bar.
       * @param current The current line input to display.
       * @param parent The parent widget.
       */
      LineInputForm(
        QString heading, std::shared_ptr<TextModel> current, QWidget& parent);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection
        connect_submit_signal(const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<TextModel> m_current;
      TextBox* m_input;
      Button* m_create_button;
      OverlayPanel* m_panel;
      boost::signals2::scoped_connection m_connection;

      void on_cancel();
      void on_create();
      void on_current(const QString& value);
  };
}

#endif
