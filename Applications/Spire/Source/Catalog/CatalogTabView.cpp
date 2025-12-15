#include "Spire/Catalog/CatalogTabView.hpp"
#include <unordered_set>
#include <boost/algorithm/string.hpp>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"
#include "Spire/Catalog/CatalogIconSelectionDialog.hpp"
#include "Spire/Catalog/CatalogSearchBarModel.hpp"
#include "Spire/Catalog/CatalogTabModel.hpp"
#include "Spire/Catalog/PersistentCatalogEntry.hpp"
#include "Spire/LegacyUI/LineInputDialog.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "ui_CatalogTabView.h"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace boost::uuids;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  class CanvasNodeTypeFilterProxyModel : public QSortFilterProxyModel {
    public:
      CanvasNodeTypeFilterProxyModel(const CatalogTabView::Filter& filter,
          Ref<CatalogTabModel> model, QObject* parent)
          : QSortFilterProxyModel(parent),
            m_filter(filter),
            m_model(model.get()) {
        for(const auto& entry : m_model->GetEntries()) {
          if(m_filter(*entry)) {
            m_entries.insert(entry);
          }
        }
      }

    protected:
      virtual bool filterAcceptsRow(int sourceRow,
          const QModelIndex& sourceParent) const {
        auto entry = m_model->GetEntries()[sourceRow];
        if(m_entries.find(entry) != m_entries.end()) {
          return QSortFilterProxyModel::filterAcceptsRow(sourceRow,
            sourceParent);
        }
        return false;
      }

    private:
      CatalogTabView::Filter m_filter;
      CatalogTabModel* m_model;
      unordered_set<const CatalogEntry*> m_entries;
  };
}

CatalogTabView::CatalogTabView(Ref<CatalogTabModel> model,
    Ref<CatalogSearchBarModel> searchBarModel, const Filter& filter,
    Ref<UserProfile> userProfile, bool saving, QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_CatalogTabView>()),
      m_model(model.get()),
      m_searchBarModel(searchBarModel.get()),
      m_userProfile(userProfile.get()),
      m_isAllTab(m_model == &m_userProfile->GetCatalogSettings().GetAllTab()),
      m_searchBarSelected(false) {
  m_ui->setupUi(this);
  m_ui->m_searchLayout->setSpacing(
    scale_width(m_ui->m_searchLayout->spacing()));
  m_ui->m_searchButton->setFixedSize(scale(m_ui->m_searchButton->size()));
  m_ui->m_searchButton->setIconSize(m_ui->m_searchButton->iconSize());
  m_ui->m_searchBar->setMinimumWidth(
    scale_width(m_ui->m_searchBar->minimumWidth()));
  m_ui->m_searchBar->setMaximumHeight(
    scale_height(m_ui->m_searchBar->maximumHeight()));
  m_ui->m_catalogEntryList->setIconSize(
    scale(m_ui->m_catalogEntryList->iconSize()));
  m_ui->m_catalogEntryList->setSpacing(
    scale_width(m_ui->m_catalogEntryList->spacing()));
  m_ui->m_descriptionText->setMinimumHeight(
    scale_height(m_ui->m_descriptionText->minimumHeight()));
  m_proxyModel = new CanvasNodeTypeFilterProxyModel(filter, model, this);
  m_proxyModel->setSourceModel(m_model);
  m_proxyModel->setDynamicSortFilter(true);
  m_proxyModel->sort(0, Qt::AscendingOrder);
  m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  if(saving) {
    m_ui->m_searchButton->setIcon(QIcon(":/icons/save_as.png"));
    m_ui->m_catalogEntryList->setSelectionMode(
      QAbstractItemView::SingleSelection);
  } else {
    m_ui->m_searchButton->setIcon(QIcon(":/icons/magnifying_glass.png"));
    m_ui->m_catalogEntryList->setSelectionMode(
      QAbstractItemView::ExtendedSelection);
  }
  m_ui->m_searchBar->installEventFilter(this);
  m_searchBarModelConnection = m_searchBarModel->ConnectUpdateSignal(
    std::bind(&CatalogTabView::OnSearchBarUpdate, this, std::placeholders::_1,
    std::placeholders::_2));
  m_ui->m_catalogEntryList->installEventFilter(this);
  m_ui->m_catalogEntryList->setModel(m_proxyModel);
  m_ui->m_catalogEntryList->setResizeMode(QListView::Adjust);
  connect(m_ui->m_catalogEntryList, &QListView::activated, this,
    &CatalogTabView::OnActivated);
  connect(m_ui->m_searchBar, &QLineEdit::textEdited, this,
    &CatalogTabView::OnSearchTextEdited);
  connect(m_ui->m_searchBar, &QLineEdit::returnPressed, this,
    &CatalogTabView::OnSearchTextReturn);
  connect(m_ui->m_catalogEntryList->selectionModel(),
    &QItemSelectionModel::selectionChanged, this,
    &CatalogTabView::OnSelectionChanged);
  connect(m_ui->m_descriptionText, &QTextEdit::textChanged, this,
    &CatalogTabView::OnDescriptionChanged);
  m_ui->m_descriptionText->resize(
    m_ui->m_descriptionText->width(), scale_height(60));
}

