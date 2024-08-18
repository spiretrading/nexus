#include "Spire/UI/CanvasWindow.hpp"
#include <QClipboard>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/ControlNodes/AggregateNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeActivator.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/CanvasView/AddRootCommand.hpp"
#include "Spire/CanvasView/CopyNodeCommand.hpp"
#include "Spire/CanvasView/CustomNodeDialog.hpp"
#include "Spire/CanvasView/CutNodeCommand.hpp"
#include "Spire/CanvasView/DeleteNodeCommand.hpp"
#include "Spire/CanvasView/PasteNodeCommand.hpp"
#include "Spire/CanvasView/PlaceNodeCommand.hpp"
#include "Spire/CanvasView/ReplaceNodeCommand.hpp"
#include "Spire/CanvasView/CanvasNodeProtectionDialog.hpp"
#include "Spire/CanvasView/CanvasTableWidget.hpp"
#include "Spire/CanvasView/ValueCell.hpp"
#include "Spire/Catalog/CatalogEntry.hpp"
#include "Spire/Catalog/CatalogWindow.hpp"
#include "Spire/Catalog/CatalogTabModel.hpp"
#include "Spire/Catalog/RegistryCatalogEntry.hpp"
#include "Spire/Catalog/UserCatalogEntry.hpp"
#include "Spire/UI/CanvasWindowSettings.hpp"
#include "Spire/UI/IgnoreCommandException.hpp"
#include "Spire/UI/MacroUndoCommand.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_CanvasWindow.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  enum {
    READY,
    READ_ONLY,
    ENTER,
    EDIT,
    POINT_STATE,
  };

  void CheckAggregate(const vector<const CanvasNode*>& selection,
      QAction* action) {
    bool foundTask = false;
    for(const auto& node : selection) {
      if(IsRoot(*node) &&
          node->GetType().GetCompatibility(OrderReferenceType::GetInstance()) ==
          CanvasType::Compatibility::EQUAL) {
        if(foundTask) {
          action->setEnabled(true);
          return;
        } else {
          foundTask = true;
        }
      }
    }
    action->setEnabled(false);
  }

  void CheckReference(const vector<const CanvasNode*>& selection,
      QAction* action) {
    boost::optional<const CanvasNode&> root;
    for(const auto& node : selection) {
      if(!root) {
        root = GetRoot(*node);
      } else if(&*root != &GetRoot(*node)) {
        action->setEnabled(false);
        return;
      }
    }
    action->setEnabled(selection.size() > 1);
  }

  QLabel* MakeStateLabel() {
    auto label = new QLabel();
    QSizePolicy sizePolicy(QSizePolicy::Maximum,
      label->sizePolicy().verticalPolicy());
    sizePolicy.setHorizontalStretch(1);
    label->setSizePolicy(sizePolicy);
    label->setContentsMargins(12, 0, 12, 0);
    return label;
  }

  QLabel* MakeTypeLabel() {
    auto label = new QLabel();
    QSizePolicy sizePolicy(QSizePolicy::Ignored,
      label->sizePolicy().verticalPolicy());
    sizePolicy.setHorizontalStretch(1);
    label->setSizePolicy(sizePolicy);
    return label;
  }

  QLabel* MakeErrorLabel() {
    auto label = new QLabel();
    QSizePolicy sizePolicy(QSizePolicy::Ignored,
      label->sizePolicy().verticalPolicy());
    sizePolicy.setHorizontalStretch(1);
    label->setSizePolicy(sizePolicy);
    return label;
  }

  QWidget* MakeCoordinatesLabel(QLabel*& xCoordinateLabel,
      QLabel*& yCoordinateLabel) {
    auto coordinates = new QWidget();
    auto coordinateLayout = new QHBoxLayout();
    coordinateLayout->setContentsMargins(4, 0, 4, 0);
    xCoordinateLabel = new QLabel();
    QFontMetrics metrics(xCoordinateLabel->font());
    xCoordinateLabel->setMinimumWidth(metrics.horizontalAdvance("Row: 999"));
    coordinateLayout->addWidget(xCoordinateLabel);
    yCoordinateLabel = new QLabel();
    yCoordinateLabel->setMinimumWidth(metrics.horizontalAdvance("Col: 999"));
    coordinateLayout->addWidget(yCoordinateLabel);
    coordinates->setLayout(coordinateLayout);
    coordinates->setSizePolicy(QSizePolicy::Preferred,
      coordinates->sizePolicy().verticalPolicy());
    return coordinates;
  }
}

