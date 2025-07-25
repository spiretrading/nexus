#ifndef NEXUS_COMPLIANCE_RULE_ENTRY_HPP
#define NEXUS_COMPLIANCE_RULE_ENTRY_HPP
#include <cstdint>
#include <ostream>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"

namespace Nexus::Compliance {
namespace Details {
  BEAM_ENUM(ComplianceRuleEntryStateDefinition,

    /** The rule will reject operations that fail the compliance check. */
    ACTIVE,

    /** The rule will log operations that fail the compliance check. */
    PASSIVE,

    /** The rule performs no validation. */
    DISABLED,

    /** The rule has been deleted. */
    DELETED);

  inline std::ostream& operator <<(
      std::ostream& out, ComplianceRuleEntryStateDefinition::Type state) {
    if(state == ComplianceRuleEntryStateDefinition::ACTIVE) {
      return out << "ACTIVE";
    } else if(state == ComplianceRuleEntryStateDefinition::PASSIVE) {
      return out << "PASSIVE";
    } else if(state == ComplianceRuleEntryStateDefinition::DISABLED) {
      return out << "DISABLED";
    } else if(state == ComplianceRuleEntryStateDefinition::DELETED) {
      return out << "DELETED";
    } else {
      return out << "UNKNOWN";
    }
  }
}

  /** Represents a single instance of a compliance rule. */
  class ComplianceRuleEntry {
    public:

      /** Type used to identify a rule. */
      using Id = std::uint64_t;

      /**
       * Specifies how this rule handles operations that fail to pass this
       * check.
       */
      using State = Details::ComplianceRuleEntryStateDefinition;

      /**
       * Constructs a ComplianceRuleEntry.
       * @param id The entry's id.
       * @param directory_entry The DirectoryEntry this rule is assigned to.
       * @param state The rule's State.
       * @param schema The entry's schema.
       */
      ComplianceRuleEntry(
        Id id, Beam::ServiceLocator::DirectoryEntry directory_entry,
        State state, ComplianceRuleSchema schema) noexcept;

      /** Returns the id. */
      Id get_id() const;

      /** Returns the DirectoryEntry this rule is assigned to. */
      const Beam::ServiceLocator::DirectoryEntry& get_directory_entry() const;

      /** Returns the State. */
      State get_state() const;

      /**
       * Sets the State.
       * @param state This entry's new State.
       */
      void set_state(State state);

      /** Returns the schema. */
      const ComplianceRuleSchema& get_schema() const;

      bool operator ==(const ComplianceRuleEntry&) const = default;

    private:
      friend struct Beam::Serialization::DataShuttle;
      friend struct Beam::Serialization::Shuttle<ComplianceRuleEntry>;
      Id m_id;
      Beam::ServiceLocator::DirectoryEntry m_directory_entry;
      State m_state;
      ComplianceRuleSchema m_schema;

      ComplianceRuleEntry(Beam::Serialization::ReceiveBuilder);
  };

  inline std::ostream& operator <<(
      std::ostream& out, ComplianceRuleEntry::State state) {
    return out << static_cast<ComplianceRuleEntry::State::Type>(state);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const ComplianceRuleEntry& entry) {
    return out << '(' << entry.get_id() << ' ' << entry.get_directory_entry() <<
      ' ' << entry.get_state() << ' ' << entry.get_schema() << ')';
  }

  inline ComplianceRuleEntry::ComplianceRuleEntry(Id id,
    Beam::ServiceLocator::DirectoryEntry directory_entry, State state,
    ComplianceRuleSchema schema) noexcept
    : m_id(id),
      m_directory_entry(std::move(directory_entry)),
      m_state(state),
      m_schema(std::move(schema)) {}

  inline ComplianceRuleEntry::Id ComplianceRuleEntry::get_id() const {
    return m_id;
  }

  inline const Beam::ServiceLocator::DirectoryEntry&
      ComplianceRuleEntry::get_directory_entry() const {
    return m_directory_entry;
  }

  inline ComplianceRuleEntry::State ComplianceRuleEntry::get_state() const {
    return m_state;
  }

  inline void ComplianceRuleEntry::set_state(State state) {
    m_state = state;
  }

  inline const ComplianceRuleSchema& ComplianceRuleEntry::get_schema() const {
    return m_schema;
  }

  inline ComplianceRuleEntry::ComplianceRuleEntry(
    Beam::Serialization::ReceiveBuilder) {}
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::Compliance::ComplianceRuleEntry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Compliance::ComplianceRuleEntry& value,
        unsigned int version) const {
      shuttle.Shuttle("id", value.m_id);
      shuttle.Shuttle("directory_entry", value.m_directory_entry);
      shuttle.Shuttle("state", value.m_state);
      shuttle.Shuttle("schema", value.m_schema);
    }
  };
}

#endif
