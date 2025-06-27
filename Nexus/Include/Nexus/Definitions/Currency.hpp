#ifndef NEXUS_CURRENCY_HPP
#define NEXUS_CURRENCY_HPP
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
#include <Beam/Utilities/ScopedStreamManipulator.hpp>
#include <Beam/Utilities/YamlConfig.hpp>

namespace Nexus {

  /** Stores a currency id, typically the ISO 4217 NUM. */
  class CurrencyId {
    public:

      /** Represents an invalid or no currency. */
      static const CurrencyId NONE;

      /** Constructs an invalid id. */
      constexpr CurrencyId() noexcept;

      /**
       * Constructs a CurrencyId from its id.
       * @param value The currency id.
       */
      constexpr explicit CurrencyId(std::uint16_t value) noexcept;

      /** Returns the integral representation of this id. */
      constexpr explicit operator std::uint16_t() const noexcept;

      auto operator <=>(const CurrencyId& rhs) const = default;

    private:
      std::uint16_t m_value;
  };

  inline const CurrencyId CurrencyId::NONE(~0);

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

      /** The entry returned for any failed lookup. */
      inline static const auto NONE = [] {
        auto none = Entry();
        none.m_code = "???";
        none.m_sign = "?";
        return none;
      }();

      /** Constructs an empty CurrencyDatabase. */
      CurrencyDatabase() = default;

      /** Returns the list of currencies represented. */
      const std::vector<Entry>& get_entries() const;

      /**
       * Returns an Entry from its CurrencyId.
       * @param id The CurrencyId to lookup.
       * @return The currency Entry with the specified <i>id</i>.
       */
      const Entry& from(CurrencyId id) const;

      /**
       * Returns an Entry from its three letter code.
       * @param code The three letter code to lookup.
       * @return The currency Entry with the specified <i>code</i>.
       */
      const Entry& from(Beam::FixedString<3> code) const;

      /**
       * Adds an Entry.
       * @param entry The Entry to add.
       */
      void add(const Entry& entry);

      /**
       * Removes an Entry.
       * @param id The CurrencyId of the Entry to delete.
       */
      void remove(CurrencyId id);

    private:
      friend struct Beam::Serialization::Shuttle<CurrencyDatabase>;

      std::vector<Entry> m_entries;
  };

  /**
   * Parses a CurrencyId from a string.
   * @param source The string to parse.
   * @param database The CurrencyDatabase used to find the CurrencyId.
   * @return The CurrencyId represented by the <i>source</i>.
   */
  inline CurrencyId parse_currency(
      std::string_view source, const CurrencyDatabase& database) {
    return database.from(source).m_id;
  }

  /**
   * Parses a CurrencyDatabase Entry from a YAML node.
   * @param node The node to parse the CurrencyDatabase Entry from.
   * @return The CurrencyDatabase Entry represented by the <i>node</i>.
   */
  inline CurrencyDatabase::Entry parse_currency_database_entry(
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
  inline CurrencyDatabase parse_currency_database(const YAML::Node& node) {
    return Beam::TryOrNest([&] {
      auto database = CurrencyDatabase();
      for(auto& entry : node) {
        database.add(parse_currency_database_entry(entry));
      }
      return database;
    }, std::runtime_error("Failed to parse currency database."));
  }

  inline auto operator <<(std::ostream& out, const CurrencyDatabase& database) {
    return Beam::ScopedStreamManipulator(out, database);
  }

  inline std::ostream& operator <<(std::ostream& out, CurrencyId value) {
    extern const CurrencyDatabase DEFAULT_CURRENCIES;
    auto database = static_cast<const CurrencyDatabase*>(
      out.pword(Beam::ScopedStreamManipulator<CurrencyDatabase>::ID));
    if(!database) {
      database = &DEFAULT_CURRENCIES;
    }
    auto& entry = database->from(value);
    if(entry.m_id != CurrencyId::NONE) {
      return out << entry.m_code;
    }
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

  constexpr CurrencyId::CurrencyId() noexcept
    : CurrencyId(~0) {}

  constexpr CurrencyId::CurrencyId(std::uint16_t value) noexcept
    : m_value(value) {}

  constexpr CurrencyId::operator std::uint16_t() const noexcept {
    return m_value;
  }

  inline const std::vector<CurrencyDatabase::Entry>&
      CurrencyDatabase::get_entries() const {
    return m_entries;
  }

  inline const CurrencyDatabase::Entry& CurrencyDatabase::from(
      CurrencyId id) const {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(), id,
      [] (const auto& lhs, auto rhs) {
        return lhs.m_id < rhs;
      });
    if(i != m_entries.end() && i->m_id == id) {
      return *i;
    }
    return NONE;
  }

  inline const CurrencyDatabase::Entry& CurrencyDatabase::from(
      Beam::FixedString<3> code) const {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (const auto& entry) {
        return entry.m_code == code;
      });
    if(i == m_entries.end()) {
      return NONE;
    }
    return *i;
  }

  inline void CurrencyDatabase::add(const Entry& entry) {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(), entry,
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_id < rhs.m_id;
      });
    if(i == m_entries.end() || i->m_id != entry.m_id) {
      m_entries.insert(i, entry);
    }
  }

  inline void CurrencyDatabase::remove(CurrencyId id) {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(), id,
      [] (const auto& lhs, auto rhs) {
        return lhs.m_id < rhs;
      });
    if(i == m_entries.end() || i->m_id != id) {
      return;
    }
    m_entries.erase(i);
  }
}

namespace Beam::Serialization {
  template<>
  struct IsStructure<Nexus::CurrencyId> : std::false_type {};

  template<>
  struct Send<Nexus::CurrencyId> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, const char* name, Nexus::CurrencyId value) const {
      shuttle.Send(name, static_cast<std::uint16_t>(value));
    }
  };

  template<>
  struct Receive<Nexus::CurrencyId> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, const char* name, Nexus::CurrencyId& value) const {
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
