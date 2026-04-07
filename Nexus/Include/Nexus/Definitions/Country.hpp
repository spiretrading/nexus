#ifndef NEXUS_COUNTRY_HPP
#define NEXUS_COUNTRY_HPP
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <istream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <Beam/Collections/View.hpp>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <Beam/Utilities/ScopedStreamManipulator.hpp>
#include <Beam/Utilities/YamlConfig.hpp>

namespace Nexus {

  /** Identifies a country by a unique code. */
  class CountryCode {
    public:

      /** Represents an invalid or no country. */
      static const CountryCode NONE;

      /** Constructs an invalid code. */
      constexpr CountryCode() noexcept;

      /**
       * Constructs a CountryCode from its ISO code.
       * @param value The country's ISO code.
       */
      constexpr explicit CountryCode(std::uint16_t value) noexcept;

      /** Returns the integral representation of this code. */
      constexpr explicit operator std::uint16_t() const noexcept;

      /** Tests if this CountryCode is not equal to NONE. */
      constexpr explicit operator bool() const;

      constexpr auto operator <=>(const CountryCode&) const = default;

    private:
      std::uint16_t m_value;
  };

  inline const CountryCode CountryCode::NONE(~0);

  /** Stores a database of countries. */
  class CountryDatabase {
    public:

      /** The type used to represent a two-letter country code. */
      using TwoLetterCode = Beam::FixedString<2>;

      /** The type used to represent a three-letter country code. */
      using ThreeLetterCode = Beam::FixedString<3>;

      /** Stores a single entry in a CountryDatabase. */
      struct Entry {

        /** The country's code. */
        CountryCode m_code;

        /** The short form name. */
        std::string m_name;

        /** The country's two letter code. */
        TwoLetterCode m_two_letter_code;

        /** The country's three letter code. */
        ThreeLetterCode m_three_letter_code;

        bool operator ==(const Entry& rhs) const = default;
      };

      /** The entry returned for any failed lookup. */
      inline static const auto NONE = [] {
        auto none = Entry();
        none.m_two_letter_code = "??";
        none.m_three_letter_code = "???";
        none.m_name = "";
        none.m_code = CountryCode::NONE;
        return none;
      }();

      /** Constructs an empty CountryDatabase. */
      CountryDatabase() = default;

      CountryDatabase(const CountryDatabase& database) noexcept;

      /** Returns the list of countries represented. */
      Beam::View<const Entry> get_entries() const;

      /** Returns the country represented by its numeric code. */
      const Entry& from(CountryCode code) const;

      /** Returns the country represented by its name. */
      const Entry& from_name(std::string_view name) const;

      /** Returns the country represented by its code. */
      const Entry& from(const char* code) const;

      /** Returns the country represented by its code. */
      const Entry& from(std::string_view code) const;

      /** Returns the country represented by its two letter code. */
      const Entry& from(TwoLetterCode code) const;

      /** Returns the country represented by its three letter code. */
      const Entry& from(ThreeLetterCode code) const;

      /**
       * Adds an Entry.
       * @param entry The Entry to add.
       */
      void add(const Entry& entry);

      /**
       * Removes an Entry.
       * @param code The CountryCode of the Entry to delete.
       */
      void remove(CountryCode code);

      CountryDatabase& operator =(const CountryDatabase& database) noexcept;

    private:
      friend struct Beam::Shuttle<CountryDatabase>;
      std::atomic<std::shared_ptr<std::vector<Entry>>> m_entries;
  };

  /**
   * Parses a CountryCode from a string.
   * @param source The string to parse.
   * @param database The CountryDatabase used to find the CountryCode.
   * @return The CountryCode represented by the <i>source</i>.
   */
  inline CountryCode parse_country_code(
      std::string_view source, const CountryDatabase& database) {
    auto length = source.size();
    if(length == 2) {
      auto code = database.from(CountryDatabase::TwoLetterCode(source));
      if(code.m_code) {
        return code.m_code;
      }
    } else if(length == 3) {
      auto code = database.from(CountryDatabase::ThreeLetterCode(source));
      if(code.m_code) {
        return code.m_code;
      }
    }
    return database.from_name(source).m_code;
  }

  /**
   * Parses a CountryCode from a string using the default database.
   * @param source The string to parse.
   * @return The CountryCode represented by the <i>source</i>.
   */
  inline CountryCode parse_country_code(std::string_view source) {
    extern const CountryDatabase& DEFAULT_COUNTRIES;
    return parse_country_code(source, DEFAULT_COUNTRIES);
  }

