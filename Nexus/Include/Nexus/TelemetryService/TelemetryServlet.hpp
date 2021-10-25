#ifndef NEXUS_TELEMETRY_SERVICE_TELEMETRY_SERVLET_HPP
#define NEXUS_TELEMETRY_SERVICE_TELEMETRY_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/IndexedSubscriptions.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Services/ServiceProtocolServlet.hpp>
#include <Beam/ServiceLocator/SessionEncryption.hpp>
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"
#include "Nexus/TelemetryService/TelemetryRegistry.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"
#include "Nexus/TelemetryService/TelemetryServices.hpp"
#include "Nexus/TelemetryService/TelemetrySession.hpp"

namespace Nexus::TelemetryService {

  /**
   * Implements the servlet for telemetry data.
   * @param <C> The container instantiating this servlet.
   * @param <T> The type of TimeClient used for timestamps.
   * @param <A> The type of AdministrationClient to use for permissions.
   * @param <D> The type of TelemetryDataStore to use.
   */
  template<typename C, typename T, typename A, typename D>
  class TelemetryServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of TimeClient used for timestamps. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /** The type of AdministrationClient used to check permissions. */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /** The type of TelemetryDataStore used. */
      using TelemetryDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs an TelemetryServlet.
       * @param timeClient Initializes the TimeClient.
       * @param administrationClient Initializes the AdministrationClient.
       * @param dataStore Initializes the TelemetryDataStore.
       */
      template<typename TF, typename AF, typename DF>
      TelemetryServlet(
        TF&& timeClient, AF&& administrationClient, DF&& dataStore);

      void RegisterServices(
        Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      template<typename T>
      using RequestToken =
        Beam::Services::RequestToken<ServiceProtocolClient, T>;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      Beam::Queries::IndexedSubscriptions<TelemetryEvent,
        Beam::ServiceLocator::DirectoryEntry, ServiceProtocolClient>
          m_telemetryEventSubscriptions;
      TelemetryRegistry m_registry;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      TelemetryServlet(const TelemetryServlet&) = delete;
      TelemetryServlet& operator =(const TelemetryServlet&) = delete;
      void OnQueryAccountTelemetry(
        RequestToken<QueryAccountTelemetryService>& request,
        const AccountQuery& query);
      void OnRecord(ServiceProtocolClient& client,
        const std::vector<RecordUpdate>& updates);
  };

  template<typename T, typename A, typename D>
  struct MetaTelemetryServlet {
    using Session = TelemetrySession;
    template<typename C>
    struct apply {
      using type = TelemetryServlet<C, T, A, D>;
    };
  };

  template<typename C, typename T, typename A, typename D>
  template<typename TF, typename AF, typename DF>
  TelemetryServlet<C, T, A, D>::TelemetryServlet(
    TF&& timeClient, AF&& administrationClient, DF&& dataStore)
    : m_timeClient(std::forward<TF>(timeClient)),
      m_administrationClient(std::forward<AF>(administrationClient)),
      m_dataStore(std::forward<DF>(dataStore)) {}

  template<typename C, typename T, typename A, typename D>
  void TelemetryServlet<C, T, A, D>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    Queries::RegisterQueryTypes(Beam::Store(slots->GetRegistry()));
    RegisterTelemetryServices(Beam::Store(slots));
    RegisterTelemetryMessages(Beam::Store(slots));
    QueryAccountTelemetryService::AddRequestSlot(Beam::Store(slots),
      std::bind_front(&TelemetryServlet::OnQueryAccountTelemetry, this));
    RecordService::AddSlot(Beam::Store(slots),
      std::bind_front(&TelemetryServlet::OnRecord, this));
  }

  template<typename C, typename T, typename A, typename D>
  void TelemetryServlet<C, T, A, D>::HandleClientAccepted(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    session.m_sessionId = Beam::ServiceLocator::GenerateSessionId();
    session.m_roles = m_administrationClient->LoadAccountRoles(
      session.GetAccount());
  }

  template<typename C, typename T, typename A, typename D>
  void TelemetryServlet<C, T, A, D>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_telemetryEventSubscriptions.RemoveAll(client);
  }

  template<typename C, typename T, typename A, typename D>
  void TelemetryServlet<C, T, A, D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_tasks.Break();
    m_tasks.Wait();
    m_dataStore->Close();
    m_openState.Close();
  }

  template<typename C, typename T, typename A, typename D>
  void TelemetryServlet<C, T, A, D>::OnQueryAccountTelemetry(
      RequestToken<QueryAccountTelemetryService>& request,
      const AccountQuery& query) {
    auto& session = request.GetSession();
    if(!session.m_roles.Test(
        AdministrationService::AccountRole::ADMINISTRATOR)) {
      request.SetResult(AccountTelemetryQueryResult());
      return;
    }
    auto filter =
      Beam::Queries::Translate<Queries::EvaluatorTranslator>(query.GetFilter());
    auto result = AccountTelemetryQueryResult();
    result.m_queryId = m_telemetryEventSubscriptions.Initialize(
      query.GetIndex(), request.GetClient(), query.GetRange(),
      std::move(filter));
    result.m_snapshot = m_dataStore->LoadTelemetryEvents(query);
    m_telemetryEventSubscriptions.Commit(query.GetIndex(), std::move(result),
      [&] (const auto& result) {
        request.SetResult(result);
      });
  }

  template<typename C, typename T, typename A, typename D>
  void TelemetryServlet<C, T, A, D>::OnRecord(
      ServiceProtocolClient& client, const std::vector<RecordUpdate>& updates) {
    auto& session = client.GetSession();
    auto account = session.GetAccount();
    auto revisedUpdates = std::vector<TelemetryEvent>();
    auto timestamp = m_timeClient->GetTime();
    for(auto i = updates.rbegin(); i != updates.rend(); ++i) {
      auto revisedTimestamp = timestamp -
        std::max<boost::posix_time::time_duration>(updates.back().timestamp -
          i->timestamp, boost::posix_time::seconds(0));
      revisedUpdates.emplace_back(
        session.m_sessionId, i->name, revisedTimestamp, i->data);
    }
    for(auto i = revisedUpdates.rbegin(); i != revisedUpdates.rend(); ++i) {
      m_registry.Publish(Beam::Queries::IndexedValue(*i, account),
        [&] {
          return LoadInitialSequences(*m_dataStore, account);
        },
        [&] (const auto& event) {
          m_dataStore->Store(event);
          m_telemetryEventSubscriptions.Publish(event,
            [&] (const auto& clients) {
              Beam::Services::BroadcastRecordMessage<TelemetryEventMessage>(
                clients, event);
            });
        });
    }
  }
}

#endif
