#ifndef SPIRE_DEFINITIONS_HPP
#define SPIRE_DEFINITIONS_HPP
#include <boost/date_time/local_time/tz_database.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Stores the set of common financial/market definitions used. */
  class Definitions {
    public:

      /** Returns a set of Definitions based on the built-in defaults. */
      static Definitions GetDefaults();

      /**
       * Constructs the set of Definitions.
       * @param country_database The country database.
       * @param market_database The market database.
       * @param time_zone_database The time zone database.
       */
      Definitions(Nexus::CountryDatabase country_database,
        Nexus::MarketDatabase market_database,
        boost::local_time::tz_database time_zone_database);

      /** Returns the country database. */
      const Nexus::CountryDatabase& get_country_database() const;

      /** Returns the market database. */
      const Nexus::MarketDatabase& get_market_database() const;

      /** Returns the time zone database. */
      const boost::local_time::tz_database& get_time_zone_database() const;

    private:
      Nexus::CountryDatabase m_country_database;
      Nexus::MarketDatabase m_market_database;
      boost::local_time::tz_database m_time_zone_database;
  };
}

#endif
