#ifndef SPIRE_CANVASWINDOW_HPP
#define SPIRE_CANVASWINDOW_HPP
#include <deque>
#include <QFrame>
#include <QUndoStack>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/Catalog/Catalog.hpp"
#include "Spire/UI/PersistentWindow.hpp"
#include "Spire/UI/WindowSettings.hpp"

class QAction;
class QLabel;
class QStatusBar;
class QTableWidgetItem;
class QToolBar;
class Ui_CanvasWindow;

namespace Spire {
namespace UI {

  /*! \class CanvasWindow
      \brief Used to create/edit CanvasNodes.
   */
  class CanvasWindow : public QFrame, public PersistentWindow {
    public:

      //! Constructs a CanvasWindow.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CanvasWindow(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      //! Returns the CanvasNodeModel being displayed.
      const CanvasNodeModel& GetCanvasNodeModel() const;

      //! Returns the CanvasNodeModel being displayed.
      CanvasNodeModel& GetCanvasNodeModel();

      virtual std::unique_ptr<WindowSettings> GetWindowSettings() const;

    protected:
      virtual void closeEvent(QCloseEvent* event);

      virtual bool eventFilter(QObject* object, QEvent* event);

    private:
      typedef std::function<void ()> StateContext;
      friend class CanvasWindowSettings;
      std::unique_ptr<Ui_CanvasWindow> m_ui;
      UserProfile* m_userProfile;
      QStatusBar* m_statusBar;
      int m_state;
      std::deque<StateContext> m_stateContexts;
      QLabel* m_stateLabel;
      QLabel* m_typeLabel;
      QLabel* m_xCoordinateLabel;
      QLabel* m_yCoordinateLabel;
      QLabel* m_errorLabel;
      QToolBar* m_toolbar;
      QAction* m_newCustomNodeAction;
      QAction* m_openAction;
      QAction* m_saveAction;
      QAction* m_saveAsAction;
      QAction* m_editAction;
      QAction* m_protectionAction;
      QAction* m_deleteAction;
      QAction* m_cutAction;
      QAction* m_copyAction;
      QAction* m_pasteAction;
      QAction* m_undoAction;
      QAction* m_redoAction;
      QAction* m_backwardAction;
      QAction* m_forwardAction;
      QAction* m_aggregateAction;
      QAction* m_referenceAction;
      QUndoStack m_undoStack;

      std::vector<QAction*> GetActions() const;
      void ApplyCommand(QUndoCommand* command);
      void SetState(int state);
      void UpdateStatus();
      bool OnBeginEdit(const CanvasNode& node);
      void OnCanvasTableCommand(QUndoCommand* command);
      QUndoCommand* MakeReference(std::vector<const CanvasNode*> nodes);
      QUndoCommand* LinkReferences(const CanvasNode& target,
        const std::vector<const CanvasNode*>& references);
      void OnNewCustomNodeAction();
      void OnOpenAction();
      void OnSaveAction();
      void OnSaveAsAction();
      void OnEditAction();
      void OnProtectionAction();
      void OnDeleteAction();
      void OnCutAction();
      void OnCopyAction();
      void OnPasteAction();
      void OnUndoAction();
      void OnRedoAction();
      void OnBackwardAction();
      void OnForwardAction();
      void OnAggregateAction();
      void OnReferenceAction();
      void OnItemSelectionChanged();
      void OnCurrentItemChanged(QTableWidgetItem* current,
        QTableWidgetItem* previous);
      void OnCellActivated(int row, int column);
      void OnCellChanged(int row, int column);
      void OnCatalogEntryDeleted(CatalogEntry& entry);
      void OnClipboardDataChanged();
  };
}
}

#endif