CanvasWindow::CanvasWindow(Ref<UserProfile> userProfile, QWidget* parent,
    Qt::WindowFlags flags)
    : QFrame(parent, flags),
      m_ui(make_unique<Ui_CanvasWindow>()),
      m_userProfile(userProfile.Get()) {
  m_state = READY;
  m_ui->setupUi(this);
  m_ui->m_canvasTable->SetUserProfile(Ref(*m_userProfile));
  m_ui->m_canvasTable->installEventFilter(this);
  m_ui->m_canvasTable->ConnectBeginEditSignal(
    std::bind(&CanvasWindow::OnBeginEdit, this, std::placeholders::_1));
  m_ui->m_canvasTable->ConnectCommandSignal(
    std::bind(&CanvasWindow::OnCanvasTableCommand, this,
    std::placeholders::_1));
  m_statusBar = new QStatusBar(this);
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(m_statusBar->sizePolicy().hasHeightForWidth());
  m_statusBar->setSizePolicy(sizePolicy);
  m_ui->verticalLayout->addWidget(m_statusBar);
  m_stateLabel = MakeStateLabel();
  m_statusBar->addWidget(m_stateLabel);
  m_typeLabel = MakeTypeLabel();
  m_statusBar->addWidget(m_typeLabel);
  m_errorLabel = MakeErrorLabel();
  m_statusBar->addWidget(m_errorLabel);
  QWidget* coordinates = MakeCoordinatesLabel(m_xCoordinateLabel,
    m_yCoordinateLabel);
  m_statusBar->addWidget(coordinates);
  m_toolbar = new QToolBar(this);
  m_ui->verticalLayout->insertWidget(0, m_toolbar);
  m_newCustomNodeAction = new QAction(this);
  m_newCustomNodeAction->setText(tr("New Custom Node"));
  m_newCustomNodeAction->setToolTip(tr("New Custom Node"));
  m_newCustomNodeAction->setIcon(QIcon(":/icons/branch_element_new.png"));
  m_newCustomNodeAction->setShortcut(QKeySequence::New);
  addAction(m_newCustomNodeAction);
  m_toolbar->addAction(m_newCustomNodeAction);
  connect(m_newCustomNodeAction, &QAction::triggered, this,
    &CanvasWindow::OnNewCustomNodeAction);
  m_openAction = new QAction(this);
  m_openAction->setText(tr("Open"));
  m_openAction->setToolTip(tr("Open catalog"));
  m_openAction->setIcon(QIcon(":/icons/dictionary.png"));
  m_openAction->setShortcut(QKeySequence::Open);
  addAction(m_openAction);
  m_toolbar->addAction(m_openAction);
  connect(m_openAction, &QAction::triggered, this, &CanvasWindow::OnOpenAction);
  m_saveAction = new QAction(this);
  m_saveAction->setText(tr("Save"));
  m_saveAction->setToolTip(tr("Save"));
  m_saveAction->setIcon(QIcon(":/icons/floppy_disk.png"));
  m_saveAction->setShortcut(QKeySequence::Save);
  addAction(m_saveAction);
  m_toolbar->addAction(m_saveAction);
  connect(m_saveAction, &QAction::triggered, this, &CanvasWindow::OnSaveAction);
  m_saveAsAction = new QAction(this);
  m_saveAsAction->setText(tr("Save As"));
  m_saveAsAction->setToolTip(tr("Save As"));
  m_saveAsAction->setIcon(QIcon(":/icons/save_as.png"));
  m_saveAsAction->setShortcut(QKeySequence::SaveAs);
  addAction(m_saveAsAction);
  m_toolbar->addAction(m_saveAsAction);
  connect(m_saveAsAction, &QAction::triggered, this,
    &CanvasWindow::OnSaveAsAction);
  if(m_userProfile->GetCatalogSettings().HasRegistryAccess()) {
    m_saveToRegistryAction = new QAction(this);
    m_saveToRegistryAction->setText(tr("Save To Registry"));
    m_saveToRegistryAction->setToolTip(tr("Save To Registry"));
    m_saveToRegistryAction->setIcon(QIcon(":/icons/server_from_client.png"));
    addAction(m_saveToRegistryAction);
    m_toolbar->addAction(m_saveToRegistryAction);
    connect(m_saveToRegistryAction, &QAction::triggered, this,
      &CanvasWindow::OnSaveToRegistryAction);
  } else {
    m_saveToRegistryAction = nullptr;
  }
  m_editAction = new QAction(this);
  m_editAction->setText(tr("Edit"));
  m_editAction->setToolTip(tr("Edit"));
  m_editAction->setIcon(QIcon(":/icons/table_edit.png"));
  addAction(m_editAction);
  m_toolbar->addAction(m_editAction);
  connect(m_editAction, &QAction::triggered, this, &CanvasWindow::OnEditAction);
  m_protectionAction = new QAction(this);
  m_protectionAction->setText(tr("Protection"));
  m_protectionAction->setToolTip(tr("Protect node."));
  m_protectionAction->setIcon(QIcon(":/icons/safe.png"));
  addAction(m_protectionAction);
  m_toolbar->addAction(m_protectionAction);
  connect(m_protectionAction, &QAction::triggered, this,
    &CanvasWindow::OnProtectionAction);
  m_deleteAction = new QAction(this);
  m_deleteAction->setText(tr("Delete"));
  m_deleteAction->setToolTip(tr("Delete"));
  m_deleteAction->setShortcut(QKeySequence::Delete);
  addAction(m_deleteAction);
  connect(m_deleteAction, &QAction::triggered, this,
    &CanvasWindow::OnDeleteAction);
  m_toolbar->addSeparator();
  m_cutAction = new QAction(this);
  m_cutAction->setText(tr("Cut"));
  m_cutAction->setToolTip(tr("Cut"));
  m_cutAction->setIcon(QIcon(":/icons/cut.png"));
  m_cutAction->setShortcut(QKeySequence::Cut);
  addAction(m_cutAction);
  m_toolbar->addAction(m_cutAction);
  connect(m_cutAction, &QAction::triggered, this, &CanvasWindow::OnCutAction);
  m_copyAction = new QAction(this);
  m_copyAction->setText(tr("Copy"));
  m_copyAction->setToolTip(tr("Copy"));
  m_copyAction->setShortcut(QKeySequence::Copy);
  m_copyAction->setIcon(QIcon(":/icons/copy.png"));
  addAction(m_copyAction);
  m_toolbar->addAction(m_copyAction);
  connect(m_copyAction, &QAction::triggered, this, &CanvasWindow::OnCopyAction);
  m_pasteAction = new QAction(this);
  m_pasteAction->setText(tr("Paste"));
  m_pasteAction->setToolTip(tr("Paste"));
  m_pasteAction->setShortcut(QKeySequence::Paste);
  m_pasteAction->setIcon(QIcon(":/icons/paste.png"));
  m_pasteAction->setEnabled(CheckClipboardForCanvasNode());
  addAction(m_pasteAction);
  m_toolbar->addAction(m_pasteAction);
  connect(m_pasteAction, &QAction::triggered, this,
    &CanvasWindow::OnPasteAction);
  m_toolbar->addSeparator();
  m_undoAction = new QAction(this);
  m_undoAction->setText(tr("Undo"));
  m_undoAction->setToolTip(tr("Undo"));
  m_undoAction->setShortcut(QKeySequence::Undo);
  m_undoAction->setIcon(QIcon(":/icons/undo.png"));
  m_undoAction->setEnabled(false);
  addAction(m_undoAction);
  m_toolbar->addAction(m_undoAction);
  connect(m_undoAction, &QAction::triggered, this, &CanvasWindow::OnUndoAction);
  m_redoAction = new QAction(this);
  m_redoAction->setText(tr("Redo"));
  m_redoAction->setToolTip(tr("Redo"));
  m_redoAction->setShortcut(QKeySequence::Redo);
  m_redoAction->setIcon(QIcon(":/icons/redo.png"));
  m_redoAction->setEnabled(false);
  addAction(m_redoAction);
  m_toolbar->addAction(m_redoAction);
  connect(m_redoAction, &QAction::triggered, this, &CanvasWindow::OnRedoAction);
  m_backwardAction = new QAction(this);
  m_backwardAction->setText(tr("Backward"));
  m_backwardAction->setToolTip(tr("Backward"));
  m_backwardAction->setIcon(QIcon(":/icons/table_previous.png"));
  addAction(m_backwardAction);
  m_toolbar->addAction(m_backwardAction);
  connect(m_backwardAction, &QAction::triggered, this,
    &CanvasWindow::OnBackwardAction);
  m_forwardAction = new QAction(this);
  m_forwardAction->setText(tr("Forward"));
  m_forwardAction->setToolTip(tr("Forward"));
  m_forwardAction->setIcon(QIcon(":/icons/table_next.png"));
  addAction(m_forwardAction);
  m_toolbar->addAction(m_forwardAction);
  connect(m_forwardAction, &QAction::triggered, this,
    &CanvasWindow::OnForwardAction);
  m_toolbar->addSeparator();
  m_aggregateAction = new QAction(this);
  m_aggregateAction->setText(tr("Aggregate"));
  m_aggregateAction->setToolTip(tr("Aggregate a schema selection."));
  m_aggregateAction->setIcon(QIcon(":/icons/package.png"));
  m_aggregateAction->setEnabled(false);
  addAction(m_aggregateAction);
  m_toolbar->addAction(m_aggregateAction);
  connect(m_aggregateAction, &QAction::triggered, this,
    &CanvasWindow::OnAggregateAction);
  m_referenceAction = new QAction(this);
  m_referenceAction->setText(tr("Reference"));
  m_referenceAction->setToolTip(tr("Reference a node."));
  m_referenceAction->setIcon(QIcon(":/icons/link.png"));
  m_referenceAction->setEnabled(false);
  addAction(m_referenceAction);
  m_toolbar->addAction(m_referenceAction);
  connect(m_referenceAction, &QAction::triggered, this,
    &CanvasWindow::OnReferenceAction);
  QFontMetrics metrics(font());
  m_ui->m_canvasTable->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 8);
  connect(m_ui->m_canvasTable, &CanvasTableWidget::itemSelectionChanged,
    this, &CanvasWindow::OnItemSelectionChanged);
  connect(m_ui->m_canvasTable, &CanvasTableWidget::currentItemChanged, this,
    &CanvasWindow::OnCurrentItemChanged);
  OnCurrentItemChanged(nullptr, nullptr);
  connect(m_ui->m_canvasTable, &CanvasTableWidget::cellActivated, this,
    &CanvasWindow::OnCellActivated);
  connect(m_ui->m_canvasTable, &CanvasTableWidget::cellChanged, this,
    &CanvasWindow::OnCellChanged);
  connect(&m_undoStack, &QUndoStack::canUndoChanged, m_undoAction,
    &QAction::setEnabled);
  connect(&m_undoStack, &QUndoStack::canRedoChanged, m_redoAction,
    &QAction::setEnabled);
  connect(QApplication::clipboard(), &QClipboard::dataChanged, this,
    &CanvasWindow::OnClipboardDataChanged);
  UpdateStatus();
}

