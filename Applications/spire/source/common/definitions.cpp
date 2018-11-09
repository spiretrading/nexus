#include "spire/spire/definitions.hpp"

using namespace boost;
using namespace boost::local_time;
using namespace Nexus;
using namespace Spire;

Definitions::Definitions(CountryDatabase country_database,
    MarketDatabase market_database, tz_database time_zone_database)
    : m_country_database(std::move(country_database)),
      m_market_database(std::move(market_database)),
      m_time_zone_database(std::move(time_zone_database)) {}

const CountryDatabase& Definitions::get_country_database() const {
  return m_country_database;
}

const MarketDatabase& Definitions::get_market_database() const {
  return m_market_database;
}

const tz_database& Definitions::get_time_zone_database() const {
  return m_time_zone_database;
}