  /**
   * Parses a CountryDatabase Entry from a YAML node.
   * @param node The node to parse the CountryDatabase Entry from.
   * @return The CountryDatabase Entry represented by the <i>node</i>.
   */
  inline CountryDatabase::Entry parse_country_database_entry(
      const YAML::Node& node) {
    return Beam::try_or_nest([&] {
      auto entry = CountryDatabase::Entry();
      entry.m_name = Beam::extract<std::string>(node, "name");
      entry.m_two_letter_code =
        Beam::extract<std::string>(node, "two_letter_code");
      entry.m_three_letter_code =
        Beam::extract<std::string>(node, "three_letter_code");
      entry.m_code = Beam::extract<CountryCode>(node, "code");
      return entry;
    }, std::runtime_error("Failed to parse country database entry."));
  }

  /**
   * Parses a CountryDatabase from a YAML node.
   * @param node The node to parse the CountryDatabase from.
   * @return The CountryDatabase represented by the <i>node</i>.
   */
  inline CountryDatabase parse_country_database(const YAML::Node& node) {
    return Beam::try_or_nest([&] {
      auto database = CountryDatabase();
      for(auto& entry : node) {
        database.add(parse_country_database_entry(entry));
      }
      return database;
    }, std::runtime_error("Failed to parse country database."));
  }

  inline auto operator <<(std::ostream& out, const CountryDatabase& database) {
    return Beam::ScopedStreamManipulator(out, database);
  }

  inline std::ostream& operator <<(std::ostream& out, CountryCode code) {
    extern const CountryDatabase& DEFAULT_COUNTRIES;
    auto database = static_cast<const CountryDatabase*>(
      out.pword(Beam::ScopedStreamManipulator<CountryDatabase>::ID));
    if(!database) {
      database = &DEFAULT_COUNTRIES;
    }
    auto& entry = database->from(code);
    if(entry.m_code) {
      return out << entry.m_two_letter_code;
    }
    return out << static_cast<std::uint16_t>(code);
  }

  inline std::istream& operator >>(std::istream& in, CountryCode& code) {
    auto value = std::uint16_t();
    in >> value;
    code = CountryCode(value);
    return in;
  }

  inline std::size_t hash_value(CountryCode code) {
    return static_cast<std::uint16_t>(code);
  }

  constexpr CountryCode::CountryCode() noexcept
    : CountryCode(~0) {}

  constexpr CountryCode::CountryCode(std::uint16_t value) noexcept
    : m_value(value) {}

  constexpr CountryCode::operator std::uint16_t() const noexcept {
    return m_value;
  }

  constexpr CountryCode::operator bool() const {
    return m_value != CountryCode().m_value;
  }

  inline CountryDatabase::CountryDatabase(
    const CountryDatabase& database) noexcept
    : m_entries(database.m_entries.load()) {}

  inline Beam::View<const CountryDatabase::Entry>
      CountryDatabase::get_entries() const {
    if(auto entries = m_entries.load()) {
      return Beam::View(Beam::SharedIterator(entries, entries->begin()),
        Beam::SharedIterator(entries, entries->end()));
    }
    return Beam::View<const Entry>();
  }

  inline const CountryDatabase::Entry& CountryDatabase::from(
      CountryCode code) const {
    if(auto entries = m_entries.load()) {
      auto i = std::lower_bound(entries->begin(), entries->end(), code,
        [] (const auto& lhs, auto rhs) {
          return lhs.m_code < rhs;
        });
      if(i != entries->end() && i->m_code == code) {
        return *i;
      }
    }
    return NONE;
  }

  inline const CountryDatabase::Entry& CountryDatabase::from_name(
      std::string_view name) const {
    if(auto entries = m_entries.load()) {
      auto i = std::find_if(entries->begin(), entries->end(),
        [&] (const auto& entry) {
          return entry.m_name == name;
        });
      if(i != entries->end()) {
        return *i;
      }
    }
    return NONE;
  }

  inline const CountryDatabase::Entry& CountryDatabase::from(
      const char* code) const {
    return from(std::string_view(code));
  }

