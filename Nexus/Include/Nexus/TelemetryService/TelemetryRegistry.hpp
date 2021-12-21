#ifndef NEXUS_TELEMETRY_SERVICE_TELEMETRY_REGISTRY_HPP
#define NEXUS_TELEMETRY_SERVICE_TELEMETRY_REGISTRY_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include "Nexus/TelemetryService/AccountTelemetryEventEntry.hpp"
#include "Nexus/TelemetryService/AccountQuery.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {

  /** Keeps and updates a registry of telemetry events. */
  class TelemetryRegistry {
    public:

      /** Constructs a TelemetryRegistry. */
      TelemetryRegistry() = default;

      /**
       * Publishes a TelemetryEvent.
       * @param event The TelemetryEvent to publish.
       * @param initialSequenceLoader Loads initial Sequences for the account
       *        that submitted the event.
       * @param f Receives synchronized access to the TelemetryEvent.
       */
      template<typename InitialSequenceLoader, typename F>
      void Publish(const AccountTelemetryEvent& event,
        const InitialSequenceLoader& initialSequenceLoader, F&& f);

    private:
      using SyncAccountTelemetryEventEntry = Beam::Threading::Sync<
        AccountTelemetryEventEntry, Beam::Threading::Mutex>;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<SyncAccountTelemetryEventEntry>, Beam::Threading::Mutex>
        m_telemetryEventEntries;

      TelemetryRegistry(const TelemetryRegistry&) = delete;
      TelemetryRegistry& operator =(const TelemetryRegistry&) = delete;
  };

  template<typename InitialSequenceLoader, typename F>
  void TelemetryRegistry::Publish(const AccountTelemetryEvent& event,
      const InitialSequenceLoader& initialSequenceLoader, F&& f) {
    auto entry = m_telemetryEventEntries.GetOrInsert(event.GetIndex(),
      [&] {
        auto initialSequences = initialSequenceLoader();
        return std::make_shared<SyncAccountTelemetryEventEntry>(
          event.GetIndex(), initialSequences);
      });
    Beam::Threading::With(*entry, [&] (auto& entry) {
      auto sequencedEvent = entry.Publish(event);
      std::forward<F>(f)(sequencedEvent);
    });
  }
}

#endif
