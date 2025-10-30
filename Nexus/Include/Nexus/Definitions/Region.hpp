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
      static const Region GLOBAL;

      /**
       * Constructs a global named Region.
       * @param name The name of the Region.
       */
      static Region make_global(std::string name);

      /** Constructs an empty Region. */
      Region() noexcept;

      /**
       * Constructs an empty named Region.
       * @param name The name of the Region.
       */
      explicit Region(std::string name) noexcept;

      /**
       * Constructs a Region consisting of a single country.
       * @param country The country to represent.
       */
      Region(CountryCode country);

      /**
       * Constructs a Region consisting of a single venue.
       * @param venue The venue to represent.
       */
      Region(Venue venue);

      /**
       * Constructs a Region consisting of a single Security.
       * @param security The Security to represent.
       */
      Region(Security security);

      /** Returns the name of this Region. */
      const std::string& get_name() const;

      /** Returns <code>true</code> iff this is the global Region. */
      bool is_global() const;

      /** Returns <code>true</code> iff this Region is empty. */
      bool is_empty() const;

      /** Returns the countries in this Region. */
      const std::unordered_set<CountryCode>& get_countries() const;

      /** Returns the venues in this Region. */
      const std::unordered_set<Venue>& get_venues() const;

      /** Returns the Securities in this Region. */
      const std::unordered_set<Security>& get_securities() const;

      /**
       * Returns <code>true</code> iff <i>region</i> is a subset of
       * <i>this</i>.
       */
      bool contains(const Region& region) const;

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
      friend struct Beam::Shuttle<Region>;
      std::string m_name;
      bool m_is_global;
      std::unordered_set<CountryCode> m_countries;
      std::unordered_set<Venue> m_venues;
      std::unordered_set<Security> m_securities;

      explicit Region(GlobalTag);
      Region(GlobalTag, std::string name);
  };

  inline std::ostream& operator <<(std::ostream& out, const Region& region) {
    if(!region.get_name().empty()) {
      return out << region.get_name();
    } else {
      return out;
    }
  }

  inline std::size_t hash_value(const Region& region) {
    if(region.is_global()) {
      return std::size_t(0x9e3779b97f4a7c15);
    }
    const auto COUNTRY_SALT = std::size_t(0x1bd11bdaa9fc1a22);
    const auto VENUE_SALT = std::size_t(0x3c79ac492ba7b653);
    const auto SECURITY_SALT = std::size_t(0x1f123bb5dfcbb123);
    auto mix = [] (auto x) {
      x += 0x9e3779b97f4a7c15ull;
      x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
      x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
      return x ^ (x >> 31);
    };
    auto rotate_left = [] (auto x, auto r) {
      return (x << r) | (x >> (64 - r));
    };
    auto set_hash = [&] (const auto& set, auto salt) {
      auto sum = std::size_t(0);
      auto xors = std::size_t(0);
      for(auto& element : set) {
        auto hv = mix(set.hash_function()(element) + salt);
        sum += hv;
        xors ^= rotate_left(hv, 23);
      }
      return mix(sum + rotate_left(xors, 17));
    };
    auto countries_hash = set_hash(region.get_countries(), COUNTRY_SALT);
    auto venues_hash = set_hash(region.get_venues(), VENUE_SALT);
    auto securities_hash = set_hash(region.get_securities(), SECURITY_SALT);
    auto hash = countries_hash + rotate_left(venues_hash, 21) +
      rotate_left(securities_hash, 42);
    return mix(hash);
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

  inline const Region Region::GLOBAL = Region(Region::GlobalTag());

  inline Region Region::make_global(std::string name) {
    return Region(GlobalTag(), std::move(name));
  }

  inline Region::Region() noexcept
    : m_is_global(false) {}

  inline Region::Region(std::string name) noexcept
    : m_is_global(false),
      m_name(std::move(name)) {}

  inline Region::Region(CountryCode country)
      : m_is_global(false) {
    m_countries.insert(country);
  }

  inline Region::Region(Venue venue)
      : m_is_global(false) {
    m_venues.insert(venue);
  }

  inline Region::Region(Security security)
      : m_is_global(false) {
    m_securities.insert(std::move(security));
  }

  inline const std::string& Region::get_name() const {
    return m_name;
  }

  inline bool Region::is_global() const {
    return m_is_global;
  }

  inline bool Region::is_empty() const {
    return m_countries.empty() && m_venues.empty() && m_securities.empty() &&
      !is_global();
  }

  inline const std::unordered_set<CountryCode>& Region::get_countries() const {
    return m_countries;
  }

  inline const std::unordered_set<Venue>& Region::get_venues() const {
    return m_venues;
  }

  inline const std::unordered_set<Security>& Region::get_securities() const {
    return m_securities;
  }

  inline bool Region::contains(const Region& region) const {
    return region <= *this;
  }

  inline Region& Region::operator +=(const Region& region) {
    if(region.m_is_global) {
      m_is_global = true;
      m_countries = {};
      m_venues = {};
      m_securities = {};
    } else if(!m_is_global) {
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
    if(region.m_is_global) {
      return true;
    } else if(m_is_global) {
      return false;
    }
    for(auto& security : m_securities) {
      if(region.m_securities.contains(security)) {
        continue;
      }
      if(region.m_venues.contains(security.get_venue())) {
        continue;
      }
      auto country = DEFAULT_VENUES.from(security.get_venue()).m_country_code;
      if(country && region.m_countries.contains(country)) {
        continue;
      }
      return false;
    }
    for(auto& venue : m_venues) {
      if(region.m_venues.contains(venue)) {
        continue;
      }
      auto country = DEFAULT_VENUES.from(venue).m_country_code;
      if(country && region.m_countries.contains(country)) {
        continue;
      }
      return false;
    }
    for(auto& country : m_countries) {
      if(region.m_countries.contains(country)) {
        continue;
      }
      return false;
    }
    return true;
  }

  inline bool Region::operator ==(const Region& region) const {
    return std::tie(m_is_global, m_countries, m_venues, m_securities) ==
      std::tie(region.m_is_global, region.m_countries, region.m_venues,
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
    : m_is_global(true) {}

  inline Region::Region(GlobalTag, std::string name)
    : m_is_global(true),
      m_name(std::move(name)) {}
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::Region> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::Region& value, unsigned int version) const {
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("is_global", value.m_is_global);
      shuttle.shuttle("countries", value.m_countries);
      shuttle.shuttle("venues", value.m_venues);
      shuttle.shuttle("securities", value.m_securities);
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
