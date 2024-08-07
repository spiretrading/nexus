#ifndef NEXUS_TELEMETRY_CLIENT_BOX_HPP
#define NEXUS_TELEMETRY_CLIENT_BOX_HPP
#include <type_traits>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include "Nexus/TelemetryService/AccountQuery.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {

  /** Provides a generic interface over an arbitrary TelemetryClient. */
  class TelemetryClientBox {
    public:

      /**
       * Constructs a TelemetryClientBox of a specified type using emplacement.
       * @param <T> The type of telemetry client to emplace.
       * @param args The arguments to pass to the emplaced telemetry client.
       */
      template<typename T, typename... Args>
      explicit TelemetryClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a TelemetryClientBox by copying an existing telemetry
       * client.
       * @param client The client to copy.
       */
      template<typename TelemetryClient>
      explicit TelemetryClientBox(TelemetryClient client);

      explicit TelemetryClientBox(TelemetryClientBox* client);

      explicit TelemetryClientBox(
        const std::shared_ptr<TelemetryClientBox>& client);

      explicit TelemetryClientBox(
        const std::unique_ptr<TelemetryClientBox>& client);

      void QueryTelemetryEvents(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedTelemetryEvent> queue);

      void QueryTelemetryEvents(const AccountQuery& query,
        Beam::ScopedQueueWriter<TelemetryEvent> queue);

      void Record(const std::string& name, const Beam::JsonObject& data);

      void Close();

    private:
      struct VirtualTelemetryClient {
        virtual ~VirtualTelemetryClient() = default;
        virtual void QueryTelemetryEvents(const AccountQuery& query,
          Beam::ScopedQueueWriter<SequencedTelemetryEvent> queue) = 0;
        virtual void QueryTelemetryEvents(const AccountQuery& query,
          Beam::ScopedQueueWriter<TelemetryEvent> queue) = 0;
        virtual void Record(
          const std::string& name, const Beam::JsonObject& data) = 0;
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedTelemetryClient final : VirtualTelemetryClient {
        using TelemetryClient = C;
        Beam::GetOptionalLocalPtr<TelemetryClient> m_client;

        template<typename... Args>
        WrappedTelemetryClient(Args&&... args);
        void QueryTelemetryEvents(const AccountQuery& query,
          Beam::ScopedQueueWriter<SequencedTelemetryEvent> queue) override;
        void QueryTelemetryEvents(const AccountQuery& query,
          Beam::ScopedQueueWriter<TelemetryEvent> queue) override;
        void Record(const std::string& name, const Beam::JsonObject& data)
          override;
        void Close() override;
      };
      std::shared_ptr<VirtualTelemetryClient> m_client;
  };

  template<typename T, typename... Args>
  TelemetryClientBox::TelemetryClientBox(
    std::in_place_type_t<T>, Args&&... args)
      : m_client(std::make_shared<WrappedTelemetryClient<T>>(
          std::forward<Args>(args)...)) {}

  template<typename TelemetryClient>
  TelemetryClientBox::TelemetryClientBox(TelemetryClient client)
    : TelemetryClientBox(
        std::in_place_type<TelemetryClient>, std::move(client)) {}

  inline TelemetryClientBox::TelemetryClientBox(TelemetryClientBox* client)
    : TelemetryClientBox(*client) {}

  inline TelemetryClientBox::TelemetryClientBox(
    const std::shared_ptr<TelemetryClientBox>& client)
      : TelemetryClientBox(*client) {}

  inline TelemetryClientBox::TelemetryClientBox(
    const std::unique_ptr<TelemetryClientBox>& client)
      : TelemetryClientBox(*client) {}

  inline void TelemetryClientBox::QueryTelemetryEvents(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedTelemetryEvent> queue) {
    m_client->QueryTelemetryEvents(query, std::move(queue));
  }

  inline void TelemetryClientBox::QueryTelemetryEvents(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<TelemetryEvent> queue) {
    m_client->QueryTelemetryEvents(query, std::move(queue));
  }

  inline void TelemetryClientBox::Record(
      const std::string& name, const Beam::JsonObject& data) {
    m_client->Record(name, data);
  }

  inline void TelemetryClientBox::Close() {
    m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  TelemetryClientBox::WrappedTelemetryClient<C>::WrappedTelemetryClient(
    Args&&... args)
      : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  void TelemetryClientBox::WrappedTelemetryClient<C>::QueryTelemetryEvents(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedTelemetryEvent> queue) {
    m_client->QueryTelemetryEvents(query, std::move(queue));
  }

  template<typename C>
  void TelemetryClientBox::WrappedTelemetryClient<C>::QueryTelemetryEvents(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<TelemetryEvent> queue) {
    m_client->QueryTelemetryEvents(query, std::move(queue));
  }

  template<typename C>
  void TelemetryClientBox::WrappedTelemetryClient<C>::Record(
      const std::string& name, const Beam::JsonObject& data) {
    m_client->Record(name, data);
  }

  template<typename C>
  void TelemetryClientBox::WrappedTelemetryClient<C>::Close() {
    m_client->Close();
  }
}

#endif
