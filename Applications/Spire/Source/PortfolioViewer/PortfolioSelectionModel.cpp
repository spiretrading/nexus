#include "Spire/PortfolioViewer/PortfolioSelectionModel.hpp"
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/VariantLambdaVisitor.hpp>
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

PortfolioSelectionModel::PortfolioSelectionModel(
    Ref<UserProfile> userProfile,
    const PortfolioViewerProperties& properties, QObject* parent)
    : QAbstractItemModel(parent),
      m_userProfile(userProfile.Get()) {
  for(size_t i = 0; i < SELECTION_TYPES_COUNT; ++i) {
    m_roots[i] = createIndex(i, 0, -1);
  }
  m_groups = m_userProfile->GetServiceClients().
    GetAdministrationClient().LoadManagedTradingGroups(
    m_userProfile->GetServiceClients().GetServiceLocatorClient().GetAccount());
  std::sort(m_groups.begin(), m_groups.end(), &DirectoryEntry::NameComparator);
  if(properties.IsSelectingAllGroups()) {
    for(auto i = m_groups.begin(); i != m_groups.end(); ++i) {
      m_selectedGroups.insert(*i);
    }
  } else {
    m_selectedGroups = properties.GetSelectedGroups();
  }
  if(properties.IsSelectingAllCurrencies()) {
    for(auto i = m_userProfile->GetCurrencyDatabase().GetEntries().begin();
        i != m_userProfile->GetCurrencyDatabase().GetEntries().end(); ++i) {
      m_selectedCurrencies.insert(i->m_id);
    }
  } else {
    m_selectedCurrencies = properties.GetSelectedCurrencies();
  }
  if(properties.IsSelectingAllMarkets()) {
    for(auto i = m_userProfile->GetMarketDatabase().GetEntries().begin();
        i != m_userProfile->GetMarketDatabase().GetEntries().end(); ++i) {
      m_selectedMarkets.insert(i->m_code);
    }
  } else {
    m_selectedMarkets = properties.GetSelectedMarkets();
  }
  m_selectedSides = properties.GetSelectedSides();
}

PortfolioSelectionModel::~PortfolioSelectionModel() {}

void PortfolioSelectionModel::UpdateProperties(
    Out<PortfolioViewerProperties> properties) const {
  properties->GetSelectedGroups() = m_selectedGroups;
  if(m_selectedGroups.size() == m_groups.size()) {
    properties->SetSelectingAllGroups(true);
  } else {
    properties->SetSelectingAllGroups(false);
  }
  properties->GetSelectedCurrencies() = m_selectedCurrencies;
  if(m_selectedCurrencies.size() ==
      m_userProfile->GetCurrencyDatabase().GetEntries().size()) {
    properties->SetSelectingAllCurrencies(true);
  } else {
    properties->SetSelectingAllCurrencies(false);
  }
  properties->GetSelectedMarkets() = m_selectedMarkets;
  if(m_selectedMarkets.size() ==
      m_userProfile->GetMarketDatabase().GetEntries().size()) {
    properties->SetSelectingAllMarkets(true);
  } else {
    properties->SetSelectingAllMarkets(false);
  }
  properties->GetSelectedSides() = m_selectedSides;
}

bool PortfolioSelectionModel::TestSelectionAcceptsEntry(
    const PortfolioViewerModel::Entry& entry) const {
  if(m_selectedGroups.find(entry.m_group) == m_selectedGroups.end()) {
    return false;
  }
  if(m_selectedCurrencies.find(entry.m_inventory.m_position.m_key.m_currency) ==
      m_selectedCurrencies.end()) {
    return false;
  }
  if(m_selectedMarkets.find(
      entry.m_inventory.m_position.m_key.m_index.GetMarket()) ==
      m_selectedMarkets.end()) {
    return false;
  }
  if(m_selectedSides.find(GetSide(entry.m_inventory.m_position)) ==
      m_selectedSides.end()) {
    return false;
  }
  return true;
}

Qt::ItemFlags PortfolioSelectionModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);
  if(index.isValid()) {
    flags |= Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    flags &= ~Qt::ItemNeverHasChildren;
  }
  return flags;
}

QModelIndex PortfolioSelectionModel::index(int row, int column,
    const QModelIndex& parent) const {
  if(!parent.isValid()) {
    if(column == 0 && row >= 0 && row < SELECTION_TYPES_COUNT) {
      return m_roots[row];
    }
    return QModelIndex();
  } else if(parent == m_roots[GROUP_SELECTION]) {
    if(column == 0 && row >= 0 && row <= static_cast<int>(m_groups.size())) {
      return createIndex(row, 0, GROUP_SELECTION);
    }
  } else if(parent == m_roots[CURRENCY_SELECTION]) {
    if(column == 0 && row >= 0 && row <= static_cast<int>(
        m_userProfile->GetCurrencyDatabase().GetEntries().size())) {
      return createIndex(row, 0, CURRENCY_SELECTION);
    }
  } else if(parent == m_roots[MARKET_SELECTION]) {
    if(column == 0 && row >= 0 && row <= static_cast<int>(
        m_userProfile->GetMarketDatabase().GetEntries().size())) {
      return createIndex(row, 0, MARKET_SELECTION);
    }
  } else if(parent == m_roots[SIDE_SELECTION]) {
    if(column == 0 && row >= 0 && row <= Side::COUNT) {
      return createIndex(row, 0, SIDE_SELECTION);
    }
  }
  return QModelIndex();
}

