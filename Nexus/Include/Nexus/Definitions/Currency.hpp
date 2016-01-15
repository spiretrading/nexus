#ifndef NEXUS_CURRENCY_HPP
#define NEXUS_CURRENCY_HPP
#include <algorithm>
#include <string>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  /*! \struct CurrencyId
      \brief Stores a currency id, typically the ISO 4217 NUM.
   */
  struct CurrencyId {

    //! Returns an empty CurrencyId.
    static CurrencyId NONE();

    //! The currency's id.
    std::uint16_t m_value;

    //! Constructs a NONE CurrencyId.
    CurrencyId();

    //! Constructs a CurrencyId.
    /*!
      \param value The currency's NUM value.
    */
    explicit CurrencyId(std::uint16_t value);

    //! Constructs a CurrencyId.
    /*!
      \param value The currency's NUM value.
    */
    explicit CurrencyId(int value);
  };

  /*! \class CurrencyDatabase
      \brief Stores the database of all Currency.
   */
  class CurrencyDatabase {
    public:

      /*! \struct Entry
          \brief Stores a single entry in a CurrencyDatabase.
       */
      struct Entry {

        //! The currency's numeric code.
        CurrencyId m_id;

        //! The currency's three letter code.
        Beam::FixedString<3> m_code;

        //! The currency's sign.
        std::string m_sign;
      };

      //! Constructs an empty CurrencyDatabase.
      CurrencyDatabase() = default;

      //! Returns the list of currencies represented.
      const std::vector<Entry>& GetEntries() const;

      //! Returns an Entry from its CurrencyId.
      /*!
        \param id The CurrencyId to lookup.
        \return The currency Entry with the specified <i>id</i>.
      */
      const Entry& FromId(CurrencyId id) const;

      //! Returns an Entry from its three letter code.
      /*!
        \param code The three letter code to lookup.
        \return The currency Entry with the specified <i>code</i>.
      */
      const Entry& FromCode(Beam::FixedString<3> code) const;

      //! Adds an Entry.
      /*!
        \param entry The Entry to add.
      */
      void Add(const Entry& entry);

      //! Deletes an Entry.
      /*!
        \param id The CurrencyId of the Entry to delete.
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

  //! Tests if two CurrencyIds are equal.
  /*!
    \param lhs The left hand side of the comparison.
    \param rhs The right hand side of the comparison.
    \return <code>true</code> iff the two CurrencyId's are equal.
  */
  inline bool operator ==(CurrencyId lhs, CurrencyId rhs) {
    return lhs.m_value == rhs.m_value;
  }

  //! Tests if two CurrencyIds are not equal.
  /*!
    \param lhs The left hand side of the comparison.
    \param rhs The right hand side of the comparison.
    \return <code>true</code> iff the two CurrencyId's are not equal.
  */
  inline bool operator !=(CurrencyId lhs, CurrencyId rhs) {
    return !(lhs.m_value == rhs.m_value);
  }

  //! Tests if a CurrencyId's value is than another.
  /*!
    \param lhs The left hand side of the comparison.
    \param rhs The right hand side of the comparison.
    \return <code>true</code> iff <i>lhs</i>'s value is less than
            <i>rhs</i>'s value.
  */
  inline bool operator <(CurrencyId lhs, CurrencyId rhs) {
    return lhs.m_value < rhs.m_value;
  }

  //! Tests two CurrencyDatabase Entries for equality.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff the two CurrencyDatabase Entries are equal.
  */
  inline bool operator ==(const CurrencyDatabase::Entry& lhs,
      const CurrencyDatabase::Entry& rhs) {
    return lhs.m_id == rhs.m_id && lhs.m_code == rhs.m_code &&
      lhs.m_sign == rhs.m_sign;
  }

  //! Tests two CurrencyDatabase Entries for equality.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff the two CurrencyDatabase Entries are equal.
  */
  inline bool operator !=(const CurrencyDatabase::Entry& lhs,
      const CurrencyDatabase::Entry& rhs) {
    return !(lhs == rhs);
  }

  //! Parses a CurrencyId from a string.
  /*!
    \param source The string to parse.
    \param currencyDatabase The CurrencyDatabase used to find the CurrencyId.
    \return The CurrencyId represented by the <i>source</i>.
  */
  inline CurrencyId ParseCurrency(const std::string& source,
      const CurrencyDatabase& currencyDatabase) {
    return currencyDatabase.FromCode(source).m_id;
  }

  //! Parses a CurrencyDatabase Entry from a YAML node.
  /*!
    \param node The node to parse the CurrencyDatabase Entry from.
    \return The CurrencyDatabase Entry represented by the <i>node</i>.
  */
  inline CurrencyDatabase::Entry ParseCurrencyDatabaseEntry(
      const YAML::Node& node) {
    CurrencyDatabase::Entry entry;
    entry.m_id = CurrencyId{Beam::Extract<int>(node, "id")};
    entry.m_code = Beam::Extract<std::string>(node, "code");
    entry.m_sign = Beam::Extract<std::string>(node, "sign");
    return entry;
  }

  //! Parses a CurrencyDatabase from a YAML node.
  /*!
    \param node The node to parse the CurrencyDatabase from.
    \return The CurrencyDatabase represented by the <i>node</i>.
  */
  inline CurrencyDatabase ParseCurrencyDatabase(const YAML::Node& node) {
    CurrencyDatabase currencyDatabase;
    for(auto& entryNode : node) {
      auto entry = ParseCurrencyDatabaseEntry(entryNode);
      currencyDatabase.Add(entry);
    }
    return currencyDatabase;
  }

  inline CurrencyId CurrencyId::NONE() {
    return CurrencyId{};
  }

  inline CurrencyId::CurrencyId()
      : m_value{-0} {}

  inline CurrencyId::CurrencyId(std::uint16_t value)
      : m_value{value} {}

  inline CurrencyId::CurrencyId(int value)
      : m_value{static_cast<std::uint16_t>(value)} {}

  inline const std::vector<CurrencyDatabase::Entry>&
      CurrencyDatabase::GetEntries() const {
    return m_entries;
  }

  inline const CurrencyDatabase::Entry& CurrencyDatabase::FromId(
      CurrencyId id) const {
    Entry comparator;
    comparator.m_id = id;
    auto entryIterator = std::lower_bound(m_entries.begin(), m_entries.end(),
      comparator,
      [] (const Entry& lhs, const Entry& rhs) {
        return lhs.m_id < rhs.m_id;
      });
    if(entryIterator != m_entries.end() && entryIterator->m_id == id) {
      return *entryIterator;
    }
    return NoneEntry<void>::NONE_ENTRY;
  }

  inline const CurrencyDatabase::Entry& CurrencyDatabase::FromCode(
      Beam::FixedString<3> code) const {
    for(auto i = m_entries.begin(); i!= m_entries.end(); ++i) {
      if(i->m_code == code) {
        return *i;
      }
    }
    return NoneEntry<void>::NONE_ENTRY;
  }

  inline void CurrencyDatabase::Add(const Entry& entry) {
    auto entryIterator = std::lower_bound(m_entries.begin(), m_entries.end(),
      entry,
      [] (const Entry& lhs, const Entry& rhs) {
        return lhs.m_code < rhs.m_code;
      });
    if(entryIterator == m_entries.end() || entryIterator->m_id != entry.m_id) {
      m_entries.insert(entryIterator, entry);
    }
  }

  inline void CurrencyDatabase::Delete(CurrencyId id) {
    auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
      [=] (const Entry& entry) {
        return entry.m_id == id;
      });
    if(entryIterator == m_entries.end()) {
      return;
    }
    m_entries.erase(entryIterator);
  }

  inline CurrencyDatabase::Entry CurrencyDatabase::MakeNoneEntry() {
    Entry noneEntry;
    noneEntry.m_code = "????";
    noneEntry.m_sign = "?";
    return noneEntry;
  }

  template<typename T>
  CurrencyDatabase::Entry CurrencyDatabase::NoneEntry<T>::NONE_ENTRY =
    CurrencyDatabase::MakeNoneEntry();

  inline std::size_t hash_value(CurrencyId id) {
    return id.m_value;
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsStructure<Nexus::CurrencyId> : std::false_type {};

  template<>
  struct Send<Nexus::CurrencyId> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::CurrencyId value) const {
      shuttle.Send(name, value.m_value);
    }
  };

  template<>
  struct Receive<Nexus::CurrencyId> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::CurrencyId& value) const {
      shuttle.Shuttle(name, value.m_value);
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
