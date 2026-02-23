#ifndef SPIRE_INTERACTIONSWIDGET_HPP
#define SPIRE_INTERACTIONSWIDGET_HPP
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Nexus/Definitions/ScopeTable.hpp"
#include "Spire/KeyBindings/InteractionsProperties.hpp"
#include "Spire/UI/UI.hpp"

class Ui_InteractionsWidget;

namespace Spire {

  /*! \class InteractionsWidget
      \brief Displays and allows modification of the InteractionsProperties.
   */
  class InteractionsWidget : public QWidget {
    public:

      //! Constructs an InteractionsWidget, call init after construction.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      InteractionsWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~InteractionsWidget();

      //! Initializes the InteractionsWidget, must be called after construction.
      /*!
        \param userProfile The user's profile.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile);

      //! Initializes the InteractionsWidget, must be called after construction.
      /*!
        \param userProfile The user's profile.
        \param ticker The Ticker to display the InteractionsProperties for.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile,
        const Nexus::Ticker& ticker);

      //! Returns the InteractionsProperties represented by this widget.
      const Nexus::ScopeTable<InteractionsProperties>& GetProperties();

    private:
      struct ScopeEntry {
        Nexus::Scope m_scope;
        bool m_isActive;
      };
      std::unique_ptr<Ui_InteractionsWidget> m_ui;
      UserProfile* m_userProfile;
      Nexus::ScopeTable<InteractionsProperties> m_properties;
      std::unordered_map<std::string, ScopeEntry> m_scopes;
      std::string m_scopeIndex;
      int m_quantityModifierIndex;
      int m_priceModifierIndex;

      void AddScope(ScopeEntry scope);
      void StyleScope(const ScopeEntry& scope);
      void Update();
      void Store();
      void OnScopeIndexChanged(int index);
      void OnKeyboardModifierIndexChanged(int index);
      void OnActivateScopeClicked();
      void OnResetScopeClicked();
  };
}

#endif