QModelIndex PortfolioSelectionModel::parent(const QModelIndex& index) const {
  if(index.internalId() == -1) {
    return QModelIndex();
  }
  return m_roots[index.internalId()];
}

int PortfolioSelectionModel::rowCount(const QModelIndex& parent) const {
  if(!parent.isValid()) {
    return SELECTION_TYPES_COUNT;
  }
  if(parent == m_roots[GROUP_SELECTION]) {
    return static_cast<int>(m_groups.size());
  } else if(parent == m_roots[CURRENCY_SELECTION]) {
    return static_cast<int>(
      m_userProfile->GetCurrencyDatabase().GetEntries().size());
  } else if(parent == m_roots[MARKET_SELECTION]) {
    return static_cast<int>(
      m_userProfile->GetMarketDatabase().GetEntries().size());
  } else if(parent == m_roots[SIDE_SELECTION]) {
    return Side::COUNT + 1;
  }
  return 0;
}

int PortfolioSelectionModel::columnCount(const QModelIndex& parent) const {
  if(!parent.isValid()) {
    return 1;
  }
  return 1;
}

QVariant PortfolioSelectionModel::data(const QModelIndex& index,
    int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  if(role == Qt::CheckStateRole && index.column() == 0) {
    if(index == m_roots[GROUP_SELECTION]) {
      if(m_selectedGroups.size() == m_groups.size()) {
        return Qt::Checked;
      } else {
        return Qt::Unchecked;
      }
    } else if(index == m_roots[CURRENCY_SELECTION]) {
      const CurrencyDatabase& currencies = m_userProfile->GetCurrencyDatabase();
      if(m_selectedCurrencies.size() == currencies.GetEntries().size()) {
        return Qt::Checked;
      } else {
        return Qt::Unchecked;
      }
    } else if(index == m_roots[MARKET_SELECTION]) {
      const MarketDatabase& markets = m_userProfile->GetMarketDatabase();
      if(m_selectedMarkets.size() == markets.GetEntries().size()) {
        return Qt::Checked;
      } else {
        return Qt::Unchecked;
      }
    } else if(index == m_roots[SIDE_SELECTION]) {
      if(m_selectedSides.size() == Side::COUNT + 1) {
        return Qt::Checked;
      } else {
        return Qt::Unchecked;
      }
    } else {
      boost::optional<SelectionVariant> selection = Find(index);
      if(selection.is_initialized()) {
        return ApplyVariantLambdaVisitor<QVariant>(*selection,
          [&] (const DirectoryEntry& group) -> QVariant {
            if(m_selectedGroups.find(group) != m_selectedGroups.end()) {
              return Qt::Checked;
            } else {
              return Qt::Unchecked;
            }
          },
          [&] (const CurrencyDatabase::Entry& currency) -> QVariant {
            if(m_selectedCurrencies.find(currency.m_id) !=
                m_selectedCurrencies.end()) {
              return Qt::Checked;
            } else {
              return Qt::Unchecked;
            }
          },
          [&] (const MarketDatabase::Entry& market) -> QVariant {
            if(m_selectedMarkets.find(market.m_code) !=
                m_selectedMarkets.end()) {
              return Qt::Checked;
            } else {
              return Qt::Unchecked;
            }
          },
          [&] (Side side) -> QVariant {
            if(m_selectedSides.find(side) != m_selectedSides.end()) {
              return Qt::Checked;
            } else {
              return Qt::Unchecked;
            }
          });
      }
    }
  } else if(role == Qt::DisplayRole) {
    if(index == m_roots[GROUP_SELECTION]) {
      return tr("Groups");
    } else if(index == m_roots[CURRENCY_SELECTION]) {
      return tr("Currency");
    } else if(index == m_roots[MARKET_SELECTION]) {
      return tr("Market");
    } else if(index == m_roots[SIDE_SELECTION]) {
      return tr("Side");
    } else {
      auto selection = Find(index);
      if(selection.is_initialized()) {
        return ApplyVariantLambdaVisitor<QVariant>(*selection,
          [] (const DirectoryEntry& group) {
            return QString::fromStdString(group.m_name);
          },
          [] (const CurrencyDatabase::Entry& currency) {
            return QVariant::fromValue(currency.m_id);
          },
          [] (const MarketDatabase::Entry& market) {
            return QString::fromStdString(market.m_displayName);
          },
          [] (Side side) {
            return QVariant::fromValue(PositionSideToken(side));
          });
      }
    }
  }
  return QVariant();
}

