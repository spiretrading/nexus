#ifndef SPIRE_TOOLBAR_HPP
#define SPIRE_TOOLBAR_HPP
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <boost/bimap.hpp>
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/UI/PersistentWindow.hpp"
#include "Spire/UI/WindowSettings.hpp"
#include "Spire/UI/UI.hpp"

class QAction;
class QMenu;
class Ui_Toolbar;

namespace Spire {
namespace UI {

  /*! \class Toolbar
      \brief Contains all tools available to the user.
   */
  class Toolbar : public QWidget, public PersistentWindow {
    public:

      //! Constructs the Toolbar.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      Toolbar(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual std::unique_ptr<WindowSettings> GetWindowSettings() const;

    protected:
      virtual void closeEvent(QCloseEvent* event);

    private:
      std::unique_ptr<Ui_Toolbar> m_ui;
      UserProfile* m_userProfile;
      QMenu* m_recentlyClosedMenu;
      QMenu* m_blotterMenu;
      QAction* m_minimizeAllAction;
      QAction* m_restoreAllAction;
      QAction* m_importExportAction;
      QAction* m_newCanvasAction;
      QAction* m_profileAction;
      QAction* m_logoutAction;
      QAction* m_openTimeAndSalesAction;
      QAction* m_openBookViewAction;
      QAction* m_openChartWindowAction;
      QAction* m_openDashboardWindowAction;
      QAction* m_openOrderImbalanceIndicatorAction;
      QAction* m_keyBindingsAction;
      QAction* m_newBlotterAction;
      QAction* m_accountViewAction;
      QAction* m_openPortfolioViewerAction;
      std::unordered_map<QAction*, BlotterModel*> m_actionToBlotter;
      boost::signals2::scoped_connection m_blotterAddedConnection;
      boost::signals2::scoped_connection m_blotterRemovedConnection;

      void OnBlotterAdded(BlotterModel& blotter);
      void OnBlotterRemoved(BlotterModel& blotter);
      void OnMinimizeAllAction();
      void OnRestoreAllAction();
      void OnImportExportAction();
      void OnRecentlyClosedButtonPressed();
      void OnRecentlyClosedAction(QAction* action);
      void OnNewCanvasAction();
      void OnProfileAction();
      void OnKeyBindingsAction();
      void OnLogoutAction();
      void OnOpenTimeAndSalesAction();
      void OnOpenBookViewAction();
      void OnOpenChartWindowAction();
      void OnOpenDashboardWindowAction();
      void OnOpenOrderImbalanceIndicatorAction();
      void OnOpenBlotterAction();
      void OnNewBlotterAction();
      void OnAccountViewAction();
      void OnOpenPortfolioViewerAction();
  };
}
}

#endif
