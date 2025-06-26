#ifndef NEXUS_REGION_HPP
#define NEXUS_REGION_HPP
#include <functional>
#include <string>
#include <unordered_set>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUnorderedSet.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Represents a group of trading locations. */
  class Region {
    public:

      /** Returns the global Region. */
      static Region Global();

      /**
       * Constructs a global named Region.
       * @param name The name of the Region.
       */
      static Region Global(std::string name);

      /** Constructs an empty Region. */
      Region();

      /**
       * Constructs an empty named Region.
       * @param name The name of the Region.
       */
      explicit Region(std::string name);

      /**
       * Constructs a Region consisting of a single country.
       * @param country The country to represent.
       */
      Region(CountryCode country);

      /**
       * Constructs a Region consisting of a single venue.
       * @param venue The venue to represent.
       * @param country The country the venue belongs to.
       */
      Region(Venue venue, CountryCode country);

      /**
       * Constructs a Region consisting of a single venue.
       * @param venue The venue to represent.
       */
      Region(const VenueDatabase::Entry& venue);

      /**
       * Constructs a Region consisting of a single Security.
       * @param security The Security to represent.
       */
      Region(Security security);

      /** Returns the name of this Region. */
      const std::string& GetName() const;

      /** Sets the name of this Region. */
      void SetName(const std::string& name);

      /** Returns <code>true</code> iff this is the global Region. */
      bool IsGlobal() const;

      /** Returns <code>true</code> iff this Region is empty. */
      bool IsEmpty() const;

      /** Returns the countries in this Region. */
      const std::unordered_set<CountryCode>& GetCountries() const;

      /** Returns the venues in this Region. */
      std::unordered_set<Venue> GetVenues() const;

      /** Returns the Securities in this Region. */
      const std::unordered_set<Security>& GetSecurities() const;

      /**
       * Returns <code>true</code> iff <i>region</i> is a subset of
       * <i>this</i>.
       */
      bool Contains(const Region& region) const;

      /**
       * Combines <i>this</i> Region with another.
       * @param region The Region to combine.
       * @return A Region containing all of <i>this</i>'s elements and
       *         <i>region</i>'s elements.
       */
      Region& operator +=(const Region& region);

      /**
       * Returns <code>true</code> iff <i>this</i> Region is a strict subset of
       * another.
       */
      bool operator <(const Region& region) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Region is a subset of
       * another.
       */
      bool operator <=(const Region& region) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Region is equal to another.
       */
      bool operator ==(const Region& region) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Region is not equal to
       * another.
       */
      bool operator !=(const Region& region) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Region is a superset of
       * another.
       */
      bool operator >=(const Region& region) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Region is a strict superset
       * of another.
       */
      bool operator >(const Region& region) const;

    private:
      struct GlobalTag {};
      struct VenueEntry {
        Venue m_venue;
        CountryCode m_country;

        VenueEntry() = default;
        VenueEntry(Venue venue, CountryCode country);
        bool operator ==(const VenueEntry& venueEntry) const;
        friend struct Beam::Serialization::Shuttle<VenueEntry>;
      };
      struct VenueEntryHash {
        std::size_t operator ()(const VenueEntry& venueEntry) const;
      };
      friend struct Beam::Serialization::Shuttle<Region>;
      friend struct Beam::Serialization::Shuttle<Region::VenueEntry>;
      std::string m_name;
      bool m_isGlobal;
      std::unordered_set<CountryCode> m_countries;
      std::unordered_set<VenueEntry, VenueEntryHash> m_venues;
      std::unordered_set<Security> m_securities;

      explicit Region(GlobalTag);
      Region(GlobalTag, std::string name);
  };

  inline std::ostream& operator <<(std::ostream& out, const Region& region) {
    if(region.IsGlobal()) {
      return out << "GLOBAL";
    } else if(!region.GetName().empty()) {
      return out << region.GetName();
    } else {
      return out;
    }
  }

  inline std::size_t hash_value(const Region& region) {
    auto seed = std::size_t(0);
    for(auto& country : region.GetCountries()) {
      boost::hash_combine(seed, country);
    }
    for(auto& venue : region.GetVenues()) {
      boost::hash_combine(seed, venue);
    }
    for(auto& security : region.GetSecurities()) {
      boost::hash_combine(seed, security);
    }
    return seed;
  }

  inline bool operator <(const Security& security, const Region& region) {
    return Region(security) < region;
  }

  inline bool operator <=(const Security& security, const Region& region) {
    return Region(security) <= region;
  }

  inline bool operator ==(const Security& security, const Region& region) {
    return Region(security) == region;
  }

  inline bool operator !=(const Security& security, const Region& region) {
    return Region(security) != region;
  }

  inline bool operator >=(const Security& security, const Region& region) {
    return Region(security) >= region;
  }

  inline bool operator >(const Security& security, const Region& region) {
    return Region(security) > region;
  }

  inline Region operator +(Region left, const Region& right) {
    left += right;
    return left;
  }

  inline Region::VenueEntry::VenueEntry(Venue venue, CountryCode country)
    : m_venue(venue),
      m_country(country) {}

  inline bool Region::VenueEntry::operator ==(
      const VenueEntry& venueEntry) const {
    return m_venue == venueEntry.m_venue;
  }

  inline std::size_t Region::VenueEntryHash::operator ()(
      const Region::VenueEntry& value) const {
    return std::hash<Venue>()(value.m_venue);
  }

  inline Region Region::Global() {
    return Region(GlobalTag{});
  }

  inline Region Region::Global(std::string name) {
    return Region(GlobalTag{}, std::move(name));
  }

  inline Region::Region()
    : m_isGlobal(false) {}

  inline Region::Region(std::string name)
    : m_isGlobal(false),
      m_name(std::move(name)) {}

  inline Region::Region(CountryCode country)
      : m_isGlobal(false) {
    m_countries.insert(country);
  }

  inline Region::Region(Venue venue, CountryCode country)
      : m_isGlobal(false) {
    m_venues.insert(VenueEntry(venue, country));
  }

  inline Region::Region(const VenueDatabase::Entry& venue)
    : Region(venue.m_venue, venue.m_country_code) {}

  inline Region::Region(Security security)
      : m_isGlobal(false) {
    if(security.GetSymbol() == "*") {
      if(security.GetVenue() == "*" || security.GetVenue() == Venue()) {
        if(security.GetCountry() == CountryCode::NONE) {
          m_isGlobal = true;
        } else {
          m_countries.insert(security.GetCountry());
        }
      } else {
        m_venues.insert(
          VenueEntry(security.GetVenue(), security.GetCountry()));
      }
    } else {
      m_securities.insert(std::move(security));
    }
  }

  inline const std::string& Region::GetName() const {
    return m_name;
  }

  inline void Region::SetName(const std::string& name) {
    m_name = name;
  }

  inline bool Region::IsGlobal() const {
    return m_isGlobal;
  }

  inline bool Region::IsEmpty() const {
    return m_countries.empty() && m_venues.empty() && m_securities.empty();
  }

  inline const std::unordered_set<CountryCode>& Region::GetCountries() const {
    return m_countries;
  }

  inline std::unordered_set<Venue> Region::GetVenues() const {
    auto venues = std::unordered_set<Venue>();
    for(auto& venue : m_venues) {
      venues.insert(venue.m_venue);
    }
    return venues;
  }

  inline const std::unordered_set<Security>& Region::GetSecurities() const {
    return m_securities;
  }

  inline bool Region::Contains(const Region& region) const {
    return region <= *this;
  }

  inline Region& Region::operator +=(const Region& region) {
    if(region.m_isGlobal) {
      m_isGlobal = true;
      m_countries = {};
      m_venues = {};
      m_securities = {};
    } else if(!m_isGlobal) {
      m_countries.insert(region.m_countries.begin(), region.m_countries.end());
      m_venues.insert(region.m_venues.begin(), region.m_venues.end());
      m_securities.insert(
        region.m_securities.begin(), region.m_securities.end());
    }
    return *this;
  }

  inline bool Region::operator <(const Region& region) const {
    return (*this <= region) && *this != region;
  }

  inline bool Region::operator <=(const Region& region) const {
    if(region.m_isGlobal) {
      return true;
    } else if(m_isGlobal) {
      return false;
    }
    for(auto& security : m_securities) {
      if(region.m_securities.count(security)) {
        continue;
      }
      if(region.m_venues.count(
          VenueEntry(security.GetVenue(), security.GetCountry()))) {
        continue;
      }
      if(region.m_countries.count(security.GetCountry())) {
        continue;
      }
      return false;
    }
    for(auto& entry : m_venues) {
      if(region.m_venues.count(entry)) {
        continue;
      }
      if(region.m_countries.count(entry.m_country)) {
        continue;
      }
      return false;
    }
    for(auto& country : m_countries) {
      if(region.m_countries.count(country)) {
        continue;
      }
      return false;
    }
    return true;
  }

  inline bool Region::operator ==(const Region& region) const {
    return std::tie(m_isGlobal, m_countries, m_venues, m_securities) ==
      std::tie(region.m_isGlobal, region.m_countries, region.m_venues,
        region.m_securities);
  }

  inline bool Region::operator !=(const Region& region) const {
    return !(*this == region);
  }

  inline bool Region::operator >=(const Region& region) const {
    return region <= *this;
  }

  inline bool Region::operator >(const Region& region) const {
    return (*this >= region) && *this != region;
  }

  inline Region::Region(GlobalTag)
    : m_isGlobal(true) {}

  inline Region::Region(GlobalTag, std::string name)
    : m_isGlobal(true),
      m_name(std::move(name)) {}
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::Region::VenueEntry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::Region::VenueEntry& value,
        unsigned int version) {
      shuttle.Shuttle("venue", value.m_venue);
      shuttle.Shuttle("country", value.m_country);
    }
  };

  template<>
  struct Shuttle<Nexus::Region> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::Region& value,
        unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("is_global", value.m_isGlobal);
      shuttle.Shuttle("countries", value.m_countries);
      shuttle.Shuttle("venues", value.m_venues);
      shuttle.Shuttle("securities", value.m_securities);
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::Region> {
    std::size_t operator ()(const Nexus::Region& region) const {
      return hash_value(region);
    }
  };
}

#endif
