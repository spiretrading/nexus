#ifndef NEXUS_COMPLIANCE_RULE_ENTRY_HPP
#define NEXUS_COMPLIANCE_RULE_ENTRY_HPP
#include <cstdint>
#include <ostream>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"

namespace Nexus {
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

      /** Constructs an empty ComplianceRuleEntry. */
      ComplianceRuleEntry() noexcept;

      /**
       * Constructs a ComplianceRuleEntry.
       * @param id The entry's id.
       * @param directory_entry The DirectoryEntry this rule is assigned to.
       * @param state The rule's State.
       * @param schema The entry's schema.
       */
      ComplianceRuleEntry(Id id, Beam::DirectoryEntry directory_entry,
        State state, ComplianceRuleSchema schema) noexcept;

      /** Returns the id. */
      Id get_id() const;

      /** Returns the DirectoryEntry this rule is assigned to. */
      const Beam::DirectoryEntry& get_directory_entry() const;

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
      friend struct Beam::DataShuttle;
      friend struct Beam::Shuttle<ComplianceRuleEntry>;
      Id m_id;
      Beam::DirectoryEntry m_directory_entry;
      State m_state;
      ComplianceRuleSchema m_schema;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const ComplianceRuleEntry& entry) {
    return out << '(' << entry.get_id() << ' ' << entry.get_directory_entry() <<
      ' ' << entry.get_state() << ' ' << entry.get_schema() << ')';
  }

  inline ComplianceRuleEntry::ComplianceRuleEntry() noexcept
    : m_id(0),
      m_state(State::ACTIVE) {}

  inline ComplianceRuleEntry::ComplianceRuleEntry(
    Id id, Beam::DirectoryEntry directory_entry, State state,
    ComplianceRuleSchema schema) noexcept
    : m_id(id),
      m_directory_entry(std::move(directory_entry)),
      m_state(state),
      m_schema(std::move(schema)) {}

  inline ComplianceRuleEntry::Id ComplianceRuleEntry::get_id() const {
    return m_id;
  }

  inline const Beam::DirectoryEntry&
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
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ComplianceRuleEntry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::ComplianceRuleEntry& value,
        unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("directory_entry", value.m_directory_entry);
      shuttle.shuttle("state", value.m_state);
      shuttle.shuttle("schema", value.m_schema);
    }
  };
}

#endif