CatalogTabModel& CatalogTabView::GetModel() {
  return *m_model;
}

const CatalogTabModel& CatalogTabView::GetModel() const {
  return *m_model;
}

vector<string> CatalogTabView::GetEntryNames() const {
  vector<string> entryNames;
  string textBar = m_searchBarModel->GetText();
  split(entryNames, textBar, is_any_of(","));
  auto i = entryNames.begin();
  while(i != entryNames.end()) {
    trim(*i);
    if(i->empty()) {
      i = entryNames.erase(i);
    } else {
      ++i;
    }
  }
  return entryNames;
}

vector<CatalogEntry*> CatalogTabView::GetSelection() const {
  QModelIndexList indexes =
    m_ui->m_catalogEntryList->selectionModel()->selectedIndexes();
  vector<CatalogEntry*> entries;
  for(auto i = indexes.begin(); i != indexes.end(); ++i) {
    QModelIndex translatedIndex = m_proxyModel->mapToSource(*i);
    entries.push_back(m_model->GetEntries()[translatedIndex.row()]);
  }
  return entries;
}

void CatalogTabView::GiveFocus() {
  m_ui->m_searchBar->setFocus(Qt::TabFocusReason);
  m_ui->m_searchBar->selectAll();
}

void CatalogTabView::SendTabDropEvent(QDropEvent& event) {
  vector<CatalogEntry*> entries = CatalogEntry::DecodeFromMimeData(
    *event.mimeData(), m_userProfile->GetCatalogSettings());
  for(auto i = entries.begin(); i != entries.end(); ++i) {
    m_model->Add(Ref(**i));
  }
  event.acceptProposedAction();
}

connection CatalogTabView::ConnectCatalogEntryActivatedSignal(
    const CatalogEntryActivatedSignal::slot_type& slot) const {
  return m_catalogEntryActivatedSignal.connect(slot);
}

bool CatalogTabView::eventFilter(QObject* object, QEvent* event) {
  if(object == m_ui->m_catalogEntryList) {
    if(event->type() == QEvent::KeyPress) {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      if(keyEvent->key() == Qt::Key_Delete) {
        CatalogSettings& settings = m_userProfile->GetCatalogSettings();
        vector<CatalogEntry*> selection = GetSelection();
        Delete(selection);
        return true;
      }
    } else if(event->type() == QEvent::ContextMenu) {
      return OnCatalogContextMenu(static_cast<QContextMenuEvent*>(event));
    }
  } else if(object == m_ui->m_searchBar) {
    if(event->type() == QEvent::KeyPress) {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      if(keyEvent->key() == Qt::Key_Enter ||
          keyEvent->key() == Qt::Key_Return) {
        if(m_proxyModel->rowCount() != 0) {
          OnActivated(m_proxyModel->index(0, 0));
          return true;
        }
      }
    } else if(event->type() == QEvent::FocusIn) {
      if(m_searchBarModel->GetMode() != CatalogSearchBarModel::INPUT) {
        m_ui->m_searchBar->selectAll();
        m_searchBarSelected = true;
        return true;
      }
    } else if(event->type() == QEvent::MouseButtonPress) {
      if(m_searchBarSelected) {
        m_searchBarSelected = false;
        return true;
      }
    }
  }
  return QWidget::eventFilter(object, event);
}