const CanvasNodeModel& CanvasWindow::GetCanvasNodeModel() const {
  return *m_ui->m_canvasTable;
}

CanvasNodeModel& CanvasWindow::GetCanvasNodeModel() {
  return *m_ui->m_canvasTable;
}

unique_ptr<WindowSettings> CanvasWindow::GetWindowSettings() const {
  unique_ptr<WindowSettings> settings =
    std::make_unique<CanvasWindowSettings>(*this);
  return settings;
}

void CanvasWindow::closeEvent(QCloseEvent* event) {
  unique_ptr<CanvasWindowSettings> settings(new CanvasWindowSettings(*this));
  m_userProfile->AddRecentlyClosedWindow(std::move(settings));
  QFrame::closeEvent(event);
}

bool CanvasWindow::eventFilter(QObject* object, QEvent* event) {
  if(m_state == READY && object == m_ui->m_canvasTable &&
      event->type() == QEvent::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    int key = keyEvent->key();
    if(key == Qt::Key_Enter || key == Qt::Key_Return) {
      auto currentNode = m_ui->m_canvasTable->GetCurrentNode();
      if(currentNode.is_initialized() && IsRoot(*currentNode)) {
        Activate(*currentNode, *m_userProfile, this);
        return true;
      }
    }
  }
  return QWidget::eventFilter(object, event);
}

