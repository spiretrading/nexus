#ifndef NEXUS_TELEMETRY_DATA_STORE_BOX_HPP
#define NEXUS_TELEMETRY_DATA_STORE_BOX_HPP
#include <memory>
#include <type_traits>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/TelemetryService/TelemetryDataStore.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {

  /** Provides a generic interface over an arbitrary TelemetryDataStore. */
  class TelemetryDataStoreBox {
    public:

      /**
       * Constructs a TelemetryDataStoreBox of a specified type using
       * emplacement.
       * @param <T> The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<typename T, typename... Args>
      explicit TelemetryDataStoreBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an TelemetryDataStoreBox by copying an existing data store.
       * @param client The client to copy.
       */
      template<typename DataStore>
      explicit TelemetryDataStoreBox(DataStore dataStore);

      explicit TelemetryDataStoreBox(TelemetryDataStoreBox* dataStore);

      explicit TelemetryDataStoreBox(
        const std::shared_ptr<TelemetryDataStoreBox>& dataStore);

      explicit TelemetryDataStoreBox(
        const std::unique_ptr<TelemetryDataStoreBox>& dataStore);

      std::vector<SequencedTelemetryEvent> LoadTelemetryEvents(
        const AccountQuery& query);

      void Store(const SequencedAccountTelemetryEvent& event);

      void Store(const std::vector<SequencedAccountTelemetryEvent>& events);

      void Close();

    private:
      struct VirtualTelemetryDataStore {
        virtual ~VirtualTelemetryDataStore() = default;
        virtual std::vector<SequencedTelemetryEvent> LoadTelemetryEvents(
          const AccountQuery& query) = 0;
        virtual void Store(const SequencedAccountTelemetryEvent& event) = 0;
        virtual void Store(
          const std::vector<SequencedAccountTelemetryEvent>& events) = 0;
        virtual void Close() = 0;
      };
      template<typename D>
      struct WrappedTelemetryDataStore final : VirtualTelemetryDataStore {
        using TelemetryDataStore = D;
        Beam::GetOptionalLocalPtr<TelemetryDataStore> m_dataStore;

        template<typename... Args>
        WrappedTelemetryDataStore(Args&&... args);
        std::vector<SequencedTelemetryEvent> LoadTelemetryEvents(
          const AccountQuery& query) override;
        void Store(const SequencedAccountTelemetryEvent& event) override;
        void Store(const std::vector<SequencedAccountTelemetryEvent>& events)
          override;
        void Close() override;
      };
      std::shared_ptr<VirtualTelemetryDataStore> m_dataStore;
  };

  template<typename T, typename... Args>
  TelemetryDataStoreBox::TelemetryDataStoreBox(
    std::in_place_type_t<T>, Args&&... args)
    : m_dataStore(std::make_shared<WrappedTelemetryDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<typename DataStore>
  TelemetryDataStoreBox::TelemetryDataStoreBox(DataStore dataStore)
    : TelemetryDataStoreBox(std::in_place_type<DataStore>,
        std::move(dataStore)) {}

  inline TelemetryDataStoreBox::TelemetryDataStoreBox(
    TelemetryDataStoreBox* dataStore)
    : TelemetryDataStoreBox(*dataStore) {}

  inline TelemetryDataStoreBox::TelemetryDataStoreBox(
    const std::shared_ptr<TelemetryDataStoreBox>& dataStore)
    : TelemetryDataStoreBox(*dataStore) {}

  inline TelemetryDataStoreBox::TelemetryDataStoreBox(
    const std::unique_ptr<TelemetryDataStoreBox>& dataStore)
    : TelemetryDataStoreBox(*dataStore) {}

  inline std::vector<SequencedTelemetryEvent>
      TelemetryDataStoreBox::LoadTelemetryEvents(const AccountQuery& query) {
    return m_dataStore->LoadTelemetryEvents(query);
  }

  inline void TelemetryDataStoreBox::Store(
      const SequencedAccountTelemetryEvent& event) {
    m_dataStore->Store(event);
  }

  inline void TelemetryDataStoreBox::Store(
      const std::vector<SequencedAccountTelemetryEvent>& events) {
    m_dataStore->Store(events);
  }

  inline void TelemetryDataStoreBox::Close() {
    m_dataStore->Close();
  }

  template<typename D>
  template<typename... Args>
  TelemetryDataStoreBox::WrappedTelemetryDataStore<D>::
    WrappedTelemetryDataStore(Args&&... args)
    : m_dataStore(std::forward<Args>(args)...) {}

  template<typename D>
  std::vector<SequencedTelemetryEvent>
      TelemetryDataStoreBox::WrappedTelemetryDataStore<D>::LoadTelemetryEvents(
        const AccountQuery& query) {
    return m_dataStore->LoadTelemetryEvents(query);
  }

  template<typename D>
  void TelemetryDataStoreBox::WrappedTelemetryDataStore<D>::Store(
      const SequencedAccountTelemetryEvent& event) {
    m_dataStore->Store(event);
  }

  template<typename D>
  void TelemetryDataStoreBox::WrappedTelemetryDataStore<D>::Store(
      const std::vector<SequencedAccountTelemetryEvent>& events) {
    m_dataStore->Store(events);
  }

  template<typename D>
  void TelemetryDataStoreBox::WrappedTelemetryDataStore<D>::Close() {
    m_dataStore->Close();
  }
}

#endif
