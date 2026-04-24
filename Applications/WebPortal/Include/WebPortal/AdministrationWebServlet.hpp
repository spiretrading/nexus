#ifndef NEXUS_ADMINISTRATION_WEB_SERVLET_HPP
#define NEXUS_ADMINISTRATION_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include "Nexus/AdministrationService/Notification.hpp"
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Services/ServiceProtocolServer.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/HttpUpgradeSlot.hpp>
#include <Beam/WebServices/WebSessionStore.hpp>
#include <Beam/WebServices/WebSocketChannel.hpp>
#include "WebPortal/WebPortalSession.hpp"
#include "WebPortal/WebSocketServerConnection.hpp"

namespace Nexus {

  /** Stores session info for a WebSocket service protocol client. */
  struct WebSocketSession {

    /** The authenticated web portal session. */
    std::shared_ptr<WebPortalSession> m_session;

    /** The queue used to receive and forward notifications to the client. */
    std::shared_ptr<Beam::ScopedQueueWriter<Notification>> m_notification_queue;
  };

  /** Provides a web interface to the AdministrationService. */
  class AdministrationWebServlet {
    public:

      /** The type of WebSocketChannel used. */
      using WebSocketChannel =
        Beam::WebSocketChannel<std::shared_ptr<Beam::TcpSocketChannel>>;

      /** The type of ServiceProtocolServer used over WebSocket. */
      using WebServiceProtocolServer = Beam::ServiceProtocolServer<
        WebSocketServerConnection*, Beam::JsonSender<Beam::SharedBuffer>,
        Beam::NullEncoder, std::unique_ptr<Beam::LiveTimer>, WebSocketSession>;

      /**
       * Constructs an AdministrationWebServlet.
       * @param sessions The available web sessions.
       */
      explicit AdministrationWebServlet(
        Beam::Ref<Beam::WebSessionStore<WebPortalSession>> sessions);

      ~AdministrationWebServlet();

      std::vector<Beam::HttpRequestSlot> get_slots();
      std::vector<Beam::HttpUpgradeSlot<WebSocketChannel>>
        get_web_socket_slots();
      void close();

    private:
      Beam::WebSessionStore<WebPortalSession>* m_sessions;
      WebSocketServerConnection m_websocket_server;
      Beam::Mutex m_websocket_session_mutex;
      Beam::ConditionVariable m_websocket_session_condition;
      std::shared_ptr<WebPortalSession> m_pending_websocket_session;
      WebServiceProtocolServer m_protocol_server;
      Beam::RoutineTaskQueue m_tasks;
      Beam::OpenState m_open_state;

      AdministrationWebServlet(const AdministrationWebServlet&) = delete;
      AdministrationWebServlet& operator=(
        const AdministrationWebServlet&) = delete;
      void on_websocket_upgrade(const Beam::HttpRequest& request,
        std::unique_ptr<WebSocketChannel> channel);
      void on_client_accepted(
        WebServiceProtocolServer::ServiceProtocolClient& client);
      void on_client_closed(
        WebServiceProtocolServer::ServiceProtocolClient& client);
      Beam::HttpResponse on_load_accounts_by_roles(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_administrators_root_entry(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_services_root_entry(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_trading_groups_root_entry(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_trading_group(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_managed_trading_groups(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_parent_trading_group(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_account_roles(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_store_account_roles(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_account_identity(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_store_account_identity(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_entitlements_database(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_account_entitlements(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_store_account_entitlements(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_risk_parameters(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_store_risk_parameters(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_account_modification_request(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_account_modification_request_ids(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_managed_account_modification_request_ids(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_entitlement_modification(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_submit_entitlement_modification_request(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_risk_modification(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_submit_risk_modification_request(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_account_modification_request_status(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_account_modification_request_updates(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_approve_account_modification_request(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_reject_account_modification_request(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_message(const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_message_ids(const Beam::HttpRequest& request);
      Beam::HttpResponse on_send_account_modification_request_message(
        const Beam::HttpRequest& request);
      Notification on_send_notification(
        WebServiceProtocolServer::ServiceProtocolClient& client,
        Beam::DirectoryEntry account, std::string description,
        std::string data, Notification::Category category);
      Notification::Id on_monitor_notifications(
        WebServiceProtocolServer::ServiceProtocolClient& client,
        const Beam::DirectoryEntry& account);
      std::vector<Notification> on_load_notifications(
        WebServiceProtocolServer::ServiceProtocolClient& client,
        const Beam::DirectoryEntry& account, const Notification::Id& id,
        Beam::SnapshotLimit limit, Notification::ReadState read_state);
      void on_mark_notification_as_read(
        WebServiceProtocolServer::ServiceProtocolClient& client,
        const Notification::Id& id);
      void on_notification(
        WebServiceProtocolServer::ServiceProtocolClient& client,
        const Notification& notification);
  };
}

#endif
