#ifndef NEXUS_COMPLIANCE_RULE_ENTRY_HPP
#define NEXUS_COMPLIANCE_RULE_ENTRY_HPP
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Compliance/Compliance.hpp"
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
}

  /** Represents a single instance of a compliance rule. */
  class ComplianceRuleEntry {
    public:

      /**
       * Specifies how this rule handles operations that fail to pass this
       * check.
       */
      using State = Details::ComplianceRuleEntryStateDefinition;

      /** Constructs an empty ComplianceRuleEntry. */
      ComplianceRuleEntry();

      /**
       * Constructs a ComplianceRuleEntry.
       * @param id The entry's id.
       * @param directoryEntry The DirectoryEntry this rule is assigned to.
       * @param state The rule's State.
       * @param schema The entry's schema.
       */
      ComplianceRuleEntry(ComplianceRuleId id,
        Beam::ServiceLocator::DirectoryEntry directoryEntry, State state,
        ComplianceRuleSchema schema);

      /** Returns the id. */
      ComplianceRuleId GetId() const;

      /**
       * Sets the id.
       * @param id This entry's new id.
       */
      void SetId(const ComplianceRuleId& id);

      /** Returns the DirectoryEntry this rule is assigned to. */
      const Beam::ServiceLocator::DirectoryEntry& GetDirectoryEntry() const;

      /** Returns the State. */
      State GetState() const;

      /**
       * Sets the State.
       * @param state This entry's new State.
       */
      void SetState(State state);

      /** Returns the schema. */
      const ComplianceRuleSchema& GetSchema() const;

      bool operator ==(const ComplianceRuleEntry& rhs) const = default;

    private:
      friend struct Beam::Serialization::Shuttle<ComplianceRuleEntry>;
      ComplianceRuleId m_id;
      Beam::ServiceLocator::DirectoryEntry m_directoryEntry;
      State m_state;
      ComplianceRuleSchema m_schema;
  };

  inline ComplianceRuleEntry::ComplianceRuleEntry()
    : m_id(0),
      m_state(State::ACTIVE) {}

  inline ComplianceRuleEntry::ComplianceRuleEntry(ComplianceRuleId id,
    Beam::ServiceLocator::DirectoryEntry directoryEntry, State state,
    ComplianceRuleSchema schema)
    : m_id(std::move(id)),
      m_directoryEntry(std::move(directoryEntry)),
      m_state(state),
      m_schema(std::move(schema)) {}

  inline ComplianceRuleId ComplianceRuleEntry::GetId() const {
    return m_id;
  }

  inline void ComplianceRuleEntry::SetId(const ComplianceRuleId& id) {
    m_id = id;
  }

  inline const Beam::ServiceLocator::DirectoryEntry& ComplianceRuleEntry::
      GetDirectoryEntry() const {
    return m_directoryEntry;
  }

  inline ComplianceRuleEntry::State ComplianceRuleEntry::GetState() const {
    return m_state;
  }

  inline void ComplianceRuleEntry::SetState(State state) {
    m_state = state;
  }

  inline const ComplianceRuleSchema& ComplianceRuleEntry::GetSchema() const {
    return m_schema;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::Compliance::ComplianceRuleEntry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Compliance::ComplianceRuleEntry& value, unsigned int version) {
      shuttle.Shuttle("id", value.m_id);
      shuttle.Shuttle("directory_entry", value.m_directoryEntry);
      shuttle.Shuttle("state", value.m_state);
      shuttle.Shuttle("schema", value.m_schema);
    }
  };
}

#endif
