#ifndef SPIRE_TEXT_BOX_HPP
#define SPIRE_TEXT_BOX_HPP
#include <QLineEdit>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a one-line text box.
  class TextBox : public QLineEdit {
    public:

      //! Signals that the current text changed.
      using CurrentSignal = Signal<void (const QString& text)>;

      //! Signals that the text is submitted.
      using SubmitSignal = Signal<void (const QString& text)>;

      //! Constructs a text box without a default text.
      /*!
        \param parent The parent widget.
      */
      explicit TextBox(QWidget* parent = nullptr);

      //! Constructs a text box with a default text.
      /*!
        \param text The text to display.
        \param parent The parent widget.
      */
      explicit TextBox(const QString& text, QWidget* parent = nullptr);

      //! Gets the text.
      QString get_text() const;

      //! Sets the text.
      void set_text(const QString& text);

      //! Connects a slot to the current signal.
      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const;

      //! Connects a slot to the submit signal.
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      QSize sizeHint() const override;

    private:
      mutable CurrentSignal m_current_signal;
      mutable SubmitSignal m_submit_signal;
      QString m_text;

      QString text() const = delete;
      void setText(const QString&) = delete;
      void on_editing_finished();
      void on_selection_changed();
      void on_text_edited(const QString& text);
      void elide_text();
  };
}

#endif
