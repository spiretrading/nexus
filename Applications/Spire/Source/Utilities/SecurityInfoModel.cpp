#include "Spire/Utilities/SecurityInfoModel.hpp"
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/Sync.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::Threading;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  int UPDATE_INTERVAL = 50;
}

SecurityInfoModel::SecurityInfoModel(Ref<UserProfile> userProfile)
    : m_userProfile(userProfile.Get()),
      m_isAliveFlag(std::make_shared<Sync<bool>>(true)) {
  connect(&m_updateTimer, &QTimer::timeout, this,
    &SecurityInfoModel::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
}

SecurityInfoModel::~SecurityInfoModel() {
  *m_isAliveFlag = false;
}

void SecurityInfoModel::Search(const string& prefix) {
  if(prefix.empty()) {
    return;
  }
  string uppercasePrefix = to_upper_copy(prefix);
  if(!m_prefixes.insert(uppercasePrefix).second) {
    return;
  }
  auto selfUserProfile = m_userProfile;
  auto isAliveFlag = m_isAliveFlag;
  Spawn(
    [=] {
      vector<SecurityInfo> securityInfoItems =
        selfUserProfile->GetServiceClients().GetMarketDataClient().
        LoadSecurityInfoFromPrefix(uppercasePrefix);
      With(*isAliveFlag,
        [&] (bool isAliveFlag) {
          if(!isAliveFlag) {
            return;
          }
          m_slotHandler.Push(
            [=] {
              this->AddSecurityInfoItems(securityInfoItems);
            });
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
  const SecurityInfo& item = m_securityInfoItems[index.row()];
  if(role == Qt::DisplayRole) {
    if(index.column() == SECURITY_COLUMN) {
      return QString::fromStdString(ToString(item.m_security,
        m_userProfile->GetMarketDatabase()));
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
    vector<SecurityInfo> securityInfoItems) {
  auto i = securityInfoItems.begin();
  while(i != securityInfoItems.end()) {
    auto searchIterator = find_if(m_securityInfoItems.begin(),
      m_securityInfoItems.end(),
      [&] (const SecurityInfo& securityInfo) {
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

void SecurityInfoModel::OnUpdateTimer() {
  HandleTasks(m_slotHandler);
}
