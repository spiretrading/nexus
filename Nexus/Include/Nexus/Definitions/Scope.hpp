#ifndef NEXUS_SCOPE_HPP
#define NEXUS_SCOPE_HPP
#include <functional>
#include <string>
#include <unordered_set>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUnorderedSet.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Represents a group of trading locations. */
  class Scope {
    public:

      /** Returns the global Scope. */
      static const Scope GLOBAL;

      /**
       * Constructs a global named Scope.
       * @param name The name of the Scope.
       */
      static Scope make_global(std::string name);

      /** Constructs an empty Scope. */
      Scope() noexcept;

      /**
       * Constructs an empty named Scope.
       * @param name The name of the Scope.
       */
      explicit Scope(std::string name) noexcept;

      /**
       * Constructs a Scope consisting of a single country.
       * @param country The country to represent.
       */
      Scope(CountryCode country);

      /**
       * Constructs a Scope consisting of a single venue.
       * @param venue The venue to represent.
       */
      Scope(Venue venue);

      /**
       * Constructs a Scope consisting of a single Ticker.
       * @param ticker The Ticker to represent.
       */
      Scope(Ticker ticker);

      /** Returns the name of this Scope. */
      const std::string& get_name() const;

      /** Returns <code>true</code> iff this is the global Scope. */
      bool is_global() const;

      /** Returns <code>true</code> iff this Scope is empty. */
      bool is_empty() const;

      /** Returns the countries in this Scope. */
      const std::unordered_set<CountryCode>& get_countries() const;

      /** Returns the venues in this Scope. */
      const std::unordered_set<Venue>& get_venues() const;

      /** Returns the Tickers in this Scope. */
      const std::unordered_set<Ticker>& get_tickers() const;

      /**
       * Returns <code>true</code> iff <i>scope</i> is a subset of
       * <i>this</i>.
       */
      bool contains(const Scope& scope) const;

      /**
       * Combines <i>this</i> Scope with another.
       * @param scope The Scope to combine.
       * @return A Scope containing all of <i>this</i>'s elements and
       *         <i>scope</i>'s elements.
       */
      Scope& operator +=(const Scope& scope);

      /**
       * Returns <code>true</code> iff <i>this</i> Scope is a strict subset of
       * another.
       */
      bool operator <(const Scope& scope) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Scope is a subset of
       * another.
       */
      bool operator <=(const Scope& scope) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Scope is equal to another.
       */
      bool operator ==(const Scope& scope) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Scope is not equal to
       * another.
       */
      bool operator !=(const Scope& scope) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Scope is a superset of
       * another.
       */
      bool operator >=(const Scope& scope) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Scope is a strict superset
       * of another.
       */
      bool operator >(const Scope& scope) const;

    private:
      struct GlobalTag {};
      friend struct Beam::Shuttle<Scope>;
      std::string m_name;
      bool m_is_global;
      std::unordered_set<CountryCode> m_countries;
      std::unordered_set<Venue> m_venues;
      std::unordered_set<Ticker> m_tickers;

      explicit Scope(GlobalTag);
      Scope(GlobalTag, std::string name);
  };

  inline std::ostream& operator <<(std::ostream& out, const Scope& scope) {
    if(!scope.get_name().empty()) {
      return out << scope.get_name();
    } else {
      return out;
    }
  }

  inline bool operator <(const Ticker& ticker, const Scope& scope) {
    return Scope(ticker) < scope;
  }

  inline bool operator <=(const Ticker& ticker, const Scope& scope) {
    return Scope(ticker) <= scope;
  }

  inline bool operator ==(const Ticker& ticker, const Scope& scope) {
    return Scope(ticker) == scope;
  }

  inline bool operator !=(const Ticker& ticker, const Scope& scope) {
    return Scope(ticker) != scope;
  }

  inline bool operator >=(const Ticker& ticker, const Scope& scope) {
    return Scope(ticker) >= scope;
  }

  inline bool operator >(const Ticker& ticker, const Scope& scope) {
    return Scope(ticker) > scope;
  }

  inline Scope operator +(Scope left, const Scope& right) {
    left += right;
    return left;
  }

  inline const Scope Scope::GLOBAL = Scope(Scope::GlobalTag());

  inline Scope Scope::make_global(std::string name) {
    return Scope(GlobalTag(), std::move(name));
  }

  inline Scope::Scope() noexcept
    : m_is_global(false) {}

  inline Scope::Scope(std::string name) noexcept
    : m_is_global(false),
      m_name(std::move(name)) {}

  inline Scope::Scope(CountryCode country)
      : m_is_global(false) {
    m_countries.insert(country);
  }

  inline Scope::Scope(Venue venue)
      : m_is_global(false) {
    m_venues.insert(venue);
  }

  inline Scope::Scope(Ticker ticker)
      : m_is_global(false) {
    m_tickers.insert(std::move(ticker));
  }

  inline const std::string& Scope::get_name() const {
    return m_name;
  }

  inline bool Scope::is_global() const {
    return m_is_global;
  }

  inline bool Scope::is_empty() const {
    return m_countries.empty() && m_venues.empty() && m_tickers.empty() &&
      !is_global();
  }

  inline const std::unordered_set<CountryCode>& Scope::get_countries() const {
    return m_countries;
  }

  inline const std::unordered_set<Venue>& Scope::get_venues() const {
    return m_venues;
  }

  inline const std::unordered_set<Ticker>& Scope::get_tickers() const {
    return m_tickers;
  }

  inline bool Scope::contains(const Scope& scope) const {
    return scope <= *this;
  }

  inline Scope& Scope::operator +=(const Scope& scope) {
    if(scope.m_is_global) {
      m_is_global = true;
      m_countries = {};
      m_venues = {};
      m_tickers = {};
    } else if(!m_is_global) {
      m_countries.insert(scope.m_countries.begin(), scope.m_countries.end());
      m_venues.insert(scope.m_venues.begin(), scope.m_venues.end());
      m_tickers.insert(scope.m_tickers.begin(), scope.m_tickers.end());
    }
    return *this;
  }

  inline bool Scope::operator <(const Scope& scope) const {
    return (*this <= scope) && *this != scope;
  }

  inline bool Scope::operator <=(const Scope& scope) const {
    if(scope.m_is_global) {
      return true;
    } else if(m_is_global) {
      return false;
    }
    for(auto& ticker : m_tickers) {
      if(scope.m_tickers.contains(ticker)) {
        continue;
      }
      if(scope.m_venues.contains(ticker.get_venue())) {
        continue;
      }
      auto country = DEFAULT_VENUES.from(ticker.get_venue()).m_country_code;
      if(country && scope.m_countries.contains(country)) {
        continue;
      }
      return false;
    }
    for(auto& venue : m_venues) {
      if(scope.m_venues.contains(venue)) {
        continue;
      }
      auto country = DEFAULT_VENUES.from(venue).m_country_code;
      if(country && scope.m_countries.contains(country)) {
        continue;
      }
      return false;
    }
    for(auto& country : m_countries) {
      if(scope.m_countries.contains(country)) {
        continue;
      }
      return false;
    }
    return true;
  }

  inline bool Scope::operator ==(const Scope& scope) const {
    return std::tie(m_is_global, m_countries, m_venues, m_tickers) ==
      std::tie(scope.m_is_global, scope.m_countries, scope.m_venues,
        scope.m_tickers);
  }

  inline bool Scope::operator !=(const Scope& scope) const {
    return !(*this == scope);
  }

  inline bool Scope::operator >=(const Scope& scope) const {
    return scope <= *this;
  }

  inline bool Scope::operator >(const Scope& scope) const {
    return (*this >= scope) && *this != scope;
  }

  inline Scope::Scope(GlobalTag)
    : m_is_global(true) {}

  inline Scope::Scope(GlobalTag, std::string name)
    : m_is_global(true),
      m_name(std::move(name)) {}
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::Scope> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::Scope& value, unsigned int version) const {
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("is_global", value.m_is_global);
      shuttle.shuttle("countries", value.m_countries);
      shuttle.shuttle("venues", value.m_venues);
      shuttle.shuttle("tickers", value.m_tickers);
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::Scope> {
    std::size_t operator ()(const Nexus::Scope& scope) const {
      if(scope.is_global()) {
        return std::size_t(0x9e3779b97f4a7c15);
      }
      const auto COUNTRY_SALT = std::size_t(0x1bd11bdaa9fc1a22);
      const auto VENUE_SALT = std::size_t(0x3c79ac492ba7b653);
      const auto TICKER_SALT = std::size_t(0x1f123bb5dfcbb123);
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
      auto countries_hash = set_hash(scope.get_countries(), COUNTRY_SALT);
      auto venues_hash = set_hash(scope.get_venues(), VENUE_SALT);
      auto tickers_hash = set_hash(scope.get_tickers(), TICKER_SALT);
      auto hash = countries_hash + rotate_left(venues_hash, 21) +
        rotate_left(tickers_hash, 42);
      return mix(hash);
    }
  };
}

#endif
