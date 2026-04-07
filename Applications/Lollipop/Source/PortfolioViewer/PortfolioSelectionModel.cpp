#include "Spire/PortfolioViewer/PortfolioSelectionModel.hpp"
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/VariantLambdaVisitor.hpp>
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

PortfolioSelectionModel::PortfolioSelectionModel(
    Ref<UserProfile> userProfile,
    const PortfolioViewerProperties& properties, QObject* parent)
    : QAbstractItemModel(parent),
      m_userProfile(userProfile.get()) {
  for(size_t i = 0; i < SELECTION_TYPES_COUNT; ++i) {
    m_roots[i] = createIndex(i, 0, -1);
  }
  m_groups = m_userProfile->GetClients().get_administration_client().
    load_managed_trading_groups(
      m_userProfile->GetClients().get_service_locator_client().get_account());
  std::sort(m_groups.begin(), m_groups.end(), &DirectoryEntry::name_comparator);
  if(properties.IsSelectingAllGroups()) {
    for(auto i = m_groups.begin(); i != m_groups.end(); ++i) {
      m_selectedGroups.insert(*i);
    }
  } else {
    m_selectedGroups = properties.GetSelectedGroups();
  }
  if(properties.IsSelectingAllCurrencies()) {
    for(auto i = m_userProfile->GetCurrencyDatabase().get_entries().begin();
        i != m_userProfile->GetCurrencyDatabase().get_entries().end(); ++i) {
      m_selectedCurrencies.insert(i->m_id);
    }
  } else {
    m_selectedCurrencies = properties.GetSelectedCurrencies();
  }
  if(properties.IsSelectingAllVenues()) {
    for(auto i = m_userProfile->GetVenueDatabase().get_entries().begin();
        i != m_userProfile->GetVenueDatabase().get_entries().end(); ++i) {
      m_selectedVenues.insert(i->m_venue);
    }
  } else {
    m_selectedVenues = properties.GetSelectedVenues();
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
      m_userProfile->GetCurrencyDatabase().get_entries().size()) {
    properties->SetSelectingAllCurrencies(true);
  } else {
    properties->SetSelectingAllCurrencies(false);
  }
  properties->GetSelectedVenues() = m_selectedVenues;
  if(m_selectedVenues.size() ==
      m_userProfile->GetVenueDatabase().get_entries().size()) {
    properties->SetSelectingAllVenues(true);
  } else {
    properties->SetSelectingAllVenues(false);
  }
  properties->GetSelectedSides() = m_selectedSides;
}

bool PortfolioSelectionModel::TestSelectionAcceptsEntry(
    const PortfolioViewerModel::Entry& entry) const {
  if(m_selectedGroups.find(entry.m_group) == m_selectedGroups.end()) {
    return false;
  }
  if(m_selectedCurrencies.find(entry.m_inventory.m_position.m_currency) ==
      m_selectedCurrencies.end()) {
    return false;
  }
  if(m_selectedVenues.find(
      entry.m_inventory.m_position.m_security.get_venue()) ==
      m_selectedVenues.end()) {
    return false;
  }
  if(m_selectedSides.find(get_side(entry.m_inventory.m_position)) ==
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
        m_userProfile->GetCurrencyDatabase().get_entries().size())) {
      return createIndex(row, 0, CURRENCY_SELECTION);
    }
  } else if(parent == m_roots[VENUE_SELECTION]) {
    if(column == 0 && row >= 0 && row <= static_cast<int>(
        m_userProfile->GetVenueDatabase().get_entries().size())) {
      return createIndex(row, 0, VENUE_SELECTION);
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
      m_userProfile->GetCurrencyDatabase().get_entries().size());
  } else if(parent == m_roots[VENUE_SELECTION]) {
    return static_cast<int>(
      m_userProfile->GetVenueDatabase().get_entries().size());
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
      if(m_selectedCurrencies.size() == currencies.get_entries().size()) {
        return Qt::Checked;
      } else {
        return Qt::Unchecked;
      }
    } else if(index == m_roots[VENUE_SELECTION]) {
      auto& venues = m_userProfile->GetVenueDatabase();
      if(m_selectedVenues.size() == venues.get_entries().size()) {
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
        return apply_variant_lambda_visitor(*selection,
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
          [&] (const VenueDatabase::Entry& venue) -> QVariant {
            if(m_selectedVenues.find(venue.m_venue) != m_selectedVenues.end()) {
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
    } else if(index == m_roots[VENUE_SELECTION]) {
      return tr("Venue");
    } else if(index == m_roots[SIDE_SELECTION]) {
      return tr("Side");
    } else {
      auto selection = Find(index);
      if(selection.is_initialized()) {
        return apply_variant_lambda_visitor(*selection,
          [] (const DirectoryEntry& group) {
            return QVariant(QString::fromStdString(group.m_name));
          },
          [] (const CurrencyDatabase::Entry& currency) {
            return QVariant::fromValue(currency.m_id);
          },
          [] (const VenueDatabase::Entry& venue) {
            return QVariant(QString::fromStdString(venue.m_display_name));
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
      auto currencies = m_userProfile->GetCurrencyDatabase().get_entries();
      if(state == Qt::Checked) {
        for(auto i = currencies.begin(); i != currencies.end(); ++i) {
          m_selectedCurrencies.insert(i->m_id);
        }
      } else {
        m_selectedCurrencies.clear();
      }
      dataChanged(this->index(0, 0, m_roots[CURRENCY_SELECTION]), this->index(
        currencies.size() - 1, 0,
        m_roots[CURRENCY_SELECTION]));
      return true;
    } else if(index == m_roots[VENUE_SELECTION]) {
      auto venues = m_userProfile->GetVenueDatabase().get_entries();
      if(state == Qt::Checked) {
        for(auto i = venues.begin(); i != venues.end(); ++i) {
          m_selectedVenues.insert(i->m_venue);
        }
      } else {
        m_selectedVenues.clear();
      }
      dataChanged(this->index(0, 0, m_roots[VENUE_SELECTION]), this->index(
        venues.size() - 1, 0, m_roots[VENUE_SELECTION]));
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
        apply_variant_lambda_visitor(*selection,
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
          [&] (const VenueDatabase::Entry& venue) {
            if(state == Qt::Checked) {
              m_selectedVenues.insert(venue.m_venue);
            } else {
              m_selectedVenues.erase(venue.m_venue);
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
        static_cast<int>(currencies.get_entries().size())) {
      return SelectionVariant{currencies.get_entries()[index.row()]};
    }
  } else if(index.parent() == m_roots[VENUE_SELECTION]) {
    const auto& venues = m_userProfile->GetVenueDatabase();
    if(index.column() == 0 && index.row() >= 0 && index.row() <
        static_cast<int>(venues.get_entries().size())) {
      return SelectionVariant{venues.get_entries()[index.row()]};
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