void CatalogTabView::hideEvent(QHideEvent* event) {
  CatalogSettings& settings = m_userProfile->GetCatalogSettings();
  for(auto i = m_modifiedEntries.begin(); i != m_modifiedEntries.end(); ++i) {
    for(auto j = settings.GetCatalogEntries().begin();
        j != settings.GetCatalogEntries().end(); ++j) {
      if(*i == (*j)->GetUid()) {
        static_cast<PersistentCatalogEntry&>(**j).Save();
      }
    }
  }
  m_modifiedEntries.clear();
}

bool CatalogTabView::OnCatalogContextMenu(QContextMenuEvent* event) {
  QModelIndex sourceIndex = m_ui->m_catalogEntryList->indexAt(event->pos());
  if(!sourceIndex.isValid()) {
    return QWidget::eventFilter(m_ui->m_catalogEntryList, event);
  }
  QModelIndex itemIndex = m_proxyModel->mapToSource(sourceIndex);
  CatalogEntry* entry = m_model->GetEntries()[itemIndex.row()];
  if(entry->IsReadOnly()) {
    QMenu itemMenu;
    QAction disabledAction("Entry can not be edited.", &itemMenu);
    disabledAction.setEnabled(false);
    itemMenu.addAction(&disabledAction);
    itemMenu.exec(event->globalPos());
    return true;
  }
  QMenu itemMenu;
  QAction renameAction("Rename", &itemMenu);
  itemMenu.addAction(&renameAction);
  QAction iconAction("Change Icon", &itemMenu);
  itemMenu.addAction(&iconAction);
  QAction deleteAction("Delete", &itemMenu);
  itemMenu.addAction(&deleteAction);
  QAction* selectedAction = itemMenu.exec(event->globalPos());
  if(selectedAction == &renameAction) {
    LineInputDialog renameDialog("Rename", "Name:", entry->GetName(), this);
    if(renameDialog.exec() == QDialog::Rejected) {
      return true;
    }
    string entryName = renameDialog.GetInput();
    CatalogSettings& catalogSettings = m_userProfile->GetCatalogSettings();
    unique_ptr<CatalogEntry> newEntry = entry->SetName(entryName);
    try {
      catalogSettings.Replace(*entry, std::move(newEntry));
    } catch(std::exception& e) {
      QMessageBox::warning(nullptr, QObject::tr("Error"),
        QString::fromStdString(e.what()));
      return true;
    }
  } else if(selectedAction == &iconAction) {
    CatalogIconSelectionDialog dialog(this);
    if(dialog.exec() == QDialog::Rejected) {
      return true;
    }
    string iconPath = dialog.GetIconPath();
    CatalogSettings& catalogSettings = m_userProfile->GetCatalogSettings();
    unique_ptr<CatalogEntry> newEntry = entry->SetIconPath(iconPath);
    try {
      catalogSettings.Replace(*entry, std::move(newEntry));
    } catch(std::exception& e) {
      QMessageBox::warning(nullptr, QObject::tr("Error"),
        QString::fromStdString(e.what()));
      return true;
    }
  } else if(selectedAction == &deleteAction) {
    vector<CatalogEntry*> deletion;
    deletion.push_back(entry);
    Delete(deletion);
  }
  return true;
}

