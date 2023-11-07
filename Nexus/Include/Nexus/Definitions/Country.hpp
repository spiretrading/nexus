#ifndef NEXUS_COUNTRY_HPP
#define NEXUS_COUNTRY_HPP
#include <algorithm>
#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Definitions/Definitions.hpp"

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
      constexpr CountryCode();

      /**
       * Constructs a CountryCode from its ISO code.
       * @param value The country's ISO code.
       */
      constexpr explicit CountryCode(std::uint16_t value);

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

      /** Stores a single entry in a CountryDatabase. */
      struct Entry {

        /** The country's code. */
        CountryCode m_code;

        /** The short form name. */
        std::string m_name;

        /** The country's two letter code. */
        Beam::FixedString<2> m_twoLetterCode;

        /** The country's three letter code. */
        Beam::FixedString<3> m_threeLetterCode;

        bool operator ==(const Entry& rhs) const = default;
      };

      /** Constructs an empty CountryDatabase. */
      CountryDatabase() = default;

      /** Returns the list of countries represented. */
      const std::vector<Entry>& GetEntries() const;

      /** Returns the country represented by its numeric code. */
      const Entry& FromCode(CountryCode code) const;

      /** Returns the country represented by its name. */
      const Entry& FromName(const std::string& name) const;

      /** Returns the country represented by its two letter code. */
      const Entry& FromTwoLetterCode(const Beam::FixedString<2>& code) const;

      /** Returns the country represented by its three letter code. */
      const Entry& FromThreeLetterCode(const Beam::FixedString<3>& code) const;

      /**
       * Adds an Entry.
       * @param entry The Entry to add.
       */
      void Add(const Entry& entry);

      /**
       * Deletes an Entry.
       * @param code The CountryCode of the Entry to delete.
       */
      void Delete(CountryCode code);

    private:
      friend struct Beam::Serialization::Shuttle<CountryDatabase>;
      static Entry MakeNoneEntry();
      template<typename T>
      struct NoneEntry {
        static Entry NONE_ENTRY;
      };
      std::vector<Entry> m_entries;
  };

  /**
   * Parses a CountryCode from a string.
   * @param source The string to parse.
   * @param countryDatabase The CountryDatabase used to find the CountryCode.
   * @return The CountryCode represented by the <i>source</i>.
   */
  inline CountryCode ParseCountryCode(const std::string& source,
      const CountryDatabase& countryDatabase) {
    auto code = countryDatabase.FromTwoLetterCode(source);
    if(code.m_code != CountryCode::NONE) {
      return code.m_code;
    }
    code = countryDatabase.FromThreeLetterCode(source);
    if(code.m_code != CountryCode::NONE) {
      return code.m_code;
    }
    return countryDatabase.FromName(source).m_code;
  }

  /**
   * Parses a CountryDatabase Entry from a YAML node.
   * @param node The node to parse the CountryDatabase Entry from.
   * @return The CountryDatabase Entry represented by the <i>node</i>.
   */
  inline CountryDatabase::Entry ParseCountryDatabaseEntry(
      const YAML::Node& node) {
    return Beam::TryOrNest([&] {
      auto entry = CountryDatabase::Entry();
      entry.m_name = Beam::Extract<std::string>(node, "name");
      entry.m_twoLetterCode = Beam::Extract<std::string>(node,
        "two_letter_code");
      entry.m_threeLetterCode = Beam::Extract<std::string>(node,
        "three_letter_code");
      entry.m_code = Beam::Extract<CountryCode>(node, "code");
      return entry;
    }, std::runtime_error("Failed to parse country database entry."));
  }

  /**
   * Parses a CountryDatabase from a YAML node.
   * @param node The node to parse the CountryDatabase from.
   * @return The CountryDatabase represented by the <i>node</i>.
   */
  inline CountryDatabase ParseCountryDatabase(const YAML::Node& node) {
    return Beam::TryOrNest([&] {
      auto database = CountryDatabase();
      for(auto& entryNode : node) {
        database.Add(ParseCountryDatabaseEntry(entryNode));
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

  constexpr CountryCode::CountryCode()
    : CountryCode(~0) {}

  constexpr CountryCode::CountryCode(std::uint16_t value)
    : m_value(value) {}

  constexpr CountryCode::operator std::uint16_t() const {
    return m_value;
  }

  inline const std::vector<CountryDatabase::Entry>&
      CountryDatabase::GetEntries() const {
    return m_entries;
  }

  inline const CountryDatabase::Entry& CountryDatabase::FromCode(
      CountryCode code) const {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (auto& entry) {
        return entry.m_code == code;
      });
    if(i == m_entries.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    return *i;
  }

  inline const CountryDatabase::Entry& CountryDatabase::FromName(
      const std::string& name) const {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (auto& entry) {
        return entry.m_name == name;
      });
    if(i == m_entries.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    return *i;
  }

  inline const CountryDatabase::Entry& CountryDatabase::FromTwoLetterCode(
      const Beam::FixedString<2>& code) const {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (auto& entry) {
        return entry.m_twoLetterCode == code;
      });
    if(i == m_entries.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    return *i;
  }

  inline const CountryDatabase::Entry& CountryDatabase::FromThreeLetterCode(
      const Beam::FixedString<3>& code) const {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (auto& entry) {
        return entry.m_threeLetterCode == code;
      });
    if(i == m_entries.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    return *i;
  }

  inline void CountryDatabase::Add(const Entry& entry) {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(),
      entry,
      [] (auto& lhs, auto& rhs) {
        return lhs.m_code < rhs.m_code;
      });
    if(i == m_entries.end() || i->m_code != entry.m_code) {
      m_entries.insert(i, entry);
    }
  }

  inline void CountryDatabase::Delete(CountryCode code) {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (auto& entry) {
        return entry.m_code == code;
      });
    if(i == m_entries.end()) {
      return;
    }
    m_entries.erase(i);
  }

  inline CountryDatabase::Entry CountryDatabase::MakeNoneEntry() {
    auto noneEntry = Entry();
    noneEntry.m_twoLetterCode = "??";
    noneEntry.m_threeLetterCode = "???";
    noneEntry.m_name = "None";
    noneEntry.m_code = CountryCode::NONE;
    return noneEntry;
  }

  template<typename T>
  CountryDatabase::Entry CountryDatabase::NoneEntry<T>::NONE_ENTRY =
    CountryDatabase::MakeNoneEntry();

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
      shuttle.Shuttle("two_letter_code", value.m_twoLetterCode);
      shuttle.Shuttle("three_letter_code", value.m_threeLetterCode);
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
