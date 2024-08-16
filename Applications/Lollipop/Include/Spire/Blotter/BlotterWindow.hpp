#ifndef SPIRE_BLOTTER_WINDOW_HPP
#define SPIRE_BLOTTER_WINDOW_HPP
#include <boost/signals2/signal.hpp>
#include <QFrame>
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterTasksModel.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/PersistentWindow.hpp"

class QAction;
class QAbstactItemModel;
class QModelIndex;
class QStatusBar;
class QToolBar;
class Ui_BlotterWindow;

namespace Spire {

  /** Displays a BlotterModel. */
  class BlotterWindow : public QFrame, public UI::PersistentWindow {
    public:

      /**
       * Returns the BlotterWindow for a specified BlotterModel.
       * @param userProfile The user's profile.
       * @param model The model to display.
       */
      static BlotterWindow& GetBlotterWindow(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<BlotterModel> model);

      /**
       * Returns the BlotterWindow for a specified BlotterModel.
       * @param userProfile The user's profile.
       * @param model The model to display.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      static BlotterWindow& GetBlotterWindow(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<BlotterModel> model, QWidget* parent,
        Qt::WindowFlags flags = Qt::WindowFlags());

      ~BlotterWindow() override = default;

      /** Returns the blotter model being displayed. */
      const BlotterModel& GetModel() const;

      /** Returns the blotter model being displayed. */
      BlotterModel& GetModel();

      std::unique_ptr<UI::WindowSettings> GetWindowSettings() const override;

    protected:
      void showEvent(QShowEvent* event) override;
      void closeEvent(QCloseEvent* event) override;
      bool eventFilter(QObject* object, QEvent* event) override;

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
      std::vector<std::shared_ptr<Task>> m_tasksExecuted;
      boost::signals2::scoped_connection m_taskAddedConnection;
      boost::signals2::scoped_connection m_taskRemovedConnection;
      EventHandler m_eventHandler;

      BlotterWindow(UserProfile* userProfile, BlotterModel* model,
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
      void SetActive(bool isActive);
      void OnTaskAdded(const BlotterTasksModel::TaskEntry& entry);
      void OnTaskRemoved(const BlotterTasksModel::TaskEntry& entry);
      void OnActiveBlotterChanged(BlotterModel& blotter);
      void OnProfitAndLossUpdate(
        const SpirePortfolioController::UpdateEntry& update);
      void OnTaskState(const std::shared_ptr<Task>& task,
        const Task::StateEntry& update);
      void OnExecuteAction();
      void OnCancelAction();
      void OnActiveBlotterToggled(bool checked);
      void OnPinBlotterToggled(bool checked);
      void OnCurrentChanged(const QModelIndex& current,
        const QModelIndex& previous);
      void OnTaskContextMenu(const QPoint& position);
      void OnPositionsAdded(const QModelIndex& parent, int first, int last);
      void OnPositionsRemoved(const QModelIndex& parent, int first, int last);
      void OnTasksAdded(const QModelIndex& parent, int first, int last);
      void OnTasksRemoved(const QModelIndex& parent, int first, int last);
      void OnPinTaskToggled(const QModelIndex& topLeft,
        const QModelIndex& bottomRight, const QVector<int>& roles);
  };
}

#endif