void CatalogTabView::Delete(const vector<CatalogEntry*>& entries) {
  CatalogSettings& settings = m_userProfile->GetCatalogSettings();
  vector<CatalogEntry*> deletionList;
  bool deletedReadOnly = false;
  for(auto i = entries.begin(); i != entries.end(); ++i) {
    if(m_isAllTab) {
      if((*i)->IsReadOnly()) {
        deletedReadOnly = true;
        continue;
      }
      deletionList.push_back(*i);
    } else {
      m_model->Remove(**i);
    }
  }
  while(!deletionList.empty()) {
    CatalogEntry* entry = deletionList.back();
    deletionList.pop_back();
    settings.Remove(*entry);
  }
  if(deletedReadOnly) {
    QMessageBox::warning(this, QObject::tr("Warning"),
      QObject::tr("Unable to delete read-only catalog entries."));
  }
}

void CatalogTabView::OnSearchTextEdited(const QString& text) {
  m_searchBarModel->SetInput(text.toStdString());
}

void CatalogTabView::OnSearchTextReturn() {
  if(m_ui->m_searchBar->text().isEmpty() || m_proxyModel->rowCount() == 0) {
    return;
  }
  m_catalogEntryActivatedSignal();
}

void CatalogTabView::OnSelectionChanged(const QItemSelection& selected,
    const QItemSelection& deselected) {
  if(m_ui->m_searchBar->hasFocus()) {
    return;
  }
  QModelIndexList selectedIndexes =
    m_ui->m_catalogEntryList->selectionModel()->selectedIndexes();
  if(selectedIndexes.empty()) {
    m_searchBarModel->SetLabel("Search Current Tab");
    m_ui->m_descriptionText->clear();
    m_ui->m_descriptionText->setReadOnly(true);
  } else {
    vector<const CatalogEntry*> catalogEntries;
    if(selectedIndexes.size() == 1) {
      QModelIndex translatedIndex = m_proxyModel->mapToSource(
        selectedIndexes.front());
      CatalogEntry* entry = m_model->GetEntries()[translatedIndex.row()];
      catalogEntries.push_back(entry);
      m_ui->m_descriptionText->setText(QString::fromStdString(
        entry->GetDescription()));
      m_ui->m_descriptionText->setReadOnly(entry->IsReadOnly());
    } else {
      for(QModelIndexList::const_iterator i = selectedIndexes.begin();
          i != selectedIndexes.end(); ++i) {
        QModelIndex translatedIndex = m_proxyModel->mapToSource(*i);
        CatalogEntry* entry = m_model->GetEntries()[translatedIndex.row()];
        catalogEntries.push_back(entry);
      }
      m_ui->m_descriptionText->clear();
      m_ui->m_descriptionText->setReadOnly(true);
    }
    m_searchBarModel->SetCatalogEntries(catalogEntries);
  }
}

void CatalogTabView::OnActivated(const QModelIndex& index) {
  QModelIndex translatedIndex = m_proxyModel->mapToSource(index);
  CatalogEntry* entry = m_model->GetEntries()[translatedIndex.row()];
  vector<const CatalogEntry*> entries;
  entries.push_back(entry);
  m_searchBarModel->SetCatalogEntries(entries);
  m_catalogEntryActivatedSignal();
}

void CatalogTabView::OnDescriptionChanged() {
  QModelIndexList selectedIndexes =
    m_ui->m_catalogEntryList->selectionModel()->selectedIndexes();
  if(selectedIndexes.size() == 1) {
    string entryName;
    QModelIndex translatedIndex = m_proxyModel->mapToSource(
      selectedIndexes.front());
    CatalogEntry* entry = m_model->GetEntries()[translatedIndex.row()];
    if(!entry->IsReadOnly()) {
      entry->SetDescription(
        m_ui->m_descriptionText->toPlainText().toStdString());
      m_modifiedEntries.insert(entry->GetUid());
    }
  }
}

void CatalogTabView::OnSearchBarUpdate(CatalogSearchBarModel::Mode mode,
    const string& text) {
  if(mode == CatalogSearchBarModel::INPUT) {
    m_proxyModel->setFilterWildcard(QString::fromStdString(text));
  }
  m_ui->m_searchBar->setText(QString::fromStdString(text));
}