bool PortfolioSelectionModel::setData(const QModelIndex& index,
    const QVariant& value, int role) {
  if(role == Qt::CheckStateRole && index.column() == 0) {
    Qt::CheckState state = static_cast<Qt::CheckState>(value.toUInt());
    if(index == m_roots[GROUP_SELECTION]) {
      if(state == Qt::Checked) {
        for(auto i = m_groups.begin(); i != m_groups.end(); ++i) {
          m_selectedGroups.insert(*i);
        }
      } else {
        m_selectedGroups.clear();
      }
      dataChanged(this->index(0, 0, m_roots[GROUP_SELECTION]), this->index(
        m_groups.size() - 1, 0, m_roots[GROUP_SELECTION]));
      return true;
    } else if(index == m_roots[CURRENCY_SELECTION]) {
      vector<CurrencyDatabase::Entry> currencies =
        m_userProfile->GetCurrencyDatabase().GetEntries();
      if(state == Qt::Checked) {
        for(auto i = currencies.begin(); i != currencies.end(); ++i) {
          m_selectedCurrencies.insert(i->m_id);
        }
      } else {
        m_selectedCurrencies.clear();
      }
      dataChanged(this->index(0, 0, m_roots[CURRENCY_SELECTION]), this->index(
        m_userProfile->GetCurrencyDatabase().GetEntries().size() - 1, 0,
        m_roots[CURRENCY_SELECTION]));
      return true;
    } else if(index == m_roots[MARKET_SELECTION]) {
      vector<MarketDatabase::Entry> markets =
        m_userProfile->GetMarketDatabase().GetEntries();
      if(state == Qt::Checked) {
        for(auto i = markets.begin(); i != markets.end(); ++i) {
          m_selectedMarkets.insert(i->m_code);
        }
      } else {
        m_selectedMarkets.clear();
      }
      dataChanged(this->index(0, 0, m_roots[MARKET_SELECTION]), this->index(
        m_userProfile->GetMarketDatabase().GetEntries().size() - 1, 0,
        m_roots[MARKET_SELECTION]));
      return true;
    } else if(index == m_roots[SIDE_SELECTION]) {
      if(state == Qt::Checked) {
        m_selectedSides.insert(Side::NONE);
        m_selectedSides.insert(Side::ASK);
        m_selectedSides.insert(Side::BID);
      } else {
        m_selectedSides.clear();
      }
      dataChanged(this->index(0, 0, m_roots[SIDE_SELECTION]), this->index(
        Side::COUNT + 1, 0, m_roots[SIDE_SELECTION]));
      return true;
    } else {
      auto selection = Find(index);
      if(selection.is_initialized()) {
        ApplyVariantLambdaVisitor<void>(*selection,
          [&] (const DirectoryEntry& group) {
            if(state == Qt::Checked) {
              m_selectedGroups.insert(group);
            } else {
              m_selectedGroups.erase(group);
            }
          },
          [&] (const CurrencyDatabase::Entry& currency) {
            if(state == Qt::Checked) {
              m_selectedCurrencies.insert(currency.m_id);
            } else {
              m_selectedCurrencies.erase(currency.m_id);
            }
          },
          [&] (const MarketDatabase::Entry& market) {
            if(state == Qt::Checked) {
              m_selectedMarkets.insert(market.m_code);
            } else {
              m_selectedMarkets.erase(market.m_code);
            }
          },
          [&] (Side side) {
            if(state == Qt::Checked) {
              m_selectedSides.insert(side);
            } else {
              m_selectedSides.erase(side);
            }
          });
        Q_EMIT dataChanged(index.parent(), index.parent());
        Q_EMIT dataChanged(index, index);
        return true;
      }
    }
  }
  return QAbstractItemModel::setData(index, value, role);
}

QVariant PortfolioSelectionModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role == Qt::DisplayRole) {
    return tr("Selections");
  }
  return QVariant();
}

boost::optional<PortfolioSelectionModel::SelectionVariant>
    PortfolioSelectionModel::Find(const QModelIndex& index) const {
  if(index.parent() == m_roots[GROUP_SELECTION]) {
    if(index.column() == 0 && index.row() >= 0 && index.row() <
        static_cast<int>(m_groups.size())) {
      return SelectionVariant{m_groups[index.row()]};
    }
  } else if(index.parent() == m_roots[CURRENCY_SELECTION]) {
    const CurrencyDatabase& currencies = m_userProfile->GetCurrencyDatabase();
    if(index.column() == 0 && index.row() >= 0 && index.row() <
        static_cast<int>(currencies.GetEntries().size())) {
      return SelectionVariant{currencies.GetEntries()[index.row()]};
    }
  } else if(index.parent() == m_roots[MARKET_SELECTION]) {
    const MarketDatabase& markets = m_userProfile->GetMarketDatabase();
    if(index.column() == 0 && index.row() >= 0 && index.row() <
        static_cast<int>(markets.GetEntries().size())) {
      return SelectionVariant{markets.GetEntries()[index.row()]};
    }
  } else if(index.parent() == m_roots[SIDE_SELECTION]) {
    if(index.column() == 0 && index.row() >= 0 && index.row() <=
        Side::COUNT + 1) {
      if(index.row() == 0) {
        return SelectionVariant{Side::ASK};
      } else if(index.row() == 1) {
        return SelectionVariant{Side::BID};
      } else {
        return SelectionVariant{Side::NONE};
      }
    }
  }
  return none;
}
