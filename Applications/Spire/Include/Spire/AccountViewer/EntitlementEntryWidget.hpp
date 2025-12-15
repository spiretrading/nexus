#ifndef SPIRE_ENTITLEMENTENTRYWIDGET_HPP
#define SPIRE_ENTITLEMENTENTRYWIDGET_HPP
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <QWidget>
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Spire/AccountViewer/AccountViewer.hpp"

class Ui_EntitlementEntryWidget;

namespace Spire {
  class UserProfile;

  /*! \class EntitlementEntryWidget
      \brief Displays information about a single market data entitlement.
   */
  class EntitlementEntryWidget : public QWidget {
    public:

      //! Constructs an EntitlementEntryWidget.
      /*!
        \param userProfile The user's profile.
        \param isReadOnly Whether the entitlement is strictly for display
               purposes.
        \param entitlement The entitlement to display.
        \param model The model the entitlement belongs to.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      EntitlementEntryWidget(Beam::Ref<UserProfile> userProfile,
        bool isReadOnly,
        const Nexus::EntitlementDatabase::Entry& entitlement,
        std::shared_ptr<AccountEntitlementModel> model,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~EntitlementEntryWidget();

    private:
      std::unique_ptr<Ui_EntitlementEntryWidget> m_ui;
      UserProfile* m_userProfile;
      bool m_isReadOnly;
      bool m_isChecked;
      Nexus::EntitlementDatabase::Entry m_entitlement;
      std::shared_ptr<AccountEntitlementModel> m_model;
      Beam::ConnectionGroup m_connections;

      void OnEntitlementGranted(const Beam::DirectoryEntry& entry);
      void OnEntitlementRevoked(const Beam::DirectoryEntry& entry);
      void OnEntitlementChecked(int checkState);
      void OnEntitlementClicked();
      void OnTableExpanded();
      void OnTableCollapsed();
  };
}

#endif
