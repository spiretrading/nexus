#ifndef SPIRE_SCOPE_INPUT_DIALOG_HPP
#define SPIRE_SCOPE_INPUT_DIALOG_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include "Nexus/Definitions/Scope.hpp"
#include "Spire/UI/UserProfile.hpp"

class Ui_ValueListInputDialog;

namespace Spire {

  /** Displays an input editor dialog for a Scope. */
  class ScopeInputDialog : public QDialog {
    public:

      /**
       * Constructs a ScopeInputDialog.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      explicit ScopeInputDialog(
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      /**
       * Constructs a ScopeInputDialog.
       * @param scope The Scope to display.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      ScopeInputDialog(Nexus::Scope scope, Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr);

      ~ScopeInputDialog() override;

      /** Returns the Scope. */
      const Nexus::Scope& GetScope() const;

    private:
      std::unique_ptr<Ui_ValueListInputDialog> m_ui;
      UserProfile* m_userProfile;
      Nexus::Scope m_scope;

      void ActivateRow(int row, QKeyEvent* event);
      void Append(const QVariant& item);
      void OnAccept();
      void OnAddItemAction();
      void OnLoadFileAction();
      void OnDeleteItemAction();
      void OnCellActivated(int row, int column);
  };
}

#endif
