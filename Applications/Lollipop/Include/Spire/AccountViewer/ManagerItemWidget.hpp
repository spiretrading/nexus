#ifndef SPIRE_MANAGERITEMWIDGET_HPP
#define SPIRE_MANAGERITEMWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_ManagerItemWidget;

namespace Spire {

  /*! \class ManagerItemWidget
      \brief Shows the main widget used to display a ManagerItem.
   */
  class ManagerItemWidget : public QWidget {
    public:

      //! Constructs a ManagerItemWidget.
      /*!
        \param userProfile The user's profile.
        \param infoModel The model used to display the account info.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ManagerItemWidget(Beam::Ref<UserProfile> userProfile,
        const std::shared_ptr<AccountInfoModel>& infoModel,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~ManagerItemWidget();

    private:
      std::unique_ptr<Ui_ManagerItemWidget> m_ui;
      std::shared_ptr<AccountInfoModel> m_infoModel;

      void OnApply();
      void OnRevert();
  };
}

#endif