vector<QAction*> CanvasWindow::GetActions() const {
  vector<QAction*> actions;
  actions.push_back(m_newCustomNodeAction);
  actions.push_back(m_openAction);
  actions.push_back(m_saveAction);
  actions.push_back(m_saveAsAction);
  if(m_saveToRegistryAction != nullptr) {
    actions.push_back(m_saveToRegistryAction);
  }
  actions.push_back(m_editAction);
  actions.push_back(m_protectionAction);
  actions.push_back(m_deleteAction);
  actions.push_back(m_cutAction);
  actions.push_back(m_copyAction);
  actions.push_back(m_pasteAction);
  actions.push_back(m_undoAction);
  actions.push_back(m_redoAction);
  actions.push_back(m_backwardAction);
  actions.push_back(m_forwardAction);
  actions.push_back(m_aggregateAction);
  actions.push_back(m_referenceAction);
  return actions;
}

void CanvasWindow::ApplyCommand(QUndoCommand* command) {
  try {
    m_undoStack.push(command);
  } catch(IgnoreCommandException&) {}
}

void CanvasWindow::SetState(int state) {
  if(state == EDIT) {
    std::vector<bool> actionContext;
    std::vector<QAction*> actions = GetActions();
    for(auto i = actions.begin(); i != actions.end(); ++i) {
      actionContext.push_back((*i)->isEnabled());
    }
    m_stateContexts.push_back(
      [=] {
        for(std::size_t i = 0; i < actions.size(); ++i) {
          QAction* action = actions[i];
          if(action == this->m_undoAction) {
            action->setEnabled(this->m_undoStack.canUndo());
          } else if(action == this->m_redoAction) {
            action->setEnabled(this->m_undoStack.canRedo());
          } else {
            action->setEnabled(actionContext[i]);
          }
        }
      });
    for(auto i = actions.begin(); i != actions.end(); ++i) {
      (*i)->setEnabled(false);
    }
  }
  if(state == READY && m_state == EDIT) {
    StateContext stateContext = m_stateContexts.back();
    m_stateContexts.pop_back();
    stateContext();
  }
  m_state = state;
  UpdateStatus();
}

