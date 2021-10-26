#ifndef NEXUS_ACCOUNT_TELEMETRY_EVENT_ENTRY_HPP
#define NEXUS_ACCOUNT_TELEMETRY_EVENT_ENTRY_HPP
#include <atomic>
#include <utility>
#include <Beam/Queries/Sequence.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/TelemetryService/AccountQuery.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {

  /** Keeps and updates a registry of TelemetryEvents. */
  class AccountTelemetryEventEntry {
    public:

      /**
       * Constructs an AccountTelemetryEventEntry.
       * @param account The account.
       * @param initialSequence The initial Sequence to use.
       */
      AccountTelemetryEventEntry(Beam::ServiceLocator::DirectoryEntry account,
        Beam::Queries::Sequence initialSequence);

      /**
       * Publishes a TelemetryEvent.
       * @param event The TelemetryEvent to publish.
       */
      SequencedAccountTelemetryEvent Publish(const TelemetryEvent& event);

    private:
      Beam::ServiceLocator::DirectoryEntry m_account;
      std::atomic<Beam::Queries::Sequence::Ordinal> m_sequence;

      AccountTelemetryEventEntry(const AccountTelemetryEventEntry&) = delete;
      AccountTelemetryEventEntry& operator =(
        const AccountTelemetryEventEntry&) = delete;
  };

  /**
   * Returns the initial Sequence for an AccountTelemetryEventEntry.
   * @param dataStore The DataStore to load the initial Sequence from.
   * @param account The account to load the initial Sequence for.
   * @return The initial Sequence to use for the specified <i>account</i>.
   */
  template<typename DataStore>
  Beam::Queries::Sequence LoadInitialSequences(DataStore& dataStore,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto query = Beam::Queries::MakeLatestQuery(account);
    auto results = dataStore.LoadTelemetryEvents(query);
    if(results.empty()) {
      return Beam::Queries::Sequence::First();
    }
    return Beam::Queries::Increment(results.back().GetSequence());
  }

  inline AccountTelemetryEventEntry::AccountTelemetryEventEntry(
    Beam::ServiceLocator::DirectoryEntry account,
    Beam::Queries::Sequence initialSequence)
    : m_account(std::move(account)),
      m_sequence(initialSequence.GetOrdinal()) {}

  inline SequencedAccountTelemetryEvent AccountTelemetryEventEntry::Publish(
      const TelemetryEvent& event) {
    auto sequence = ++m_sequence;
    auto sequencedEvent = Beam::Queries::SequencedValue(
      Beam::Queries::IndexedValue(event, m_account),
      Beam::Queries::Sequence(sequence));
    return sequencedEvent;
  }
}

#endif
