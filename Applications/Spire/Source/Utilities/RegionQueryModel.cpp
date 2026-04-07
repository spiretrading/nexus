#include "Spire/Utilities/RegionQueryModel.hpp"
#include <QTimer>
#include <boost/algorithm/string/case_conv.hpp>
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

RegionQueryModel::RegionQueryModel(Ref<UserProfile> userProfile)
    : m_userProfile(userProfile.get()) {
  auto items = std::vector<Item>();
  items.push_back(Region::GLOBAL);
  for(auto& country : DEFAULT_COUNTRIES.get_entries()) {
    items.push_back(country.m_code);
  }
  for(auto& venue : DEFAULT_VENUES.get_entries()) {
    items.push_back(venue.m_venue);
  }
  Add(std::move(items));
}

void RegionQueryModel::Search(const std::string& prefix) {
  if(prefix.empty()) {
    return;
  }
  auto uppercasePrefix = to_upper_copy(prefix);
  if(!m_prefixes.insert(uppercasePrefix).second) {
    return;
  }
  m_queryPromise = QtPromise([=] {
    return m_userProfile->GetClients().get_market_data_client().
      load_security_info_from_prefix(uppercasePrefix);
  }, LaunchPolicy::ASYNC).then(
    [=] (const std::vector<SecurityInfo>& info) {
      QTimer::singleShot(0, this, [=] {
        auto items = std::vector<Item>();
        for(auto& i : info) {
          items.push_back(i);
        }
        Add(std::move(items));
      });
    });
}

int RegionQueryModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_items.size());
}

int RegionQueryModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant RegionQueryModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& item = m_items[index.row()];
  auto column = static_cast<Column>(index.column());
  if(role == Qt::DisplayRole) {
    if(column == Column::REGION) {
      if(auto country = get<CountryCode>(&item)) {
        return QString::fromStdString(DEFAULT_COUNTRIES.from(
          *country).m_three_letter_code.get_data());
      } else if(auto venue = get<Venue>(&item)) {
        return QString::fromStdString(
          DEFAULT_VENUES.from(*venue).m_display_name);
      } else if(auto info = get<SecurityInfo>(&item)) {
        return displayText(info->m_security);
      } else if(auto region = get<Region>(&item)) {
        return displayText(*region);
      }
      return QVariant();
    } else if(column == Column::NAME) {
      if(auto country = get<CountryCode>(&item)) {
        return QString::fromStdString(
          DEFAULT_COUNTRIES.from(*country).m_name);
      } else if(auto venue = get<Venue>(&item)) {
        return QString::fromStdString(
          DEFAULT_VENUES.from(*venue).m_display_name);
      } else if(auto info = get<SecurityInfo>(&item)) {
        return QString::fromStdString(info->m_name);
      } else if(auto region = get<Region>(&item)) {
        if(region->is_global()) {
          return tr("Global");
        }
        return displayText(*region);
      }
      return QVariant();
    } else if(column == Column::DESCRIPTION) {
      if(auto country = get<CountryCode>(&item)) {
        return QString();
      } else if(auto venue = get<Venue>(&item)) {
        return QString::fromStdString(
          DEFAULT_VENUES.from(*venue).m_description);
      } else if(auto info = get<SecurityInfo>(&item)) {
        return QString::fromStdString(info->m_sector);
      } else if(auto region = get<Region>(&item)) {
        if(region->is_global()) {
          return tr("Applies to everything.");
        }
      }
      return QVariant();
    }
  }
  return QVariant();
}

QVariant RegionQueryModel::headerData(
    int section, Qt::Orientation orientation, int role) const {
  if(role == Qt::DisplayRole) {
    auto column = static_cast<Column>(section);
    if(column == Column::REGION) {
      return tr("Region");
    } else if(column == Column::NAME) {
      return tr("Name");
    } else if(column == Column::DESCRIPTION) {
      return tr("Description");
    }
  }
  return QVariant();
}

void RegionQueryModel::Add(std::vector<Item> items) {
  auto i = items.begin();
  while(i != items.end()) {
    auto searchIterator =
      std::find_if(m_items.begin(), m_items.end(), [&] (const auto& item) {
        if(item.which() != i->which()) {
          return false;
        }
        if(auto country = get<CountryCode>(&item)) {
          return *country == get<CountryCode>(*i);
        } else if(auto venue = get<Venue>(&item)) {
          return *venue == get<Venue>(*i);
        } else if(auto info = get<SecurityInfo>(&item)) {
          return info->m_security == get<SecurityInfo>(*i).m_security;
        } else if(auto region = get<Region>(&item)) {
          return *region == get<Region>(*i);
        }
        return false;
      });
    if(searchIterator != m_items.end()) {
      i = items.erase(i);
    } else {
      ++i;
    }
  }
  if(items.empty()) {
    return;
  }
  beginInsertRows(QModelIndex(), static_cast<int>(m_items.size()),
    static_cast<int>(m_items.size()) + items.size() - 1);
  m_items.insert(m_items.end(), items.begin(), items.end());
  endInsertRows();
}