void CanvasWindow::UpdateStatus() {
  m_xCoordinateLabel->setText(tr("Row: ") +
    QString::number(std::max(0, m_ui->m_canvasTable->currentRow())));
  m_yCoordinateLabel->setText(tr("Col: ") +
    QString::number(std::max(0, m_ui->m_canvasTable->currentColumn())));
  m_typeLabel->clear();
  m_errorLabel->clear();
  QTableWidgetItem* current = m_ui->m_canvasTable->currentItem();
  if(current != nullptr) {
    auto& node = static_cast<CanvasCell*>(current)->GetNode();
    if(dynamic_cast<ValueCell*>(current) != nullptr) {
      ValueCell* value = static_cast<ValueCell*>(current);
      if(!value->GetErrors().empty()) {
        QString errorMessage = QString::fromStdString(
          value->GetErrors().front().GetErrorMessage());
        m_errorLabel->setText(errorMessage);
        m_errorLabel->setToolTip(errorMessage);
      }
    }
    m_typeLabel->setText(QString::fromStdString(node.GetType().GetName()));
    m_typeLabel->setToolTip(QString::fromStdString(node.GetType().GetName()));
  }
  if(m_state == READY) {
    m_stateLabel->setText(tr("Ready"));
  } else if(m_state == READ_ONLY) {
    m_stateLabel->setText(tr("Read-only"));
  } else if(m_state == ENTER) {
    m_stateLabel->setText(tr("Enter"));
  } else if(m_state == EDIT) {
    m_stateLabel->setText(tr("Edit"));
  } else if(m_state == POINT_STATE) {
    m_stateLabel->setText(tr("Point"));
  }
}

bool CanvasWindow::OnBeginEdit(const CanvasNode& node) {
  SetState(EDIT);
  return true;
}

void CanvasWindow::OnCanvasTableCommand(QUndoCommand* command) {
  if(command != nullptr) {
    ApplyCommand(command);
  }
  SetState(READY);
  UpdateStatus();
}

QUndoCommand* CanvasWindow::MakeReference(vector<const CanvasNode*> nodes) {
  sort(nodes.begin(), nodes.end(),
    [&] (const CanvasNode* lhs, const CanvasNode* rhs) -> bool {
      CanvasNodeModel::Coordinate lhsCoordinate =
        this->m_ui->m_canvasTable->GetCoordinate(*lhs);
      CanvasNodeModel::Coordinate rhsCoordinate =
        this->m_ui->m_canvasTable->GetCoordinate(*rhs);
      return std::tie(lhsCoordinate.m_column, lhsCoordinate.m_row) <
        std::tie(rhsCoordinate.m_column, rhsCoordinate.m_row);
    });
  vector<const CanvasNode*> references(nodes.begin() + 1, nodes.end());
  return LinkReferences(*nodes.front(), references);
}

QUndoCommand* CanvasWindow::LinkReferences(const CanvasNode& target,
    const vector<const CanvasNode*>& references) {
  MacroUndoCommand* command = new MacroUndoCommand();
  for(auto i = references.begin(); i != references.end(); ++i) {
    CanvasNodeModel::Coordinate coordinate =
      m_ui->m_canvasTable->GetCoordinate(**i);
    command->AddCommand(new ReplaceNodeCommand(Ref(*m_ui->m_canvasTable),
      coordinate, ReferenceNode(GetPath(**i, target))));
  }
  return command;
}

