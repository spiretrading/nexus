#ifndef SPIRE_SECURITY_INPUT_LINE_EDIT_HPP
#define SPIRE_SECURITY_INPUT_LINE_EDIT_HPP
#include <QLineEdit>
#include "Beam/Pointers/Ref.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/SecurityInput/SecurityInfoListView.hpp"
#include "Spire/Spire/QtPromise.hpp"

namespace Spire {

  //! Displays a line edit with a drop down list of securities.
  class SecurityInputLineEdit : public QLineEdit {
    public:

      //! Signals that editing has completed.
      /*!
        \param s The security that was input.
      */
      using CommitSignal = Signal<void (const Nexus::Security& s)>;

      //! Constructs a SecurityInputLineEdit.
      /*
        \param initial_text The initial text to display.
        \param model The security input model.
        \param parent The parent widget.
      */
      SecurityInputLineEdit(const QString& initial_text,
        Beam::Ref<SecurityInputModel> model, QWidget* parent = nullptr);

      //! Returns the selected security, or an empty Security if there was no
      //! selection.
      const Nexus::Security& get_security() const;

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_commit_signal(
        const CommitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void hideEvent(QHideEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;
  
    private:
      mutable CommitSignal m_commit_signal;
      SecurityInputModel* m_model;
      SecurityInfoListView* m_securities;
      QtPromise<std::vector<Nexus::SecurityInfo>> m_completions;
      Nexus::Security m_security;

      void move_securities_list();
      void on_activated(const Nexus::Security& security);
      void on_commit(const Nexus::Security& security);
      void on_text_edited();
  };
}

#endif
