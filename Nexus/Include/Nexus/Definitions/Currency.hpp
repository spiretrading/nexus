#ifndef NEXUS_CURRENCY_HPP
#define NEXUS_CURRENCY_HPP
#include <algorithm>
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {
namespace Details {
  template<typename T>
  struct CurrencyIdDefinitions {

    /** Stores an invalid id. */
    static const T NONE;

    auto operator <=>(const CurrencyIdDefinitions& rhs) const = default;
  };
}

  /** Stores a currency id, typically the ISO 4217 NUM. */
  class CurrencyId : private Details::CurrencyIdDefinitions<CurrencyId> {
    public:

      /** Constructs an invalid id. */
      constexpr CurrencyId();

      /**
       * Constructs a CurrencyId from its id.
       * @param value The currency id.
       */
      constexpr explicit CurrencyId(std::uint16_t value);

      /** Returns the integral representation of this id. */
      constexpr explicit operator std::uint16_t() const;

      auto operator <=>(const CurrencyId& rhs) const = default;

      using Details::CurrencyIdDefinitions<CurrencyId>::NONE;

    private:
      std::uint16_t m_value;
  };

  /** Stores the database of all Currency. */
  class CurrencyDatabase {
    public:

      /** Stores a single entry in a CurrencyDatabase. */
      struct Entry {

        /** The currency's id. */
        CurrencyId m_id;

        /** The currency's three letter code. */
        Beam::FixedString<3> m_code;

        /** The currency's sign. */
        std::string m_sign;

        bool operator ==(const Entry& rhs) const = default;
      };

      /** Constructs an empty CurrencyDatabase. */
      CurrencyDatabase() = default;

      /** Returns the list of currencies represented. */
      const std::vector<Entry>& GetEntries() const;

      /**
       * Returns an Entry from its CurrencyId.
       * @param id The CurrencyId to lookup.
       * @return The currency Entry with the specified <i>id</i>.
       */
      const Entry& FromId(CurrencyId id) const;

      /**
       * Returns an Entry from its three letter code.
       * @param code The three letter code to lookup.
       * @return The currency Entry with the specified <i>code</i>.
       */
      const Entry& FromCode(Beam::FixedString<3> code) const;

      /**
       * Adds an Entry.
       * @param entry The Entry to add.
       */
      void Add(const Entry& entry);

      /**
       * Deletes an Entry.
       * @param id The CurrencyId of the Entry to delete.
       */
      void Delete(CurrencyId id);

    private:
      friend struct Beam::Serialization::Shuttle<CurrencyDatabase>;
      static Entry MakeNoneEntry();
      template<typename T>
      struct NoneEntry {
        static Entry NONE_ENTRY;
      };
      std::vector<Entry> m_entries;
  };

  /**
   * Parses a CurrencyId from a string.
   * @param source The string to parse.
   * @param currencyDatabase The CurrencyDatabase used to find the CurrencyId.
   * @return The CurrencyId represented by the <i>source</i>.
   */
  inline CurrencyId ParseCurrency(const std::string& source,
      const CurrencyDatabase& currencyDatabase) {
    return currencyDatabase.FromCode(source).m_id;
  }

  /**
   * Parses a CurrencyDatabase Entry from a YAML node.
   * @param node The node to parse the CurrencyDatabase Entry from.
   * @return The CurrencyDatabase Entry represented by the <i>node</i>.
   */
  inline CurrencyDatabase::Entry ParseCurrencyDatabaseEntry(
      const YAML::Node& node) {
    return Beam::TryOrNest([&] {
      auto entry = CurrencyDatabase::Entry();
      entry.m_id = CurrencyId(Beam::Extract<std::uint16_t>(node, "id"));
      entry.m_code = Beam::Extract<std::string>(node, "code");
      entry.m_sign = Beam::Extract<std::string>(node, "sign");
      return entry;
    }, std::runtime_error("Failed to parse currency database entry."));
  }

  /**
   * Parses a CurrencyDatabase from a YAML node.
   * @param node The node to parse the CurrencyDatabase from.
   * @return The CurrencyDatabase represented by the <i>node</i>.
   */
  inline CurrencyDatabase ParseCurrencyDatabase(const YAML::Node& node) {
    return Beam::TryOrNest([&] {
      auto database = CurrencyDatabase();
      for(auto& entryNode : node) {
        database.Add(ParseCurrencyDatabaseEntry(entryNode));
      }
      return database;
    }, std::runtime_error("Failed to parse currency database."));
  }

  inline std::ostream& operator <<(std::ostream& out, CurrencyId value) {
    return out << static_cast<std::uint16_t>(value);
  }

  inline std::istream& operator >>(std::istream& in, CurrencyId& id) {
    auto value = std::uint16_t();
    in >> value;
    id = CurrencyId(value);
    return in;
  }

  inline std::size_t hash_value(CurrencyId code) {
    return static_cast<std::uint16_t>(code);
  }

  constexpr CurrencyId::CurrencyId()
    : CurrencyId(~0) {}

  constexpr CurrencyId::CurrencyId(std::uint16_t value)
    : m_value(value) {}

  constexpr CurrencyId::operator std::uint16_t() const {
    return m_value;
  }

  inline const std::vector<CurrencyDatabase::Entry>&
      CurrencyDatabase::GetEntries() const {
    return m_entries;
  }

  inline const CurrencyDatabase::Entry& CurrencyDatabase::FromId(
      CurrencyId id) const {
    auto comparator = Entry();
    comparator.m_id = id;
    auto entryIterator = std::lower_bound(m_entries.begin(), m_entries.end(),
      comparator,
      [] (auto& lhs, auto& rhs) {
        return lhs.m_id < rhs.m_id;
      });
    if(entryIterator != m_entries.end() && entryIterator->m_id == id) {
      return *entryIterator;
    }
    return NoneEntry<void>::NONE_ENTRY;
  }

  inline const CurrencyDatabase::Entry& CurrencyDatabase::FromCode(
      Beam::FixedString<3> code) const {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (auto& entry) {
        return entry.m_code == code;
      });
    if(i == m_entries.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    return *i;
  }

  inline void CurrencyDatabase::Add(const Entry& entry) {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(),
      entry,
      [] (auto& lhs, auto& rhs) {
        return lhs.m_code < rhs.m_code;
      });
    if(i == m_entries.end() || i->m_id != entry.m_id) {
      m_entries.insert(i, entry);
    }
  }

  inline void CurrencyDatabase::Delete(CurrencyId id) {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [=] (auto& entry) {
        return entry.m_id == id;
      });
    if(i == m_entries.end()) {
      return;
    }
    m_entries.erase(i);
  }

  inline CurrencyDatabase::Entry CurrencyDatabase::MakeNoneEntry() {
    auto noneEntry = Entry();
    noneEntry.m_code = "????";
    noneEntry.m_sign = "?";
    return noneEntry;
  }

  template<typename T>
  CurrencyDatabase::Entry CurrencyDatabase::NoneEntry<T>::NONE_ENTRY =
    CurrencyDatabase::MakeNoneEntry();