void CanvasWindow::OnNewCustomNodeAction() {
  CustomNodeDialog dialog(Ref(*m_userProfile), this);
  SetState(POINT_STATE);
  int result = dialog.exec();
  SetState(READY);
  if(result == QDialog::Rejected) {
    return;
  }
  int row = m_ui->m_canvasTable->currentRow();
  int column = m_ui->m_canvasTable->currentColumn();
  PlaceNodeCommand* command = new PlaceNodeCommand(Ref(*m_ui->m_canvasTable),
    CanvasNodeModel::Coordinate(row, column), *dialog.GetNode());
  ApplyCommand(command);
  UpdateStatus();
}

void CanvasWindow::OnOpenAction() {
  auto currentNode = m_ui->m_canvasTable->GetCurrentNode();
  CatalogWindow::Filter catalogFilter;
  if(!currentNode.is_initialized() || IsRoot(*currentNode)) {
    catalogFilter = CatalogWindow::DisplayAllFilter;
  } else {
    catalogFilter = CatalogWindow::SubstitutionFilter(*currentNode);
  }
  CatalogWindow catalog(Ref(*m_userProfile), catalogFilter, this);
  if(catalog.exec() == QDialog::Rejected) {
    return;
  }
  vector<CatalogEntry*> selection = catalog.GetSelection();
  assert(!selection.empty());
  CanvasNodeModel::Coordinate coordinate(m_ui->m_canvasTable->currentRow(),
    m_ui->m_canvasTable->currentColumn());
  MacroUndoCommand* command = new MacroUndoCommand();
  for(auto i = selection.begin(); i != selection.end(); ++i) {
    command->AddCommand(new PlaceNodeCommand(Ref(*m_ui->m_canvasTable),
      coordinate, (*i)->GetNode()));
  }
  ApplyCommand(command);
  UpdateStatus();
}

void CanvasWindow::OnSaveAction() {
  auto node = CanvasNodeModel::StripIdentity(
    *m_ui->m_canvasTable->GetCurrentNode());
  auto& catalogSettings = m_userProfile->GetCatalogSettings();
  auto catalogEntry = catalogSettings.FindEntry(*node);
  if(!catalogEntry.is_initialized()) {
    return OnSaveAsAction();
  }
  if(catalogEntry->IsReadOnly()) {
    return OnSaveAsAction();
  }
  auto newEntry = catalogEntry->SetNode(*node);
  try {
    catalogSettings.Replace(*catalogEntry, std::move(newEntry));
  } catch(std::exception& e) {
    QMessageBox::warning(nullptr, QObject::tr("Error"), e.what());
  }
}

void CanvasWindow::OnSaveAsAction() {
  const CanvasNode& saveNode = *m_ui->m_canvasTable->GetCurrentNode();
  CatalogWindow catalog(Ref(*m_userProfile), saveNode, false, this);
  if(catalog.exec() == QDialog::Rejected) {
    return;
  }
  string entryName = catalog.GetEntryNames().front();
  CatalogEntry* oldEntry = nullptr;
  CatalogSettings& catalogSettings = m_userProfile->GetCatalogSettings();
  for(auto i = catalogSettings.GetCatalogEntries().begin();
      i != catalogSettings.GetCatalogEntries().end(); ++i) {
    if((*i)->GetName() == entryName) {
      oldEntry = i->get();
      break;
    }
  }
  unique_ptr<CatalogEntry> newEntry;
  if(oldEntry == nullptr) {
    newEntry = std::make_unique<UserCatalogEntry>(entryName, saveNode,
      ":/icons/scroll2.png", "", catalogSettings.GetSettingsPath());
  } else {
    if(oldEntry->IsReadOnly()) {
      QMessageBox::warning(nullptr, QObject::tr("Warning"),
        QObject::tr("Unable to save to a read-only catalog entry."));
      return OnSaveAction();
    }
    newEntry = oldEntry->SetNode(saveNode);
  }
  try {
    if(oldEntry == nullptr) {
      catalogSettings.Add(std::move(newEntry));
    } else {
      catalogSettings.Replace(*oldEntry, std::move(newEntry));
    }
  } catch(std::exception& e) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"), e.what());
    return OnSaveAction();
  }
}

