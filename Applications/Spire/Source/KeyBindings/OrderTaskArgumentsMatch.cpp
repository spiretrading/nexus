#include "Spire/KeyBindings/OrderTaskArgumentsMatch.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  bool matches_by_word(const QString& text, const QString& query) {
    auto words = text.splitRef(' ', Qt::SkipEmptyParts);
    for(auto& word : words) {
      if(text.midRef(
          text.indexOf(word)).startsWith(query, Qt::CaseInsensitive)) {
        return true;
      }
    }
    return false;
  }
}

bool Spire::matches(const QString& name, const QString& query) {
  for(auto i = 0; i < name.length(); ++i) {
    if(name.right(name.length() - i).startsWith(query, Qt::CaseInsensitive)) {
      return true;
    }
  }
  return false;
}

bool Spire::matches(CountryCode country, const QString& query,
    const CountryDatabase& countries) {
  auto& entry = countries.FromCode(country);
  return entry.m_code != CountryCode::NONE &&
    (QString::fromStdString(entry.m_twoLetterCode.GetData()).startsWith(
        query, Qt::CaseInsensitive) ||
      QString::fromStdString(entry.m_threeLetterCode.GetData()).startsWith(
        query, Qt::CaseInsensitive) ||
      QString::fromStdString(entry.m_name).startsWith(
        query, Qt::CaseInsensitive));
}

bool Spire::matches(MarketCode market, const QString& query,
    const MarketDatabase& markets) {
  auto& entry = markets.FromCode(market);
  return !entry.m_code.IsEmpty() &&
    (QString::fromStdString(entry.m_code.GetData()).startsWith(
        query, Qt::CaseInsensitive) ||
      QString::fromStdString(entry.m_displayName).startsWith(
        query, Qt::CaseInsensitive) ||
      matches_by_word(QString::fromStdString(entry.m_description), query));
}

bool Spire::matches(const Security& security, const QString& query,
    const MarketDatabase& markets) {
  return !security.GetSymbol().empty() &&
    (to_text(security).startsWith(query, Qt::CaseInsensitive) ||
      matches(security.GetMarket(), query, markets));
}

bool Spire::matches(const Region& region, const QString& query,
    const CountryDatabase& countries, const MarketDatabase& markets) {
  if(!region.GetName().empty() && QString::fromStdString(
      region.GetName()).startsWith(query, Qt::CaseInsensitive)) {
    return true;
  }
  for(auto& country : region.GetCountries()) {
    if(matches(country, query, countries)) {
      return true;
    }
  }
  for(auto& market : region.GetMarkets()) {
    if(matches(market, query, markets)) {
      return true;
    }
  }
  for(auto& security : region.GetSecurities()) {
    if(matches(security, query, markets)) {
      return true;
    }
  }
  return false;
}

bool Spire::matches(const Destination& destination, const QString& query,
    const DestinationDatabase& destinations) {
  auto& entry = destinations.FromId(destination);
  return !entry.m_id.empty() &&
    (QString::fromStdString(entry.m_id).startsWith(
        query, Qt::CaseInsensitive) ||
      matches_by_word(QString::fromStdString(entry.m_description), query));
}

bool Spire::matches(OrderType order_type, const QString& query) {
  return order_type != OrderType::NONE &&
    to_text(order_type).startsWith(query, Qt::CaseInsensitive);
}

bool Spire::matches(Side side, const QString& query) {
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

bool Spire::matches(QuantitySetting setting, const QString& query) {
  return to_text(setting).startsWith(query, Qt::CaseInsensitive);
}

bool Spire::matches(TimeInForce time_in_force, const QString& query) {
  if(time_in_force.GetType() == TimeInForce::Type::NONE) {
    return false;
  }
  if(to_text(time_in_force).startsWith(query, Qt::CaseInsensitive)) {
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
    if(description.startsWith(query, Qt::CaseInsensitive)) {
      return true;
    }
  }
  return false;
}

bool Spire::matches(
    const std::vector<AdditionalTag>& tags, const QString& query) {
  return false;
}

bool Spire::matches(const QKeySequence& key_sequence, const QString& query) {
  return matches_by_word(key_sequence.toString().replace('+', ' '), query);
}

bool Spire::matches(const OrderTaskArguments& order_task, const QString& query,
    const CountryDatabase& countries, const MarketDatabase& markets,
    const DestinationDatabase& destinations) {
  return matches(order_task.m_name, query) ||
    matches(order_task.m_region, query, countries, markets) ||
    matches(order_task.m_destination, query, destinations) ||
    matches(order_task.m_order_type, query) ||
    matches(order_task.m_side, query) ||
    matches(order_task.m_quantity, query) ||
    matches(order_task.m_time_in_force, query) ||
    matches(order_task.m_additional_tags, query) ||
    matches(order_task.m_key, query);
}