namespace Details {
  template<typename T>
  const T CurrencyIdDefinitions<T>::NONE;
}
}

namespace Beam::Serialization {
  template<>
  struct IsStructure<Nexus::CurrencyId> : std::false_type {};

  template<>
  struct Send<Nexus::CurrencyId> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::CurrencyId value) const {
      shuttle.Send(name, static_cast<std::uint16_t>(value));
    }
  };

  template<>
  struct Receive<Nexus::CurrencyId> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::CurrencyId& value) const {
      auto representation = std::uint16_t();
      shuttle.Shuttle(name, representation);
      value = Nexus::CurrencyId(representation);
    }
  };

  template<>
  struct Shuttle<Nexus::CurrencyDatabase::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::CurrencyDatabase::Entry& value,
        unsigned int version) {
      shuttle.Shuttle("id", value.m_id);
      shuttle.Shuttle("code", value.m_code);
      shuttle.Shuttle("sign", value.m_sign);
    }
  };

  template<>
  struct Shuttle<Nexus::CurrencyDatabase> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::CurrencyDatabase& value,
        unsigned int version) {
      shuttle.Shuttle("entries", value.m_entries);
    }
  };
}

namespace std {
  template <>
  struct hash<Nexus::CurrencyId> {
    size_t operator()(Nexus::CurrencyId value) const {
      return Nexus::hash_value(value);
    }
  };
};

#endif