void CanvasWindow::OnSaveToRegistryAction() {
  const CanvasNode& saveNode = *m_ui->m_canvasTable->GetCurrentNode();
  CatalogWindow catalog(Ref(*m_userProfile), saveNode, true, this);
  if(catalog.exec() == QDialog::Rejected) {
    return;
  }
  string entryName = catalog.GetEntryNames().front();
  CatalogEntry* oldEntry = nullptr;
  CatalogSettings& catalogSettings = m_userProfile->GetCatalogSettings();
  for(auto i = catalogSettings.GetCatalogEntries().begin();
      i != catalogSettings.GetCatalogEntries().end(); ++i) {
    if((*i)->GetName() == entryName) {
      oldEntry = i->get();
      break;
    }
  }
  unique_ptr<CatalogEntry> newEntry;
  if(oldEntry == nullptr) {
    newEntry = std::make_unique<RegistryCatalogEntry>(entryName,
      ":/icons/scroll2.png", "", saveNode,
      CatalogSettings::GetCatalogLibraryRegistryPath(),
      m_userProfile->GetServiceClients().GetRegistryClient());
  } else {
    if(oldEntry->IsReadOnly()) {
      QMessageBox::warning(nullptr, QObject::tr("Warning"),
        QObject::tr("Unable to save to a read-only catalog entry."));
      return OnSaveAction();
    }
    newEntry = oldEntry->SetNode(saveNode);
  }
  try {
    if(oldEntry == nullptr) {
      catalogSettings.Add(std::move(newEntry));
    } else {
      catalogSettings.Replace(*oldEntry, std::move(newEntry));
    }
  } catch(std::exception& e) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"), e.what());
    return OnSaveAction();
  }
}

void CanvasWindow::OnEditAction() {
  m_ui->m_canvasTable->Edit(*m_ui->m_canvasTable->GetCurrentNode());
}

void CanvasWindow::OnProtectionAction() {
  const CanvasNode& source = *m_ui->m_canvasTable->GetCurrentNode();
  CanvasNodeProtectionDialog dialog(source, this);
  SetState(POINT_STATE);
  int result = dialog.exec();
  SetState(READY);
  if(result == QDialog::Rejected) {
    return;
  }
  CanvasNodeModel::Coordinate coordinate =
    m_ui->m_canvasTable->GetCoordinate(source);
  QUndoCommand* command = new ReplaceNodeCommand(Ref(*m_ui->m_canvasTable),
    coordinate, *dialog.GetNode());
  ApplyCommand(command);
  UpdateStatus();
}

void CanvasWindow::OnDeleteAction() {
  vector<const CanvasNode*> nodes = m_ui->m_canvasTable->GetSelectedNodes();
  MacroUndoCommand* command = new MacroUndoCommand();
  for(auto i = nodes.begin(); i != nodes.end(); ++i) {
    CanvasNodeModel::Coordinate coordinate =
      m_ui->m_canvasTable->GetCoordinate(**i);
    command->AddCommand(new DeleteNodeCommand(Ref(*m_ui->m_canvasTable),
      coordinate));
  }
  ApplyCommand(command);
  UpdateStatus();
}

void CanvasWindow::OnCutAction() {
  vector<const CanvasNode*> nodes = m_ui->m_canvasTable->GetSelectedNodes();
  if(nodes.empty()) {
    QMessageBox::warning(this, tr("Spire"), tr("No nodes selected to cut."));
    return;
  } else if(nodes.size() > 1) {
    QMessageBox::warning(this, tr("Spire"),
      tr("Cut may only be used on a single node, use delete for multiple "
        "nodes."));
    return;
  }
  const CanvasNode& node = *nodes.front();
  CanvasNodeModel::Coordinate coordinate =
    m_ui->m_canvasTable->GetCoordinate(node);
  CutNodeCommand* command = new CutNodeCommand(Ref(*m_ui->m_canvasTable),
    coordinate);
  ApplyCommand(command);
  UpdateStatus();
}

void CanvasWindow::OnCopyAction() {
  vector<const CanvasNode*> nodes = m_ui->m_canvasTable->GetSelectedNodes();
  if(nodes.empty()) {
    QMessageBox::warning(this, tr("Spire"), tr("No nodes selected to copy."));
    return;
  } else if(nodes.size() > 1) {
    QMessageBox::warning(this, tr("Spire"),
      tr("Copy may only be used on a single node."));
    return;
  }
  const CanvasNode& node = *nodes.front();
  CopyNodeCommand* command = new CopyNodeCommand(node);
  ApplyCommand(command);
  UpdateStatus();
}

void CanvasWindow::OnPasteAction() {
  QModelIndex currentIndex = m_ui->m_canvasTable->currentIndex();
  CanvasNodeModel::Coordinate coordinate(currentIndex.row(),
    currentIndex.column());
  PasteNodeCommand* command = new PasteNodeCommand(Ref(*m_ui->m_canvasTable),
    coordinate);
  ApplyCommand(command);
  UpdateStatus();
}

void CanvasWindow::OnUndoAction() {
  m_undoStack.undo();
  UpdateStatus();
}

