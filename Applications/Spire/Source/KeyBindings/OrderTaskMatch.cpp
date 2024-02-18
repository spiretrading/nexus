#include "Spire/KeyBindings/OrderTaskMatch.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  bool suffix_matches(const QString& s1, const QString& s2) {
    for(auto i = 0; i < s1.length(); ++i) {
      if(s1.right(s1.length() - i).startsWith(s2)) {
        return true;
      }
    }
    return false;
  }
}

bool Spire::matches(const QString& name, const QString& query) {
  return suffix_matches(name.toLower(), query.toLower());
}

bool Spire::matches(const CountryCode& country, const QString& query) {
  auto& entry = GetDefaultCountryDatabase().FromCode(country);
  if(entry.m_code == CountryCode::NONE) {
    return false;
  }
  auto lower_query = query.toLower();
  return QString::fromStdString(
      entry.m_twoLetterCode.GetData()).toLower().startsWith(lower_query) ||
    QString::fromStdString(entry.m_name).toLower().startsWith(lower_query) ||
    matches(QString::fromStdString(entry.m_threeLetterCode.GetData()), query);
}

bool Spire::matches(const MarketCode& market, const QString& query) {
  auto& entry = GetDefaultMarketDatabase().FromCode(market);
  if(entry.m_code.IsEmpty()) {
    return false;
  }
  auto lower_query = query.toLower();
  return QString::fromStdString(
      entry.m_code.GetData()).toLower().startsWith(lower_query) ||
    QString::fromStdString(
      entry.m_description).toLower().startsWith(lower_query) ||
    matches(QString::fromStdString(entry.m_displayName), query);
}

bool Spire::matches(const Security& security, const QString& query) {
  if(security.GetSymbol().empty()) {
    return false;
  }
  if(matches(to_text(security), query)) {
    return true;
  }
  return matches(security.GetMarket(), query);
}

bool Spire::matches(const Region& region, const QString& query) {
  if(!region.GetName().empty() && QString::fromStdString(
      region.GetName()).toLower().startsWith(query.toLower())) {
    return true;
  }
  for(auto& country : region.GetCountries()) {
    if(matches(country, query)) {
      return true;
    }
  }
  for(auto& market : region.GetMarkets()) {
    if(matches(market, query)) {
      return true;
    }
  }
  for(auto& security : region.GetSecurities()) {
    if(matches(security, query)) {
      return true;
    }
  }
  return false;
}

bool Spire::matches(const Destination& destination, const QString& query) {
  auto& entry = GetDefaultDestinationDatabase().FromId(destination);
  if(entry.m_id.empty()) {
    return false;
  }
  return matches(QString::fromStdString(entry.m_id), query) ||
    QString::fromStdString(entry.m_description).toLower().startsWith(
      query.toLower());
}

bool Spire::matches(const OrderType& order_type, const QString& query) {
  if(order_type == OrderType::NONE) {
    return false;
  }
  return matches(to_text(order_type), query);
}

bool Spire::matches(const Side& side, const QString& query) {
  if(side == Side::NONE) {
    return false;
  }
  if(matches(to_text(side), query)) {
    return true;
  }
  auto osstr = std::ostringstream();
  osstr << side;
  return QString::fromStdString(osstr.str()).toLower().startsWith(
    query.toLower());
}

bool Spire::matches(const optional<Quantity>& quantity, const QString& query) {
  if(!quantity) {
    return false;
  }
  auto osstr = std::ostringstream();
  osstr << *quantity;
  auto string = osstr.str();
  auto text = [&] {
    if(auto pos = string.find('.'); pos != std::string::npos) {
      if(auto last_zero_pos = string.find_last_not_of('0');
          last_zero_pos > pos) {
        return string.erase(last_zero_pos + 1, std::string::npos);
      }
    }
    return string;
  }();
  return matches(QString::fromStdString(text), query);
}

bool Spire::matches(const TimeInForce& time_in_force, const QString& query) {
  if(time_in_force.GetType() == TimeInForce::Type::NONE) {
    return false;
  }
  if(matches(to_text(time_in_force), query)) {
    return true;
  }
  static auto descriptions =
    std::unordered_map<TimeInForce::Type, std::vector<QString>>();
  descriptions.emplace(TimeInForce::Type::DAY, std::vector<QString>{"session"});
  descriptions.emplace(TimeInForce::Type::FOK,
    std::vector<QString>{"fill", "kill", "fill or kill"});
  descriptions.emplace(TimeInForce::Type::GTC,
    std::vector<QString>{"good", "cancel", "good till cancel"});
  descriptions.emplace(TimeInForce::Type::GTD,
    std::vector<QString>{"good", "date", "good till date"});
  descriptions.emplace(TimeInForce::Type::GTX,
    std::vector<QString>{"good", "crossing", "good till crossing"});
  descriptions.emplace(TimeInForce::Type::IOC,
    std::vector<QString>{"immediate", "cancel", "immediate or cancel"});
  descriptions.emplace(TimeInForce::Type::MOC,
    std::vector<QString>{"close", "at the close"});
  descriptions.emplace(TimeInForce::Type::OPG,
    std::vector<QString>{"opening", "at the opening"});
  auto lower_query = query.toLower();
  for(auto& description : descriptions[time_in_force.GetType()]) {
    if(description.startsWith(lower_query)) {
      return true;
    }
  }
  return false;
}

bool Spire::matches(const QKeySequence& key, const QString& query) {
  return matches(key.toString().replace('+', ' '), query);
}
