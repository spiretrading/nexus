#ifndef SPIRE_ACCOUNTENTITLEMENTWIDGET_HPP
#define SPIRE_ACCOUNTENTITLEMENTWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/UI/UI.hpp"

class Ui_AccountEntitlementWidget;

namespace Spire {

  /*! \class AccountEntitlementWidget
      \brief Shows an account's entitlements.
   */
  class AccountEntitlementWidget : public QWidget {
    public:

      //! Constructs an AccountEntitlementWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      AccountEntitlementWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~AccountEntitlementWidget();

      //! Sets the user's profile.
      /*!
        \param userProfile The user's profile.
        \param isReadOnly <code>true</code> iff the model is being shown
               strictly for display purposes.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile, bool isReadOnly);

      //! Returns the model being displayed.
      const AccountEntitlementModel& GetModel() const;

      //! Returns the model being displayed.
      AccountEntitlementModel& GetModel();

      //! Sets the model that this widget displays.
      /*!
        \param model The model to display.
      */
      void SetModel(const std::shared_ptr<AccountEntitlementModel>& model);

    private:
      std::unique_ptr<Ui_AccountEntitlementWidget> m_ui;
      UserProfile* m_userProfile;
      bool m_isReadOnly;
      std::shared_ptr<AccountEntitlementModel> m_model;
  };
}

#endif
