#ifndef SPIRE_TRADERPROFILEVIEWWIDGET_HPP
#define SPIRE_TRADERPROFILEVIEWWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Spire/AccountViewer/AccountViewer.hpp"

class Ui_TraderProfileViewWidget;

namespace Spire {
  class UserProfile;

  /*! \class TraderProfileViewWidget
      \brief Displays a trader's profile.
   */
  class TraderProfileViewWidget : public QWidget {
    public:

      //! Constructs a TraderProfileViewWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      TraderProfileViewWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~TraderProfileViewWidget();

      //! Initializes the TraderProfileViewWidget.
      /*!
        \param userProfile The user's profile.
        \param isReadOnly <code>true</code> iff the models being shown are
               strictly for display purposes.
        \param isPasswordReadOnly <code>true</code> iff the password can not be
               changed.
        \param accountInfoModel The AccountInfoModel to display.
        \param accountEntitlementModel The AccountEntitlementModel to display.
        \param riskModel The RiskModel to display.
        \param complianceModel The ComplianceModel to display.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile,
        bool isReadOnly, bool isPasswordReadOnly,
        std::shared_ptr<AccountInfoModel> accountInfoModel,
        std::shared_ptr<AccountEntitlementModel> accountEntitlementModel,
        std::shared_ptr<RiskModel> riskModel,
        std::shared_ptr<ComplianceModel> complianceModel);

      //! Commits all displayed info into their respective models.
      void Commit();

      //! Reloads all models.
      void Reload();

    private:
      std::unique_ptr<Ui_TraderProfileViewWidget> m_ui;
  };
}

#endif