void CanvasWindow::OnRedoAction() {
  m_undoStack.redo();
  UpdateStatus();
}

void CanvasWindow::OnBackwardAction() {
  m_ui->m_canvasTable->NavigateBackward();
}

void CanvasWindow::OnForwardAction() {
  m_ui->m_canvasTable->NavigateForward();
}

void CanvasWindow::OnAggregateAction() {
  auto selectedNodes = m_ui->m_canvasTable->GetSelectedNodes();
  vector<std::unique_ptr<CanvasNode>> tasks;
  CanvasNodeModel::Coordinate topLeftCoordinate(INT_MAX, INT_MAX);
  MacroUndoCommand* command = new MacroUndoCommand();
  for(auto i = selectedNodes.begin(); i != selectedNodes.end(); ++i) {
    if(IsRoot(**i) && (*i)->GetType().GetCompatibility(
        OrderReferenceType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      CanvasNodeModel::Coordinate coordinate =
        m_ui->m_canvasTable->GetCoordinate(**i);
      if(coordinate < topLeftCoordinate) {
        topLeftCoordinate = coordinate;
      }
      tasks.emplace_back(CanvasNode::Clone(**i));
      command->AddCommand(new DeleteNodeCommand(Ref(*m_ui->m_canvasTable),
        coordinate));
    }
  }
  AggregateNode aggregate(std::move(tasks));
  command->AddCommand(new AddRootCommand(Ref(*m_ui->m_canvasTable),
    topLeftCoordinate, aggregate));
  ApplyCommand(command);
  UpdateStatus();
}

void CanvasWindow::OnReferenceAction() {
  vector<const CanvasNode*> references;
  const CanvasNode* target = nullptr;
  int targetCount = 0;
  const vector<const CanvasNode*>& selectedNodes =
    m_ui->m_canvasTable->GetSelectedNodes();
  for(auto i = selectedNodes.begin(); i != selectedNodes.end(); ++i) {
    const ReferenceNode* reference = dynamic_cast<const ReferenceNode*>(*i);
    if(reference != nullptr) {
      references.push_back(reference);
    } else {
      target = *i;
      ++targetCount;
    }
  }
  QUndoCommand* command;
  if(targetCount != 1) {
    command = MakeReference(selectedNodes);
  } else if(target != nullptr && !references.empty()) {
    command = LinkReferences(*target, references);
  } else {
    return;
  }
  ApplyCommand(command);
  UpdateStatus();
}

void CanvasWindow::OnItemSelectionChanged() {
  CheckAggregate(m_ui->m_canvasTable->GetSelectedNodes(), m_aggregateAction);
  CheckReference(m_ui->m_canvasTable->GetSelectedNodes(), m_referenceAction);
  UpdateStatus();
}

void CanvasWindow::OnCurrentItemChanged(QTableWidgetItem* current,
    QTableWidgetItem* previous) {
  bool currentLive = (current != nullptr);
  bool isRoot = currentLive && IsRoot(
    *m_ui->m_canvasTable->GetCurrentNode());
  m_saveAction->setEnabled(isRoot);
  m_saveAsAction->setEnabled(isRoot);
  if(m_saveToRegistryAction != nullptr) {
    m_saveToRegistryAction->setEnabled(isRoot);
  }
  m_editAction->setEnabled(currentLive);
  m_protectionAction->setEnabled(currentLive);
  m_deleteAction->setEnabled(currentLive);
  m_cutAction->setEnabled(currentLive);
  m_copyAction->setEnabled(currentLive);
  m_forwardAction->setEnabled(currentLive);
  m_backwardAction->setEnabled(currentLive);
  UpdateStatus();
}

void CanvasWindow::OnCellActivated(int row, int column) {
  ValueCell* currentCell = dynamic_cast<ValueCell*>(
    m_ui->m_canvasTable->item(row, column));
  if(currentCell != nullptr) {
    m_ui->m_canvasTable->Edit(currentCell->GetNode());
  } else {
    OnOpenAction();
  }
  UpdateStatus();
}

void CanvasWindow::OnCellChanged(int row, int column) {
  QTableWidgetItem* item = m_ui->m_canvasTable->item(row, column);
  UpdateStatus();
  if(item != m_ui->m_canvasTable->currentItem()) {
    return;
  }
  OnCurrentItemChanged(item, nullptr);
}

void CanvasWindow::OnCatalogEntryDeleted(CatalogEntry& entry) {
  UpdateStatus();
}

void CanvasWindow::OnClipboardDataChanged() {
  m_pasteAction->setEnabled(CheckClipboardForCanvasNode());
}
