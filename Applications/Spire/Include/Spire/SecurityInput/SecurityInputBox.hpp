#ifndef SPIRE_SECURITY_INPUT_WIDGET_HPP
#define SPIRE_SECURITY_INPUT_WIDGET_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/SecurityInput/SecurityInputLineEdit.hpp"

namespace Spire {

  //! Displays a input/search box for securities.
  class SecurityInputBox : public QWidget {
    public:

      //! Signals that editing has completed.
      /*!
        \param s The security that was input.
      */
      using CommitSignal = Signal<void (const Nexus::Security& s)>;

      //! Constructs a blank security input box.
      /*!
        \param model The model to query for securities.
        \param parent The parent to this widget.
      */
      explicit SecurityInputBox(Beam::Ref<SecurityInputModel> model,
        QWidget* parent = nullptr);

      //! Constructs a security input box with an initial text value.
      /*!
        \param model The model to query for securities.
        \param initial_text The initial text to display in the line edit.
        \param parent The parent to this widget.
      */
      explicit SecurityInputBox(Beam::Ref<SecurityInputModel> model,
        const QString& initial_text, QWidget* parent = nullptr);

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_commit_signal(
        const CommitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable CommitSignal m_commit_signal;
      SecurityInputLineEdit* m_security_line_edit;

      void on_commit(const Nexus::Security& security);
  };
}

#endif
