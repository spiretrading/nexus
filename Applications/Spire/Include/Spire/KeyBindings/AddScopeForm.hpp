#ifndef SPIRE_ADD_SCOPE_FORM_HPP
#define SPIRE_ADD_SCOPE_FORM_HPP
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/ScopeDropDownBox.hpp"

namespace Spire {
  class OverlayPanel;

  /** Displays a panel to add a scope. */
  class AddScopeForm : public QWidget {
    public:

      /**
       * Signals the submission of the scope.
       * @param submission The scope to be submitted.
       */
      using SubmitSignal = Signal<void (const Nexus::Scope& submission)>;

      /**
       * Constructs an AddScopeForm.
       * @param scopes A list of scopes for the user to select.
       * @param parent The parent widget.
       */
      AddScopeForm(std::shared_ptr<ScopeListModel> scopes, QWidget& parent);

      /** Returns a list of scopes for the user to select. */
      const std::shared_ptr<ScopeListModel>& get_scopes() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<ListModel<Nexus::Scope>> m_scopes;
      ScopeDropDownBox* m_scope_drop_down_box;
      OverlayPanel* m_panel;

      void on_cancel();
      void on_add();
  };
}

#endif
