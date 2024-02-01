#include "Spire/AccountViewer/AccountViewModel.hpp"
#include <QIcon>
#include "Spire/AccountViewer/AdministratorItem.hpp"
#include "Spire/AccountViewer/DirectoryItem.hpp"
#include "Spire/AccountViewer/ManagerItem.hpp"
#include "Spire/AccountViewer/RootItem.hpp"
#include "Spire/AccountViewer/ServiceItem.hpp"
#include "Spire/AccountViewer/TraderItem.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Spire;
using namespace std;

AccountViewModel::AccountViewModel(Ref<UserProfile> userProfile,
    QObject* parent)
    : QAbstractItemModel(parent),
      m_userProfile(userProfile.Get()) {}

AccountViewModel::~AccountViewModel() {}

void AccountViewModel::Load() {
  m_items.clear();
  m_roots.clear();
  m_children.clear();
  m_parents.clear();
  auto& administrationClient =
    m_userProfile->GetServiceClients().GetAdministrationClient();
  auto& serviceLocatorClient =
    m_userProfile->GetServiceClients().GetServiceLocatorClient();
  if(m_userProfile->IsAdministrator()) {
    auto administratorsRoot = std::make_unique<RootItem>("Administrators");
    m_administratorsRoot = administratorsRoot.get();
    Add(std::move(administratorsRoot));
    auto servicesRoot = std::make_unique<RootItem>("Services");
    m_servicesRoot = servicesRoot.get();
    Add(std::move(servicesRoot));
    auto administrators = administrationClient.LoadAdministrators();
    std::sort(administrators.begin(), administrators.end(),
      &DirectoryEntry::NameComparator);
    for(auto& administrator : administrators) {
      auto administratorItem = std::make_unique<AdministratorItem>(
        administrator);
      Add(std::move(administratorItem), m_administratorsRoot);
    }
    auto services = administrationClient.LoadServices();
    std::sort(services.begin(), services.end(),
      &DirectoryEntry::NameComparator);
    for(auto& service : services) {
      auto serviceItem = std::make_unique<ServiceItem>(service);
      Add(std::move(serviceItem), m_servicesRoot);
    }
  }
  auto tradingGroupsRoot = std::make_unique<RootItem>("Trading Groups");
  m_tradingGroupsRoot = tradingGroupsRoot.get();
  Add(std::move(tradingGroupsRoot));
  auto entries = administrationClient.LoadManagedTradingGroups(
    serviceLocatorClient.GetAccount());
  std::sort(entries.begin(), entries.end(), &DirectoryEntry::NameComparator);
  for(auto& entry : entries) {
    auto group = std::make_unique<DirectoryItem>(entry,
      DirectoryItem::DirectoryType::GROUP);
    Add(std::move(group), m_tradingGroupsRoot);
  }
}

const vector<RootItem*>& AccountViewModel::GetRoots() const {
  return m_roots;
}

AccountViewItem* AccountViewModel::GetItem(const QModelIndex& index) const {
  if(!index.isValid()) {
    return nullptr;
  }
  auto item = static_cast<AccountViewItem*>(index.internalPointer());
  return item;
}

QModelIndex AccountViewModel::GetIndex(const AccountViewItem& item) const {
  if(item.GetType() == AccountViewItem::Type::ROOT) {
    auto rootsIterator = std::find(m_roots.begin(), m_roots.end(), &item);
    if(rootsIterator != m_roots.end()) {
      return createIndex(distance(m_roots.begin(), rootsIterator), 0,
        *rootsIterator);
    }
    return QModelIndex();
  }
  auto parent = m_parents.at(&const_cast<AccountViewItem&>(item));
  auto& children = m_children.at(parent);
  auto indexIterator = std::find(children.begin(), children.end(), &item);
  return createIndex(std::distance(children.begin(), indexIterator), 0,
    *indexIterator);
}

void AccountViewModel::Expand(const QModelIndex& index) {
  auto item = GetItem(index);
  auto childrenIterator = m_children.find(item);
  if(childrenIterator == m_children.end()) {
    return;
  }
  auto& children = childrenIterator->second;
  for(auto child : children) {
    auto grandChildrenIterator = m_children.find(child);
    if(grandChildrenIterator == m_children.end()) {
      auto grandChildren = child->LoadChildren(*m_userProfile);
      if(grandChildren.empty()) {
        m_children.insert(std::make_pair(child, vector<AccountViewItem*>()));
      } else {
        for(auto& grandChild : grandChildren) {
          Add(std::move(grandChild), child);
        }
      }
    }
  }
}

Qt::ItemFlags AccountViewModel::flags(const QModelIndex& index) const {
  auto flags = QAbstractItemModel::flags(index);
  if(!index.isValid()) {
    return flags;
  }
  flags &= ~Qt::ItemNeverHasChildren;
  return flags;
}

QModelIndex AccountViewModel::index(int row, int column,
    const QModelIndex& parent) const {
  if(!parent.isValid()) {
    if(column == 0) {
      if(row >= 0 && row < static_cast<int>(m_roots.size())) {
        return createIndex(row, 0, m_roots[row]);
      }
    }
    return QModelIndex();
  }
  if(column != 0) {
    return QModelIndex();
  }
  auto parentItem = GetItem(parent);
  auto childrenIterator = m_children.find(parentItem);
  if(childrenIterator == m_children.end() ||
      row >= static_cast<int>(childrenIterator->second.size())) {
    return QModelIndex();
  }
  auto child = childrenIterator->second[row];
  return createIndex(row, 0, child);
}

QModelIndex AccountViewModel::parent(const QModelIndex& index) const {
  if(!index.isValid()) {
    return QModelIndex();
  }
  auto entry = GetItem(index);
  if(entry == nullptr) {
    return QModelIndex();
  }
  if(entry->GetType() == AccountViewItem::Type::ROOT) {
    return QModelIndex();
  }
  auto parent = m_parents.at(entry);
  return GetIndex(*parent);
}

int AccountViewModel::rowCount(const QModelIndex& parent) const {
  if(!parent.isValid()) {
    return m_roots.size();
  }
  auto item = GetItem(parent);
  auto childrenIterator = m_children.find(item);
  if(childrenIterator == m_children.end()) {
    return 0;
  }
  return childrenIterator->second.size();
}

int AccountViewModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant AccountViewModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  if(role == Qt::DisplayRole) {
    auto item = GetItem(index);
    return QString::fromStdString(item->GetName());
  } else if(role == Qt::DecorationRole) {
    auto item = GetItem(index);
    return item->GetIcon();
  }
  return QVariant();
}

QVariant AccountViewModel::headerData(int section, Qt::Orientation orientation,
    int role) const {
  if(role == Qt::DisplayRole) {
    return tr("Listings");
  }
  return QVariant();
}

void AccountViewModel::Add(unique_ptr<RootItem> root) {
  m_roots.push_back(root.get());
  m_children.insert(std::make_pair(root.get(), vector<AccountViewItem*>()));
  m_items.push_back(std::move(root));
}

void AccountViewModel::Add(unique_ptr<AccountViewItem> item,
    AccountViewItem* parent) {
  auto& children = m_children[parent];
  for(auto child : children) {
    if(child == item.get()) {
      return;
    }
  }
  auto parentIndex = GetIndex(*parent);
  beginInsertRows(parentIndex, children.size(), children.size());
  children.push_back(item.get());
  m_parents[item.get()] = parent;
  m_items.push_back(std::move(item));
  endInsertRows();
}
