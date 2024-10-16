#ifndef NEXUS_TELEMETRY_CLIENT_HPP
#define NEXUS_TELEMETRY_CLIENT_HPP
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/QueryClientPublisher.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <Beam/Threading/Mutex.hpp>
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"
#include "Nexus/TelemetryService/TelemetryServices.hpp"

namespace Nexus::TelemetryService {

  /**
   * Client used to access the telemetry services.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   * @param <T> The type of TimeClient used to timestamp each event.
   */
  template<typename B, typename T>
  class TelemetryClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /** The type of TimeClient used to timestamp each event. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /**
       * Constructs a TelemetryClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       * @param timeClient Initializes the TimeClient.
       */
      template<typename BF, typename TF>
      explicit TelemetryClient(BF&& clientBuilder, TF&& timeClient);

      ~TelemetryClient();

      /**
       * Submits a query for TelemetryEvents.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryTelemetryEvents(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedTelemetryEvent> queue);

      /**
       * Submits a query for TelemetryEvents.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryTelemetryEvents(const AccountQuery& query,
        Beam::ScopedQueueWriter<TelemetryEvent> queue);

      /**
       * Records an event associated with this client's session.
       * @param name The name of the event.
       * @param data The data associated with the event.
       */
      void Record(const std::string& name, const Beam::JsonObject& data);

      void Close();

    private:
      template<typename Value, typename Query, typename QueryService,
        typename EndQueryMessage>
      using QueryClientPublisher = Beam::Queries::QueryClientPublisher<Value,
        Query, Queries::EvaluatorTranslator,
        Beam::Services::ServiceProtocolClientHandler<B>, QueryService,
        EndQueryMessage>;
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      QueryClientPublisher<TelemetryEvent, AccountQuery,
        QueryAccountTelemetryService, EndAccountTelemetryEventQueryMessage>
          m_accountTelemetryEventPublisher;
      Beam::SynchronizedVector<RecordUpdate, Beam::Threading::Mutex> m_updates;
      Beam::IO::OpenState m_openState;

      TelemetryClient(const TelemetryClient&) = delete;
      TelemetryClient& operator =(const TelemetryClient&) = delete;
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
  };

  template<typename B, typename T>
  template<typename BF, typename TF>
  TelemetryClient<B, T>::TelemetryClient(BF&& clientBuilder, TF&& timeClient)
      try : m_clientHandler(std::forward<BF>(clientBuilder)),
            m_timeClient(std::forward<TF>(timeClient)),
            m_accountTelemetryEventPublisher(Beam::Ref(m_clientHandler)) {
    Queries::RegisterQueryTypes(
      Beam::Store(m_clientHandler.GetSlots().GetRegistry()));
    RegisterTelemetryServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterTelemetryMessages(Beam::Store(m_clientHandler.GetSlots()));
    m_accountTelemetryEventPublisher.
      template AddMessageHandler<TelemetryEventMessage>();
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the telemetry server."));
  }

  template<typename B, typename T>
  TelemetryClient<B, T>::~TelemetryClient() {
    Close();
  }

  template<typename B, typename T>
  void TelemetryClient<B, T>::QueryTelemetryEvents(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedTelemetryEvent> queue) {
    m_accountTelemetryEventPublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B, typename T>
  void TelemetryClient<B, T>::QueryTelemetryEvents(const AccountQuery& query,
      Beam::ScopedQueueWriter<TelemetryEvent> queue) {
    m_accountTelemetryEventPublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B, typename T>
  void TelemetryClient<B, T>::Record(
      const std::string& name, const Beam::JsonObject& data) {
    Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      m_updates.With([&] (auto& updates) {
        updates.emplace_back(name, m_timeClient->GetTime(), data);
        constexpr auto threshold = 10;
        if(updates.size() >= threshold) {
          client->template SendRequest<RecordService>(updates);
          updates.clear();
        }
      });
    }, "Failed to submit records.");
  }

  template<typename B, typename T>
  void TelemetryClient<B, T>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    try {
      auto client = m_clientHandler.GetClient();
      m_updates.With([&] (auto& updates) {
        if(!updates.empty()) {
          client->template SendRequest<RecordService>(updates);
        }
        updates.clear();
      });
    } catch(const std::exception&) {
      std::cerr << "Failed to submit records on close." << std::endl;
    }
    m_clientHandler.Close();
    m_accountTelemetryEventPublisher.Break();
    m_openState.Close();
  }

  template<typename B, typename T>
  void TelemetryClient<B, T>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_accountTelemetryEventPublisher.Recover(*client);
  }
}

#endif
