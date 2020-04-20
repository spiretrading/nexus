#ifndef SPIRE_SECURITY_INPUT_WIDGET_HPP
#define SPIRE_SECURITY_INPUT_WIDGET_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QLineEdit>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/Spire/QtPromise.hpp"

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
        \param has_border True if the input box should have a styled border.
        \param parent The parent to this widget.
      */
      explicit SecurityInputBox(Beam::Ref<SecurityInputModel> model,
        bool has_border, QWidget* parent = nullptr);

      //! Constructs a security input box with an initial text value.
      /*!
        \param model The model to query for securities.
        \param initial_text The initial text to display in the line edit.
        \param has_border True if the input box should have a styled border.
        \param parent The parent to this widget.
      */
      explicit SecurityInputBox(Beam::Ref<SecurityInputModel> model,
        const QString& initial_text, bool has_border,
        QWidget* parent = nullptr);

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_commit_signal(
        const CommitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void hideEvent(QHideEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable CommitSignal m_commit_signal;
      SecurityInputModel* m_model;
      bool m_has_border;
      QLineEdit* m_security_line_edit;
      SecurityInfoListView* m_securities;
      QtPromise<std::vector<Nexus::SecurityInfo>> m_completions;

      void on_text_edited();
      void move_line_edit();
      void enter_pressed();
      void on_activated(const Nexus::Security& security);
      void on_commit(const Nexus::Security& security);
  };
}

#endif
