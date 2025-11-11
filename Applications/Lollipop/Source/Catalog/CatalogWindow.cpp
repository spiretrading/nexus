#include "Spire/Catalog/CatalogWindow.hpp"
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QMimeType>
#include <QTabBar>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"
#include "Spire/Catalog/CatalogEntry.hpp"
#include "Spire/Catalog/CatalogTabModel.hpp"
#include "Spire/Catalog/CatalogTabView.hpp"
#include "Spire/Catalog/UserCatalogEntry.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_CatalogWindow.h"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

bool CatalogWindow::DisplayAllFilter(const CatalogEntry& entry) {
  return true;
}

CatalogWindow::Filter CatalogWindow::EqualTypeFilter(const CanvasType& type) {
  std::shared_ptr<CanvasType> filterType = type;
  Filter filter =
    [=] (const CatalogEntry& entry) -> bool {
      return entry.GetNode().GetType().GetCompatibility(*filterType) ==
        CanvasType::Compatibility::EQUAL;
    };
  return filter;
}

CatalogWindow::Filter CatalogWindow::SubstitutionFilter(
    const CanvasNode& node) {
  std::shared_ptr<CanvasNode> root(CanvasNode::Clone(GetRoot(node)));
  auto replacementNode = &*root->FindNode(GetFullName(node));
  Filter filter =
    [=] (const CatalogEntry& entry) -> bool {
      CanvasNodeBuilder builder(*root);
      try {
        builder.Replace(*replacementNode, CanvasNode::Clone(entry.GetNode()));
        builder.Make();
      } catch(std::exception&) {
        return false;
      }
      return true;
    };
  return filter;
}

