#ifndef SPIRE_TRADERITEMWIDGET_HPP
#define SPIRE_TRADERITEMWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_TraderItemWidget;

namespace Spire {

  /*! \class TraderItemWidget
      \brief Shows the main widget used to display a TraderItem.
   */
  class TraderItemWidget : public QWidget {
    public:

      //! Constructs a TraderItemWidget.
      /*!
        \param userProfile The user's profile.
        \param infoModel The model storing the account's info.
        \param entitlementModel The model used to display entitlements.
        \param riskModel The model used to display the risk parameters.
        \param complianceModel The model used to display compliance rules.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      TraderItemWidget(Beam::Ref<UserProfile> userProfile,
        std::shared_ptr<AccountInfoModel> infoModel,
        std::shared_ptr<AccountEntitlementModel> entitlementModel,
        std::shared_ptr<RiskModel> riskModel,
        std::shared_ptr<ComplianceModel> complianceModel,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~TraderItemWidget();

    private:
      std::unique_ptr<Ui_TraderItemWidget> m_ui;
      UserProfile* m_userProfile;
      std::shared_ptr<AccountInfoModel> m_infoModel;
      std::shared_ptr<AccountEntitlementModel> m_entitlementModel;
      std::shared_ptr<RiskModel> m_riskModel;
      std::shared_ptr<ComplianceModel> m_complianceModel;

      void OnApply();
      void OnReload();
      void OnOpenBlotter();
  };
}

#endif
