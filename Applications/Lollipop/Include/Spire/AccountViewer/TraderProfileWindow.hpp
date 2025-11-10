#ifndef SPIRE_TRADERPROFILEWINDOW_HPP
#define SPIRE_TRADERPROFILEWINDOW_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <QFrame>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/UI/UI.hpp"

class Ui_TraderProfileWindow;

namespace Spire {

  /*! \class TraderProfileWindow
      \brief Displays a single trader's profile.
   */
  class TraderProfileWindow : public QFrame {
    public:

      //! Constructs a TraderProfileWindow.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      TraderProfileWindow(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~TraderProfileWindow();

      //! Loads the models to display.
      /*!
        \param account The account whose trader models are to be loaded.
      */
      void Load(const Beam::DirectoryEntry& account);

    private:
      std::unique_ptr<Ui_TraderProfileWindow> m_ui;
      UserProfile* m_userProfile;
      std::shared_ptr<AccountEntitlementModel> m_entitlementModel;
      std::shared_ptr<AccountInfoModel> m_infoModel;
      std::shared_ptr<RiskModel> m_riskModel;
      std::shared_ptr<ComplianceModel> m_complianceModel;
  };
}

#endif
