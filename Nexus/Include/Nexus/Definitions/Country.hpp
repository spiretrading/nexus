#ifndef NEXUS_COUNTRY_HPP
#define NEXUS_COUNTRY_HPP
#include <algorithm>
#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/hash.hpp>

namespace Nexus {
namespace Details {
  template<typename T>
  struct CountryCodeDefinitions {

    /** Stores an invalid country code. */
    static const T NONE;

    constexpr auto operator <=>(const CountryCodeDefinitions& other) const =
      default;
  };
}

  /** Identifies a country by a unique code. */
  class CountryCode : private Details::CountryCodeDefinitions<CountryCode> {
    public:

      /** Constructs an invalid code. */
      constexpr CountryCode() noexcept;

      /**
       * Constructs a CountryCode from its ISO code.
       * @param value The country's ISO code.
       */
      constexpr explicit CountryCode(std::uint16_t value) noexcept;

      /** Returns the integral representation of this code. */
      constexpr explicit operator std::uint16_t() const;

      constexpr auto operator <=>(const CountryCode& other) const = default;

      using Details::CountryCodeDefinitions<CountryCode>::NONE;

    private:
      std::uint16_t m_value;
  };

  /** Stores the database of all countries. */
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

      /** Returns the list of countries represented. */
      const std::vector<Entry>& get_entries() const;

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

    private:
      friend struct Beam::Serialization::Shuttle<CountryDatabase>;

      std::vector<Entry> m_entries;
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
      if(code.m_code != CountryCode::NONE) {
        return code.m_code;
      }
    } else if(length == 3) {
      auto code = database.from(CountryDatabase::ThreeLetterCode(source));
      if(code.m_code != CountryCode::NONE) {
        return code.m_code;
      }
    }
    return database.from_name(source).m_code;
  }

  /**
   * Parses a CountryDatabase Entry from a YAML node.
   * @param node The node to parse the CountryDatabase Entry from.
   * @return The CountryDatabase Entry represented by the <i>node</i>.
   */
  inline CountryDatabase::Entry parse_country_database_entry(
      const YAML::Node& node) {
    return Beam::TryOrNest([&] {
      auto entry = CountryDatabase::Entry();
      entry.m_name = Beam::Extract<std::string>(node, "name");
      entry.m_two_letter_code =
        Beam::Extract<std::string>(node, "two_letter_code");
      entry.m_three_letter_code =
        Beam::Extract<std::string>(node, "three_letter_code");
      entry.m_code = Beam::Extract<CountryCode>(node, "code");
      return entry;
    }, std::runtime_error("Failed to parse country database entry."));
  }

  /**
   * Parses a CountryDatabase from a YAML node.
   * @param node The node to parse the CountryDatabase from.
   * @return The CountryDatabase represented by the <i>node</i>.
   */
  inline CountryDatabase parse_country_database(const YAML::Node& node) {
    return Beam::TryOrNest([&] {
      auto database = CountryDatabase();
      for(auto& entry : node) {
        database.add(parse_country_database_entry(entry));
      }
      return database;
    }, std::runtime_error("Failed to parse country database."));
  }

  inline std::ostream& operator <<(std::ostream& out, CountryCode code) {
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

  constexpr CountryCode::operator std::uint16_t() const {
    return m_value;
  }

  inline const std::vector<CountryDatabase::Entry>&
      CountryDatabase::get_entries() const {
    return m_entries;
  }

  inline const CountryDatabase::Entry& CountryDatabase::from(
      CountryCode code) const {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(), code,
      [] (const auto& lhs, auto rhs) {
        return lhs.m_code < rhs;
      });
    if(i == m_entries.end() || i->m_code != code) {
      return NONE;
    }
    return *i;
  }

  inline const CountryDatabase::Entry& CountryDatabase::from_name(
      std::string_view name) const {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (const auto& entry) {
        return entry.m_name == name;
      });
    if(i == m_entries.end()) {
      return NONE;
    }
    return *i;
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
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (const auto& entry) {
        return entry.m_two_letter_code == code;
      });
    if(i == m_entries.end()) {
      return NONE;
    }
    return *i;
  }

  inline const CountryDatabase::Entry& CountryDatabase::from(
      ThreeLetterCode code) const {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (const auto& entry) {
        return entry.m_three_letter_code == code;
      });
    if(i == m_entries.end()) {
      return NONE;
    }
    return *i;
  }

  inline void CountryDatabase::add(const Entry& entry) {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(), entry,
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_code < rhs.m_code;
      });
    if(i == m_entries.end() || i->m_code != entry.m_code) {
      m_entries.insert(i, entry);
    }
  }

  inline void CountryDatabase::remove(CountryCode code) {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(), code,
      [] (const auto& lhs, auto rhs) {
        return lhs.m_code < rhs;
      });
    if(i != m_entries.end() && i->m_code == code) {
      m_entries.erase(i);
    }
  }

namespace Details {
  template<typename T>
  const T CountryCodeDefinitions<T>::NONE(~0);
}
}

namespace Beam::Serialization {
  template<>
  struct IsStructure<Nexus::CountryCode> : std::false_type {};

  template<>
  struct Send<Nexus::CountryCode> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::CountryCode value) const {
      shuttle.Send(name, static_cast<std::uint16_t>(value));
    }
  };

  template<>
  struct Receive<Nexus::CountryCode> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::CountryCode& value) const {
      auto representation = std::uint16_t();
      shuttle.Shuttle(name, representation);
      value = Nexus::CountryCode(representation);
    }
  };

  template<>
  struct Shuttle<Nexus::CountryDatabase::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::CountryDatabase::Entry& value,
        unsigned int version) {
      shuttle.Shuttle("code", value.m_code);
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("two_letter_code", value.m_two_letter_code);
      shuttle.Shuttle("three_letter_code", value.m_three_letter_code);
    }
  };

  template<>
  struct Shuttle<Nexus::CountryDatabase> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::CountryDatabase& value,
        unsigned int version) {
      shuttle.Shuttle("entries", value.m_entries);
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
};

#endif