  inline const CountryDatabase::Entry& CountryDatabase::from(
      std::string_view code) const {
    auto length = code.size();
    if(length == 2) {
      return from(TwoLetterCode(code));
    } else if(length == 3) {
      return from(ThreeLetterCode(code));
    }
    return NONE;
  }

  inline const CountryDatabase::Entry& CountryDatabase::from(
      TwoLetterCode code) const {
    if(auto entries = m_entries.load()) {
      auto i = std::find_if(entries->begin(), entries->end(),
        [&] (const auto& entry) {
          return entry.m_two_letter_code == code;
        });
      if(i != entries->end()) {
        return *i;
      }
    }
    return NONE;
  }

  inline const CountryDatabase::Entry& CountryDatabase::from(
      ThreeLetterCode code) const {
    if(auto entries = m_entries.load()) {
      auto i = std::find_if(entries->begin(), entries->end(),
        [&] (const auto& entry) {
          return entry.m_three_letter_code == code;
        });
      if(i != entries->end()) {
        return *i;
      }
    }
    return NONE;
  }

  inline void CountryDatabase::add(const Entry& entry) {
    while(true) {
      auto entries = m_entries.load();
      auto new_entries = [&] {
        if(!entries) {
          auto new_entries = std::make_shared<std::vector<Entry>>();
          new_entries->push_back(entry);
          return new_entries;
        }
        auto i = std::lower_bound(entries->begin(), entries->end(), entry,
          [] (const auto& lhs, const auto& rhs) {
            return lhs.m_code < rhs.m_code;
          });
        if(i == entries->end() || i->m_code != entry.m_code) {
          auto new_entries = std::make_shared<std::vector<Entry>>(*entries);
          new_entries->insert(
            new_entries->begin() + std::distance(entries->begin(), i), entry);
          return new_entries;
        }
        return std::shared_ptr<std::vector<Entry>>();
      }();
      if(!new_entries ||
          m_entries.compare_exchange_weak(entries, new_entries)) {
        break;
      }
    }
  }

  inline void CountryDatabase::remove(CountryCode code) {
    while(true) {
      auto entries = m_entries.load();
      if(!entries) {
        break;
      }
      auto i = std::lower_bound(entries->begin(), entries->end(), code,
        [] (const auto& lhs, auto rhs) {
          return lhs.m_code < rhs;
        });
      if(i != entries->end() && i->m_code == code) {
        auto new_entries = std::make_shared<std::vector<Entry>>(*entries);
        new_entries->erase(
          new_entries->begin() + std::distance(entries->begin(), i));
        if(m_entries.compare_exchange_weak(entries, new_entries)) {
          break;
        }
      } else {
        break;
      }
    }
  }

  inline CountryDatabase& CountryDatabase::operator =(
      const CountryDatabase& database) noexcept {
    m_entries.store(database.m_entries.load());
    return *this;
  }
}

namespace Beam {
  template<>
  constexpr auto is_structure<Nexus::CountryCode> = false;

  template<>
  struct Send<Nexus::CountryCode> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, Nexus::CountryCode value) const {
      sender.send(name, static_cast<std::uint16_t>(value));
    }
  };

  template<>
  struct Receive<Nexus::CountryCode> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, Nexus::CountryCode& value) const {
      value = Nexus::CountryCode(receive<std::uint16_t>(receiver, name));
    }
  };

  template<>
  struct Shuttle<Nexus::CountryDatabase::Entry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::CountryDatabase::Entry& value,
        unsigned int version) const {
      shuttle.shuttle("code", value.m_code);
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("two_letter_code", value.m_two_letter_code);
      shuttle.shuttle("three_letter_code", value.m_three_letter_code);
    }
  };

  template<>
  struct Shuttle<Nexus::CountryDatabase> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::CountryDatabase& value, unsigned int version) const {
      if constexpr(IsSender<S>) {
        if(auto entries = value.m_entries.load()) {
          shuttle.send("entries", *entries);
        }
      } else {
        auto entries =
          std::make_shared<std::vector<Nexus::CountryDatabase::Entry>>();
        shuttle.shuttle("entries", *entries);
        value.m_entries.store(std::move(entries));
      }
    }
  };
}

namespace std {
  template <>
  struct hash<Nexus::CountryCode> {
    size_t operator()(Nexus::CountryCode value) const {
      return Nexus::hash_value(value);
    }
  };
}

#include "Nexus/Definitions/DefaultCountryDatabase.hpp"

#endif
