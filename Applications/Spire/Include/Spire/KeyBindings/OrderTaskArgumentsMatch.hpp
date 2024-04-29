#ifndef SPIRE_ORDER_TASK_MATCH_HPP
#define SPIRE_ORDER_TASK_MATCH_HPP
#include "Spire/KeyBindings/OrderTaskArguments.hpp"

namespace Spire {

  /** Matches the query against the name. */
  bool matches(const QString& name, const QString& query);

  /** Matches the query against the country. */
  bool matches(const Nexus::CountryCode& country, const QString& query);

  /** Matches the query against the market. */
  bool matches(const Nexus::MarketCode& market, const QString& query);

  /** Matches the query against the security. */
  bool matches(const Nexus::Security& security, const QString& query);

  /** Matches the query against the region. */
  bool matches(const Nexus::Region& region, const QString& query);

  /** Matches the query against the destination. */
  bool matches(const Nexus::Destination& destination, const QString& query);

  /** Matches the query against the order type. */
  bool matches(const Nexus::OrderType& order_type, const QString& query);

  /** Matches the query against the side. */
  bool matches(const Nexus::Side& side, const QString& query);

  /** Matches the query against the quantity. */
  bool matches(const boost::optional<Nexus::Quantity>& quantity,
    const QString& query);

  /** Matches the query against the time in force. */
  bool matches(const Nexus::TimeInForce& time_in_force, const QString& query);

  /** Matches the query against the tags. */
  bool matches(const std::vector<Nexus::Tag>& tags, const QString& query);

  /** Matches the query against the key. */
  bool matches(const QKeySequence& key, const QString& query);

  /** Matches the query against the OrderTaskArguments. */
  bool matches(const OrderTaskArguments& order_task, const QString& query);
}

#endif
