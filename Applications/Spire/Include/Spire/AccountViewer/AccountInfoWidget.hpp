#ifndef SPIRE_ACCOUNTINFOWIDGET_HPP
#define SPIRE_ACCOUNTINFOWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_AccountInfoWidget;

namespace Spire {

  /*! \class AccountInfoWidget
      \brief Shows an account's info.
   */
  class AccountInfoWidget : public QWidget {
    public:

      //! Constructs an AccountInfoWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      AccountInfoWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~AccountInfoWidget();

      //! Sets the user's profile.
      /*!
        \param userProfile The user's profile.
        \param isReadOnly <code>true</code> iff the model is being shown
               strictly for display purposes.
        \param isPasswordReadOnly <code>true</code> iff the password can not be
               changed.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile, bool isReadOnly,
        bool isPasswordReadOnly);

      //! Returns the model being displayed.
      const AccountInfoModel& GetModel() const;

      //! Returns the model being displayed.
      AccountInfoModel& GetModel();

      //! Sets the model that this widget displays.
      /*!
        \param model The model to display.
      */
      void SetModel(const std::shared_ptr<AccountInfoModel>& model);

      //! Commits changes to the model.
      void Commit();

    private:
      std::unique_ptr<Ui_AccountInfoWidget> m_ui;
      UserProfile* m_userProfile;
      std::shared_ptr<AccountInfoModel> m_model;
      bool m_isReadOnly;

      void OnUpdatePasswordButton();
  };
}

#endif
