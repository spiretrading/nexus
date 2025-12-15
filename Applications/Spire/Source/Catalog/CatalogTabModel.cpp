#include "Spire/Catalog/CatalogTabModel.hpp"
#include <QMimeData>
#include "Spire/Catalog/BuiltInCatalogEntry.hpp"
#include "Spire/Catalog/CatalogEntry.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::LegacyUI;

const std::string& CatalogTabModel::GetName() const {
  return m_name;
}

void CatalogTabModel::SetName(const std::string& name) {
  if(name == m_name) {
    return;
  }
  m_name = name;
  m_nameSignal();
}

const std::vector<CatalogEntry*>& CatalogTabModel::GetEntries() const {
  return m_entries;
}

void CatalogTabModel::Add(Ref<CatalogEntry> entry) {
  auto selfEntry = entry.get();
  if(std::find(m_entries.begin(), m_entries.end(), selfEntry) !=
      m_entries.end()) {
    return;
  }
  auto index = static_cast<int>(m_entries.size());
  beginInsertRows(QModelIndex(), index, index);
  m_entries.push_back(selfEntry);
  endInsertRows();
}

void CatalogTabModel::Remove(const CatalogEntry& entry) {
  auto entryIterator = std::find(m_entries.begin(), m_entries.end(), &entry);
  if(entryIterator == m_entries.end()) {
    return;
  }
  auto index =
    static_cast<int>(std::distance(m_entries.begin(), entryIterator));
  beginRemoveRows(QModelIndex(), index, index);
  m_entries.erase(entryIterator);
  endRemoveRows();
}

connection CatalogTabModel::ConnectNameSignal(
    const NameSignal::slot_type& slot) const {
  return m_nameSignal.connect(slot);
}

Qt::DropActions CatalogTabModel::supportedDropActions() const {
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags CatalogTabModel::flags(const QModelIndex& index) const {
  auto defaultFlags = QAbstractListModel::flags(index);
  if(index.isValid()) {
    return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  } else {
    return defaultFlags | Qt::ItemIsDropEnabled;
  }
}

int CatalogTabModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_entries.size());
}

QVariant CatalogTabModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto entry = m_entries[index.row()];
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  } else if(role == Qt::DecorationRole) {
    return entry->GetIcon();
  } else if(role == Qt::DisplayRole) {
    return QString::fromStdString(entry->GetName());
  }
  return QVariant();
}

QVariant CatalogTabModel::headerData(
    int section, Qt::Orientation orientation, int role) const {
  return QVariant();
}

QStringList CatalogTabModel::mimeTypes() const {
  auto types = QStringList();
  types << QString::fromStdString(CatalogEntry::MIME_TYPE);
  return types;
}

QMimeData* CatalogTabModel::mimeData(const QModelIndexList& indexes) const {
  auto entries = std::vector<CatalogEntry*>();
  for(auto& index : indexes) {
    if(index.isValid()) {
      entries.push_back(m_entries[index.row()]);
    }
  }
  return CatalogEntry::EncodeAsMimeData(entries);
}

bool CatalogTabModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
    int row, int column, const QModelIndex& parent) {
  return false;
}
