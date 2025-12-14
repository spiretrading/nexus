#include "Spire/Utilities/SecurityInfoModel.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include <QTimer>
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

SecurityInfoModel::SecurityInfoModel(Ref<UserProfile> userProfile)
  : m_userProfile(userProfile.get()) {}

void SecurityInfoModel::Search(const std::string& prefix) {
  if(prefix.empty()) {
    return;
  }
  auto uppercasePrefix = to_upper_copy(prefix);
  if(!m_prefixes.insert(uppercasePrefix).second) {
    return;
  }
  m_queryPromise = QtPromise([=] {
    return m_userProfile->GetClients().
      get_market_data_client().load_security_info_from_prefix(uppercasePrefix);
  }, LaunchPolicy::ASYNC).then(
    [=] (const std::vector<SecurityInfo>& securityInfoItems) {
      QTimer::singleShot(0, this, [=] {
        AddSecurityInfoItems(securityInfoItems);
      });
    });
}

int SecurityInfoModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_securityInfoItems.size());
}

int SecurityInfoModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant SecurityInfoModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& item = m_securityInfoItems[index.row()];
  if(role == Qt::DisplayRole) {
    if(index.column() == SECURITY_COLUMN) {
      return QString::fromStdString(to_string(item.m_security));
    } else if(index.column() == NAME_COLUMN) {
      return QString::fromStdString(item.m_name);
    } else if(index.column() == SECTOR_COLUMN) {
      return QString::fromStdString(item.m_sector);
    }
  }
  return QVariant();
}

QVariant SecurityInfoModel::headerData(int section, Qt::Orientation orientation,
    int role) const {
  if(role == Qt::DisplayRole) {
    if(section == SECURITY_COLUMN) {
      return tr("Security");
    } else if(section == NAME_COLUMN) {
      return tr("Name");
    } else if(section == SECTOR_COLUMN) {
      return tr("Sector");
    }
  }
  return QVariant();
}

void SecurityInfoModel::AddSecurityInfoItems(
    std::vector<SecurityInfo> securityInfoItems) {
  auto i = securityInfoItems.begin();
  while(i != securityInfoItems.end()) {
    auto searchIterator = find_if(m_securityInfoItems.begin(),
      m_securityInfoItems.end(), [&] (const auto& securityInfo) {
        return securityInfo.m_security == i->m_security;
      });
    if(searchIterator != m_securityInfoItems.end()) {
      i = securityInfoItems.erase(i);
    } else {
      ++i;
    }
  }
  if(securityInfoItems.empty()) {
    return;
  }
  beginInsertRows(QModelIndex(), static_cast<int>(m_securityInfoItems.size()),
    static_cast<int>(m_securityInfoItems.size()) +
    securityInfoItems.size() - 1);
  m_securityInfoItems.insert(m_securityInfoItems.end(),
    securityInfoItems.begin(), securityInfoItems.end());
  endInsertRows();
}
