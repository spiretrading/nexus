#ifndef SPIRE_INFO_PANEL_HPP
#define SPIRE_INFO_PANEL_HPP
#include <QWidget>

#ifdef ERROR
  #undef ERROR
#endif

namespace Spire {
  class OverlayPanel;
  class TextAreaBox;

  /** Displays visible status messages to the user. */
  class InfoPanel : public QWidget {
    public:

      /** Specifies the severity of the message. */
      enum class Severity {

        /** The message is informational. */
        INFO,

        /** A background process has completed successfully. */
        SUCCESS,

        /** A condition that might cause a problem in the future. */
        WARNING,

        /** An error or problem has occurred. */
        ERROR
      };

      /**
       * Constructs an InfoPanel using Severity::INFO and initial message value.
       * @param message The message displayed in the info panel.
       * @param parent The parent widget.
       */
      InfoPanel(QString message, QWidget& parent);

      /**
       * Constructs an InfoPanel.
       * @param severity The severity of the message.
       * @param message The message displayed in the info panel.
       * @param parent The parent widget.
       */
      InfoPanel(Severity severity, QString message, QWidget& parent);

      /** Returns the severity. */
      Severity get_severity() const;

      /** Returns the message. */
      const QString& get_message() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      Severity m_severity;
      QString m_message;
      TextAreaBox* m_label;
      OverlayPanel* m_panel;

      void on_message_style();
  };
}

#endif
