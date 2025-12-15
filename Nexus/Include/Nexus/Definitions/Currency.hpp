#ifndef NEXUS_CURRENCY_HPP
#define NEXUS_CURRENCY_HPP
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <istream>
#include <memory>
#include <ostream>
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

      /** Tests if this CurrencyId is not equal to NONE. */
      constexpr explicit operator bool() const;

      auto operator <=>(const CurrencyId&) const = default;

    private:
      std::uint16_t m_value;
  };

  inline const CurrencyId CurrencyId::NONE(~0);

  /** Stores a database of currencies. */
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

      CurrencyDatabase(const CurrencyDatabase& database) noexcept;

      /** Returns the list of currencies represented. */
      Beam::View<const Entry> get_entries() const;

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

      CurrencyDatabase& operator =(const CurrencyDatabase& database) noexcept;

    private:
      friend struct Beam::Shuttle<CurrencyDatabase>;
      std::atomic<std::shared_ptr<std::vector<Entry>>> m_entries;
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
   * Parses a CurrencyId from a string.
   * @param source The string to parse.
   * @return The CurrencyId represented by the <i>source</i>.
   */
  inline CurrencyId parse_currency(std::string_view source) {
    extern const CurrencyDatabase& DEFAULT_CURRENCIES;
    return parse_currency(source, DEFAULT_CURRENCIES);
  }

  /**
   * Parses a CurrencyDatabase Entry from a YAML node.
   * @param node The node to parse the CurrencyDatabase Entry from.
   * @return The CurrencyDatabase Entry represented by the <i>node</i>.
   */
  inline CurrencyDatabase::Entry parse_currency_database_entry(
      const YAML::Node& node) {
    return Beam::try_or_nest([&] {
      auto entry = CurrencyDatabase::Entry();
      entry.m_id = CurrencyId(Beam::extract<std::uint16_t>(node, "id"));
      entry.m_code = Beam::extract<std::string>(node, "code");
      entry.m_sign = Beam::extract<std::string>(node, "sign");
      return entry;
    }, std::runtime_error("Failed to parse currency database entry."));
  }

  /**
   * Parses a CurrencyDatabase from a YAML node.
   * @param node The node to parse the CurrencyDatabase from.
   * @return The CurrencyDatabase represented by the <i>node</i>.
   */
  inline CurrencyDatabase parse_currency_database(const YAML::Node& node) {
    return Beam::try_or_nest([&] {
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
    extern const CurrencyDatabase& DEFAULT_CURRENCIES;
    auto database = static_cast<const CurrencyDatabase*>(
      out.pword(Beam::ScopedStreamManipulator<CurrencyDatabase>::ID));
    if(!database) {
      database = &DEFAULT_CURRENCIES;
    }
    auto& entry = database->from(value);
    if(entry.m_id) {
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

  constexpr CurrencyId::operator bool() const {
    return m_value != CurrencyId().m_value;
  }

  inline CurrencyDatabase::CurrencyDatabase(
    const CurrencyDatabase& database) noexcept
    : m_entries(database.m_entries.load()) {}

  inline Beam::View<const CurrencyDatabase::Entry>
      CurrencyDatabase::get_entries() const {
    if(auto entries = m_entries.load()) {
      return Beam::View(Beam::SharedIterator(entries, entries->begin()),
        Beam::SharedIterator(entries, entries->end()));
    }
    return Beam::View<const Entry>();
  }

  inline const CurrencyDatabase::Entry& CurrencyDatabase::from(
      CurrencyId id) const {
    if(auto entries = m_entries.load()) {
      auto i = std::lower_bound(entries->begin(), entries->end(), id,
        [] (const auto& lhs, auto rhs) {
          return lhs.m_id < rhs;
        });
      if(i != entries->end() && i->m_id == id) {
        return *i;
      }
    }
    return NONE;
  }

  inline const CurrencyDatabase::Entry& CurrencyDatabase::from(
      Beam::FixedString<3> code) const {
    if(auto entries = m_entries.load()) {
      auto i = std::find_if(entries->begin(), entries->end(),
        [&] (const auto& entry) {
          return entry.m_code == code;
        });
      if(i != entries->end()) {
        return *i;
      }
    }
    return NONE;
  }

  inline void CurrencyDatabase::add(const Entry& entry) {
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
            return lhs.m_id < rhs.m_id;
          });
        if(i == entries->end() || i->m_id != entry.m_id) {
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

  inline void CurrencyDatabase::remove(CurrencyId id) {
    while(true) {
      auto entries = m_entries.load();
      if(!entries) {
        break;
      }
      auto i = std::lower_bound(entries->begin(), entries->end(), id,
        [] (const auto& lhs, auto rhs) {
          return lhs.m_id < rhs;
        });
      if(i != entries->end() && i->m_id == id) {
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

  inline CurrencyDatabase& CurrencyDatabase::operator =(
      const CurrencyDatabase& database) noexcept {
    m_entries.store(database.m_entries.load());
    return *this;
  }
}

namespace Beam {
  template<>
  constexpr auto is_structure<Nexus::CurrencyId> = false;

  template<>
  struct Send<Nexus::CurrencyId> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, Nexus::CurrencyId value) const {
      sender.send(name, static_cast<std::uint16_t>(value));
    }
  };

  template<>
  struct Receive<Nexus::CurrencyId> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, Nexus::CurrencyId& value) const {
      value = Nexus::CurrencyId(receive<std::uint16_t>(receiver, name));
    }
  };

  template<>
  struct Shuttle<Nexus::CurrencyDatabase::Entry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::CurrencyDatabase::Entry& value,
        unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("code", value.m_code);
      shuttle.shuttle("sign", value.m_sign);
    }
  };

  template<>
  struct Shuttle<Nexus::CurrencyDatabase> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::CurrencyDatabase& value,
        unsigned int version) const {
      if constexpr(IsSender<S>) {
        if(auto entries = value.m_entries.load()) {
          shuttle.send("entries", *entries);
        }
      } else {
        auto entries =
          std::make_shared<std::vector<Nexus::CurrencyDatabase::Entry>>();
        shuttle.shuttle("entries", *entries);
        value.m_entries.store(std::move(entries));
      }
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
}

#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"

#endif
