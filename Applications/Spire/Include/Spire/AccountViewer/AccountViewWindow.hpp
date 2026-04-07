#ifndef SPIRE_ACCOUNTVIEWWINDOW_HPP
#define SPIRE_ACCOUNTVIEWWINDOW_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QFrame>
#include "Spire/AccountViewer/AccountViewer.hpp"

class QModelIndex;
class Ui_AccountViewWindow;

namespace Spire {
  class UserProfile;

  /*! \class AccountViewWindow
      \brief Displays and manages the accounts in a TradingGroup.
   */
  class AccountViewWindow : public QFrame {
    public:

      //! Constructs an AccountViewWindow.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      AccountViewWindow(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~AccountViewWindow();

    private:
      std::unique_ptr<Ui_AccountViewWindow> m_ui;
      UserProfile* m_userProfile;
      std::unique_ptr<AccountViewModel> m_model;

      void OnEntryExpanded(const QModelIndex& index);
      void OnCurrentChanged(const QModelIndex& current,
        const QModelIndex& previous);
  };
}

#endif
