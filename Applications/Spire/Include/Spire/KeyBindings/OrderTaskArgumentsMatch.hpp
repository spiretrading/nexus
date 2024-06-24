#ifndef SPIRE_ORDER_TASK_MATCH_HPP
#define SPIRE_ORDER_TASK_MATCH_HPP
#include "Spire/KeyBindings/OrderTaskArguments.hpp"

namespace Spire {

  /** Matches the query against the name. */
  bool matches(const QString& name, const QString& query);

  /** Matches the query against the country. */
  bool matches(Nexus::CountryCode country, const QString& query,
    const Nexus::CountryDatabase& countries);

  /** Matches the query against the market. */
  bool matches(Nexus::MarketCode market, const QString& query,
    const Nexus::MarketDatabase& markets);

  /** Matches the query against the security. */
  bool matches(const Nexus::Security& security, const QString& query,
    const Nexus::MarketDatabase& markets);

  /** Matches the query against the region. */
  bool matches(const Nexus::Region& region, const QString& query,
    const Nexus::CountryDatabase& countries,
    const Nexus::MarketDatabase& markets);

  /** Matches the query against the destination. */
  bool matches(const Nexus::Destination& destination, const QString& query,
    const Nexus::DestinationDatabase& destinations);

  /** Matches the query against the order type. */
  bool matches(Nexus::OrderType order_type, const QString& query);

  /** Matches the query against the side. */
  bool matches(Nexus::Side side, const QString& query);

  /** Matches the query against the quantity setting. */
  bool matches(QuantitySetting setting, const QString& query);

  /** Matches the query against the time in force. */
  bool matches(Nexus::TimeInForce time_in_force, const QString& query);

  /** Matches the query against the tags. */
  bool matches(const std::vector<AdditionalTag>& tags, const QString& query);

  /** Matches the query against the key. */
  bool matches(const QKeySequence& key, const QString& query);

  /** Matches the query against the OrderTaskArguments. */
  bool matches(const OrderTaskArguments& order_task, const QString& query,
    const Nexus::CountryDatabase& countries,
    const Nexus::MarketDatabase& markets,
    const Nexus::DestinationDatabase& destinations);
}

#endif
