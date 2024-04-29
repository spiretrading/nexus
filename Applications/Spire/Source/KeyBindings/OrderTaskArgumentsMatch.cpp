#include "Spire/KeyBindings/OrderTaskArgumentsMatch.hpp"
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
  return QString::fromStdString(
      entry.m_twoLetterCode.GetData()).startsWith(query, Qt::CaseInsensitive) ||
    QString::fromStdString(
      entry.m_threeLetterCode.GetData()).startsWith(query,
        Qt::CaseInsensitive) ||
    QString::fromStdString(entry.m_name).startsWith(query, Qt::CaseInsensitive);
}

bool Spire::matches(const MarketCode& market, const QString& query) {
  auto& entry = GetDefaultMarketDatabase().FromCode(market);
  if(entry.m_code.IsEmpty()) {
    return false;
  }
  if(QString::fromStdString(
      entry.m_code.GetData()).startsWith(query, Qt::CaseInsensitive) ||
    QString::fromStdString(
      entry.m_displayName).startsWith(query, Qt::CaseInsensitive)) {
    return true;
  }
  for(auto& word : QString::fromStdString(entry.m_description).splitRef(' ')) {
    if(word.startsWith(query, Qt::CaseInsensitive)) {
      return true;
    }
  }
  return false;
}

bool Spire::matches(const Security& security, const QString& query) {
  if(security.GetSymbol().empty()) {
    return false;
  }
  if(to_text(security).startsWith(query, Qt::CaseInsensitive)) {
    return true;
  }
  return matches(security.GetMarket(), query);
}

bool Spire::matches(const Region& region, const QString& query) {
  if(!region.GetName().empty() && QString::fromStdString(
      region.GetName()).startsWith(query, Qt::CaseInsensitive)) {
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
  if(QString::fromStdString(entry.m_id).startsWith(query,
      Qt::CaseInsensitive)) {
    return true;
  }
  for(auto& word : QString::fromStdString(entry.m_description).splitRef(' ')) {
    if(word.startsWith(query, Qt::CaseInsensitive)) {
      return true;
    }
  }
  return false;
}

bool Spire::matches(const OrderType& order_type, const QString& query) {
  if(order_type == OrderType::NONE) {
    return false;
  }
  return to_text(order_type).startsWith(query, Qt::CaseInsensitive);
}

bool Spire::matches(const Side& side, const QString& query) {
  if(side == Side::NONE) {
    return false;
  }
  if(to_text(side).startsWith(query, Qt::CaseInsensitive)) {
    return true;
  }
  auto osstr = std::ostringstream();
  osstr << side;
  return QString::fromStdString(osstr.str()).startsWith(query,
    Qt::CaseInsensitive);
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
  return QString::fromStdString(text).startsWith(query, Qt::CaseInsensitive);
}

bool Spire::matches(const TimeInForce& time_in_force, const QString& query) {
  if(time_in_force.GetType() == TimeInForce::Type::NONE) {
    return false;
  }
  auto lower_query = query.toLower();
  if(to_text(time_in_force).toLower().startsWith(lower_query)) {
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
  for(auto& description : descriptions[time_in_force.GetType()]) {
    if(description.startsWith(lower_query)) {
      return true;
    }
  }
  return false;
}

bool Spire::matches(const std::vector<Nexus::Tag>& tags, const QString& query) {
  return false;
}

bool Spire::matches(const QKeySequence& key_sequence, const QString& query) {
  for(auto& key : key_sequence.toString().splitRef('+')) {
    if(key.startsWith(query, Qt::CaseInsensitive)) {
      return true;
    }
  }
  return key_sequence.toString().replace('+', ' ').startsWith(query,
    Qt::CaseInsensitive);
}

bool Spire::matches(const OrderTaskArguments& order_task,
    const QString& query) {
  return matches(order_task.m_name, query) ||
    matches(order_task.m_region, query) ||
    matches(order_task.m_destination, query) ||
    matches(order_task.m_order_type, query) ||
    matches(order_task.m_side, query) ||
    matches(order_task.m_quantity, query) ||
    matches(order_task.m_time_in_force, query) ||
    matches(order_task.m_additional_tags, query) ||
    matches(order_task.m_key, query);
}
