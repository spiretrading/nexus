#ifndef SPIRE_SERVICEITEMWIDGET_HPP
#define SPIRE_SERVICEITEMWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/UI/UI.hpp"

class Ui_ServiceItemWidget;

namespace Spire {

  /*! \class ServiceItemWidget
      \brief Shows the main widget used to display a ServiceItem.
   */
  class ServiceItemWidget : public QWidget {
    public:

      //! Constructs a ServiceItemWidget.
      /*!
        \param userProfile The user's profile.
        \param entitlementModel The model used to display entitlements.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ServiceItemWidget(Beam::Ref<UserProfile> userProfile,
        const std::shared_ptr<AccountEntitlementModel>& entitlementModel,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~ServiceItemWidget();

    private:
      std::unique_ptr<Ui_ServiceItemWidget> m_ui;
      std::shared_ptr<AccountEntitlementModel> m_entitlementModel;

      void OnApply();
      void OnRevert();
  };
}

#endif