CatalogWindow::CatalogWindow(Ref<UserProfile> userProfile, QWidget* parent,
    Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_filter(DisplayAllFilter),
      m_ui(std::make_unique<Ui_CatalogWindow>()),
      m_mode(LOADING),
      m_userProfile(userProfile.get()) {
  Initialize();
}

CatalogWindow::CatalogWindow(Ref<UserProfile> userProfile,
    const CanvasNode& node, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_CatalogWindow>()),
      m_mode(SAVING),
      m_userProfile(userProfile.get()) {
  m_filter = DisplayAllFilter;
  Initialize();
  SetDefaultSearchName(node);
}

CatalogWindow::CatalogWindow(Ref<UserProfile> userProfile,
    const Filter& filter, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_filter(filter),
      m_ui(std::make_unique<Ui_CatalogWindow>()),
      m_mode(LOADING),
      m_userProfile(userProfile.get()) {
  Initialize();
}

CatalogWindow::CatalogWindow(Ref<UserProfile> userProfile,
    const Filter& filter, const CanvasNode& node, QWidget* parent,
    Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_filter(filter),
      m_ui(std::make_unique<Ui_CatalogWindow>()),
      m_mode(SAVING),
      m_userProfile(userProfile.get()) {
  Initialize();
  SetDefaultSearchName(node);
}

CatalogWindow::~CatalogWindow() {}

vector<CatalogEntry*> CatalogWindow::GetSelection() const {
  auto& settings = m_userProfile->GetCatalogSettings();
  vector<CatalogEntry*> selection;
  auto entryNames = GetEntryNames();
  for(const auto& entryName : entryNames) {
    for(const auto& entry : settings.GetCatalogEntries()) {
      auto lhs = entryName;
      to_lower(lhs);
      auto rhs = entry->GetName();
      to_lower(rhs);
      if(lhs == rhs) {
        selection.push_back(entry.get());
      }
    }
  }
  return selection;
}

boost::optional<const CatalogTabView&> CatalogWindow::GetCurrentTab() const {
  auto currentWidget = m_ui->m_categoryTabs->currentWidget();
  if(currentWidget == nullptr || currentWidget == m_newTab) {
    return none;
  }
  return *static_cast<const CatalogTabView*>(currentWidget);
}

boost::optional<CatalogTabView&> CatalogWindow::GetCurrentTab() {
  auto currentWidget = m_ui->m_categoryTabs->currentWidget();
  if(currentWidget == nullptr || currentWidget == m_newTab) {
    return none;
  }
  return *static_cast<CatalogTabView*>(currentWidget);
}

vector<string> CatalogWindow::GetEntryNames() const {
  auto currentTab = GetCurrentTab();
  if(!currentTab) {
    return vector<string>();
  }
  return currentTab->GetEntryNames();
}

const CatalogSettings& CatalogWindow::GetSettings() const {
  return m_userProfile->GetCatalogSettings();
}

CatalogSettings& CatalogWindow::GetSettings() {
  return m_userProfile->GetCatalogSettings();
}

bool CatalogWindow::eventFilter(QObject* object, QEvent* event) {
  if(object == m_ui->m_categoryTabs) {
    if(event->type() == QEvent::DragMove || event->type() ==
        QEvent::DragEnter) {
      OnDragMoveTab(static_cast<QDragMoveEvent*>(event));
      return true;
    } else if(event->type() == QEvent::Drop) {
      OnDropTab(static_cast<QDropEvent*>(event));
      return true;
    }
  }
  return QDialog::eventFilter(object, event);
}

void CatalogWindow::Initialize() {
  m_ui->setupUi(this);
  m_ui->m_categoryTabs->installEventFilter(this);
  if(m_mode == SAVING) {
    setWindowTitle(tr("Catalog Save As - Spire"));
  } else {
    setWindowTitle(tr("Catalog - Spire"));
  }
  while(m_ui->m_categoryTabs->count() > 0) {
    m_ui->m_categoryTabs->removeTab(0);
  }
  m_newTab = new QWidget();
  m_ui->m_categoryTabs->addTab(m_newTab, "+");
  auto& settings = m_userProfile->GetCatalogSettings();
  const auto& tabs = settings.GetCatalogTabs();
  for(const auto& tab : tabs) {
    AddTab(*tab);
  }
  m_ui->m_categoryTabs->setCurrentIndex(0);
  GetCurrentTab()->GiveFocus();
  connect(m_ui->m_buttonBox, &QDialogButtonBox::accepted, this,
    &CatalogWindow::OnAccept);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &CatalogWindow::reject);
  m_tabAddedConnection = settings.ConnectCatalogTabModelAddedSignal(
    std::bind(&CatalogWindow::OnCatalogTabAdded, this, std::placeholders::_1));
  m_tabRemovedConnection = settings.ConnectCatalogTabModelRemovedSignal(
    std::bind(&CatalogWindow::OnCatalogTabRemoved, this,
    std::placeholders::_1));
}

void CatalogWindow::SetDefaultSearchName(const CanvasNode& node) {
  string defaultName;
  auto catalogEntry = m_userProfile->GetCatalogSettings().FindEntry(node);
  if(catalogEntry.is_initialized()) {
    defaultName = catalogEntry->GetName();
  } else if(dynamic_cast<const CustomNode*>(&node) != nullptr) {
    defaultName = node.GetText();
  }
  m_searchBarModel.SetLabel(defaultName);
}

void CatalogWindow::OnDragMoveTab(QDragMoveEvent* event) {
  if(event->mimeData()->hasFormat(QString::fromStdString(
      CatalogEntry::MIME_TYPE))) {
    if(event->type() == QEvent::DragEnter) {
      event->accept();
      return;
    }
    auto dropIndex = m_ui->m_categoryTabs->tabBar()->tabAt(event->pos());
    if(dropIndex == -1 || dropIndex == m_ui->m_categoryTabs->count() - 1) {
      event->ignore();
      return;
    }
    event->acceptProposedAction();
  }
}

void CatalogWindow::OnDropTab(QDropEvent* event) {
  auto dropIndex = m_ui->m_categoryTabs->tabBar()->tabAt(event->pos());
  if(dropIndex == -1 || dropIndex == m_ui->m_categoryTabs->count() - 1) {
    event->ignore();
    return;
  }
  auto tab = static_cast<CatalogTabView*>(
    m_ui->m_categoryTabs->widget(dropIndex));
  tab->SendTabDropEvent(*event);
}

void CatalogWindow::OnNameChanged(const CatalogTabModel& model) {
  for(auto i = 1; i < m_ui->m_categoryTabs->count() - 1; ++i) {
    auto view = static_cast<CatalogTabView*>(m_ui->m_categoryTabs->widget(i));
    if(&view->GetModel() == &model) {
      m_ui->m_categoryTabs->setTabText(i,
        QString::fromStdString(model.GetName()));
    }
  }
}

void CatalogWindow::OnCatalogTabAdded(CatalogTabModel& model) {
  AddTab(model);
}

void CatalogWindow::OnCatalogTabRemoved(CatalogTabModel& model) {
  for(auto i = m_ui->m_categoryTabs->count() - 2; i >= 0; --i) {
    auto view = static_cast<CatalogTabView*>(m_ui->m_categoryTabs->widget(i));
    if(&view->GetModel() == &model) {
      m_ui->m_categoryTabs->removeTab(i);
      m_viewConnections.disconnect(view);
    }
  }
  if(m_ui->m_categoryTabs->currentIndex() ==
      m_ui->m_categoryTabs->count() - 1) {
    m_ui->m_categoryTabs->setCurrentIndex(m_ui->m_categoryTabs->count() - 2);
  }
}

void CatalogWindow::AddTab(CatalogTabModel& model) {
  auto saving = m_mode == SAVING;
  auto view = new CatalogTabView(Ref(model), Ref(m_searchBarModel), m_filter,
    Ref(*m_userProfile), saving);
  m_ui->m_categoryTabs->insertTab(m_ui->m_categoryTabs->count() - 1, view,
    QString::fromStdString(model.GetName()));
  m_ui->m_categoryTabs->setCurrentIndex(m_ui->m_categoryTabs->count() - 2);
  if(m_mode == LOADING) {
    auto viewConnection = view->ConnectCatalogEntryActivatedSignal(
      std::bind(&CatalogWindow::OnCatalogEntryActivated, this));
    m_viewConnections.add(view, viewConnection);
  } else {
    auto viewConnection = view->ConnectCatalogEntryActivatedSignal(
      std::bind(&CatalogWindow::OnAccept, this));
    m_viewConnections.add(view, viewConnection);
  }
  auto nameConnection = model.ConnectNameSignal(
    std::bind(&CatalogWindow::OnNameChanged, this, std::ref(model)));
  m_modelConnections.add(view, nameConnection);
}

void CatalogWindow::OnAccept() {
  auto currentTab = GetCurrentTab();
  if(!currentTab || currentTab->GetEntryNames().empty()) {
    return;
  }
  auto selection = GetSelection();
  if(m_mode == SAVING && !selection.empty()) {
    QMessageBox confirmMessage(this);
    confirmMessage.setIcon(QMessageBox::Warning);
    confirmMessage.setWindowTitle(tr("Confirm Overwrite"));
    confirmMessage.setText("The catalog entry already exists.");
    confirmMessage.setInformativeText("Do you want to overwrite this entry?");
    confirmMessage.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmMessage.setDefaultButton(QMessageBox::No);
    auto result = confirmMessage.exec();
    if(result == QMessageBox::No) {
      return;
    }
  } else if(m_mode == LOADING && selection.empty()) {
    return;
  }
  Q_EMIT accept();
}

void CatalogWindow::OnCatalogEntryActivated() {
  Q_EMIT accept();
}
