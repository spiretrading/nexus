#ifndef SPIRE_BLOTTERWINDOW_HPP
#define SPIRE_BLOTTERWINDOW_HPP
#include <Beam/Queues/TaskQueue.hpp>
#include <boost/signals2/signal.hpp>
#include <QFrame>
#include <QTimer>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterTasksModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/PersistentWindow.hpp"

class QAction;
class QAbstactItemModel;
class QModelIndex;
class QStatusBar;
class QToolBar;
class Ui_BlotterWindow;

namespace Spire {

  /*! \class BlotterWindow
      \brief Displays a BlotterModel.
   */
  class BlotterWindow : public QFrame, public UI::PersistentWindow {
    public:

      //! Returns the BlotterWindow for a specified BlotterModel.
      /*!
        \param userProfile The user's profile.
        \param model The model to display.
      */
      static BlotterWindow& GetBlotterWindow(
        Beam::RefType<UserProfile> userProfile,
        Beam::RefType<BlotterModel> model);

      //! Returns the BlotterWindow for a specified BlotterModel.
      /*!
        \param userProfile The user's profile.
        \param model The model to display.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      static BlotterWindow& GetBlotterWindow(
        Beam::RefType<UserProfile> userProfile,
        Beam::RefType<BlotterModel> model, QWidget* parent,
        Qt::WindowFlags flags = 0);

      virtual ~BlotterWindow();

      //! Returns the blotter model being displayed.
      const BlotterModel& GetModel() const;

      //! Returns the blotter model being displayed.
      BlotterModel& GetModel();

      virtual std::unique_ptr<UI::WindowSettings> GetWindowSettings() const;

    protected:
      virtual void showEvent(QShowEvent* event);
      virtual void closeEvent(QCloseEvent* event);
      virtual bool eventFilter(QObject* object, QEvent* event);

    private:
      friend class BlotterWindowSettings;
      std::unique_ptr<Ui_BlotterWindow> m_ui;
      UserProfile* m_userProfile;
      BlotterModel* m_model;
      boost::signals2::scoped_connection m_activeBlotterChangedConnection;
      boost::signals2::scoped_connection m_profitAndLossUpdateConnection;
      UI::CustomVariantSortFilterProxyModel* m_proxyModel;
      QStatusBar* m_statusBar;
      UI::ValueLabel* m_totalProfitAndLossLabel;
      UI::ValueLabel* m_unrealizedProfitAndLossLabel;
      UI::ValueLabel* m_realizedProfitAndLossLabel;
      UI::ValueLabel* m_feesLabel;
      UI::ValueLabel* m_costBasisLabel;
      QToolBar* m_toolbar;
      QAction* m_executeAction;
      QAction* m_cancelAction;
      QAction* m_toggleActiveBlotterAction;
      QAction* m_pinBlotterAction;
      std::vector<std::shared_ptr<BlotterTasksModel::TaskContext>>
        m_tasksExecuted;
      boost::signals2::scoped_connection m_taskAddedConnection;
      boost::signals2::scoped_connection m_taskRemovedConnection;
      QTimer m_updateTimer;
      Beam::TaskQueue m_slotHandler;

      BlotterWindow(UserProfile* userProfile, BlotterModel* model,
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
      void SetActive(bool isActive);
      void OnTaskAdded(const BlotterTasksModel::TaskEntry& entry);
      void OnTaskRemoved(const BlotterTasksModel::TaskEntry& entry);
      void OnActiveBlotterChanged(BlotterModel& blotter);
      void OnProfitAndLossUpdate(
        const SpirePortfolioMonitor::UpdateEntry& update);
      void OnTaskState(std::weak_ptr<BlotterTasksModel::TaskContext> task,
        const Beam::Tasks::Task::StateEntry& update);
      void OnExecuteAction();
      void OnCancelAction();
      void OnActiveBlotterToggled(bool checked);
      void OnPinBlotterToggled(bool checked);
      void OnCurrentChanged(const QModelIndex& current,
        const QModelIndex& previous);
      void OnTaskContextMenu(const QPoint& position);
      void OnUpdateTimer();
  };
}

#endif
