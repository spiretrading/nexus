#include "Spire/Ui/LocalComboBoxQueryModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam::Queries;
using namespace Nexus;
using namespace Spire;
using Query = ComboBox::QueryModel::Query;

std::size_t LocalComboBoxQueryModel::AnyHash::operator
    ()(const std::any& value) const {
  // TODO: rest of these types
  if(value.type() == typeid(std::string)) {
    return std::hash<std::string>{}(std::any_cast<std::string>(value));
  } else if(value.type() == typeid(CurrencyId)) {
    return std::hash<CurrencyId>{}(std::any_cast<CurrencyId>(value));
  } else if(value.type() == typeid(MarketToken)) {

  } else if(value.type() == typeid(Region)) {

  } else if(value.type() == typeid(OrderStatus)) {

  } else if(value.type() == typeid(OrderType)) {

  } else if(value.type() == typeid(PositionSideToken)) {

  } else if(value.type() == typeid(Security)) {
    return std::hash<Security>{}(std::any_cast<Security>(value));
  } else if(value.type() == typeid(Side)) {

  } else if(value.type() == typeid(TimeInForce)) {
    
  } else if(value.type() == typeid(QString)) {
    return qHash(std::any_cast<QString>(value));
  }
  return 0;
}

bool LocalComboBoxQueryModel::Predicate::operator
    ()(const std::any& first, const std::any& second) const {
  return is_equal(first, second);
}

void LocalComboBoxQueryModel::add(const std::any& value) {
  m_data.insert(value);
}
  
void LocalComboBoxQueryModel::remove(const std::any& value) {
  m_data.erase(value);
}

QtPromise<std::vector<std::any>>
    LocalComboBoxQueryModel::query(const Query& query) const {
  if(query.m_limit == SnapshotLimit::None()) {
    return QtPromise(std::vector<std::any>());
  }
  // TODO: reentrancy, and what happens if the data is modified by a query?
  auto data = std::vector<std::any>();
  for(auto& item : m_data) {
    if(displayTextAny(item).toLower().startsWith(
        QString::fromStdString(query.m_text).toLower())) {
      data.push_back(item);
      if(data.size() == query.m_limit.GetSize()) {
        break;
      }
    }
  }
  return QtPromise(data);
}
