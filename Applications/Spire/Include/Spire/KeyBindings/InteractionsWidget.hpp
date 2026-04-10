#ifndef SPIRE_INTERACTIONS_WIDGET_HPP
#define SPIRE_INTERACTIONS_WIDGET_HPP
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Nexus/Definitions/ScopeMap.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_InteractionsWidget;

namespace Spire {
  class UserProfile;

  /** Displays and allows modification of the InteractionsProperties. */
  class InteractionsWidget : public QWidget {
    public:

      /**
       * Constructs an InteractionsWidget, call Initialize after construction.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      explicit InteractionsWidget(
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      ~InteractionsWidget() override;

      /**
       * Initializes the InteractionsWidget, must be called after construction.
       * @param userProfile The user's profile.
       */
      void Initialize(Beam::Ref<UserProfile> userProfile);

      /**
       * Initializes the InteractionsWidget, must be called after construction.
       * @param userProfile The user's profile.
       * @param ticker The Ticker to display the InteractionsProperties for.
       */
      void Initialize(
        Beam::Ref<UserProfile> userProfile, const Nexus::Ticker& ticker);

      /** Stores the current interactions to the UserProfile. */
      void Store();

    private:
      struct ScopeEntry {
        Nexus::Scope m_scope;
        bool m_isActive;
      };
      std::unique_ptr<Ui_InteractionsWidget> m_ui;
      UserProfile* m_userProfile;
      std::shared_ptr<InteractionsKeyBindingsModel> m_interactions;
      std::unordered_map<std::string, ScopeEntry> m_scopes;
      std::string m_scopeIndex;
      int m_quantityModifierIndex;
      int m_priceModifierIndex;

      void Add(ScopeEntry scope);
      void Style(const ScopeEntry& scope);
      void Update();
      void OnScopeIndexChanged(int index);
      void OnKeyboardModifierIndexChanged(int index);
      void OnActivateScopeClicked();
      void OnResetScopeClicked();
  };
}

#endif
