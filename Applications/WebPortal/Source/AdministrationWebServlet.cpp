#include "WebPortal/AdministrationWebServlet.hpp"
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "Nexus/AdministrationService/AdministrationServices.hpp"
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

AdministrationWebServlet::AdministrationWebServlet(
  Ref<WebSessionStore<WebPortalSession>> sessions)
  : m_sessions(sessions.get()),
    m_protocol_server(&m_websocket_server,
      [] { return std::make_unique<LiveTimer>(seconds(30)); },
      std::bind_front(&AdministrationWebServlet::on_client_accepted, this),
      std::bind_front(&AdministrationWebServlet::on_client_closed, this)) {
  register_administration_services(out(m_protocol_server.get_slots()));
  register_administration_messages(out(m_protocol_server.get_slots()));
  MonitorNotificationsService::add_slot(out(m_protocol_server.get_slots()),
    std::bind_front(
      &AdministrationWebServlet::on_monitor_notifications, this));
}

AdministrationWebServlet::~AdministrationWebServlet() {
  close();
}

std::vector<HttpRequestSlot> AdministrationWebServlet::get_slots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/load_accounts_by_roles"), std::bind_front(
      &AdministrationWebServlet::on_load_accounts_by_roles, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/load_administrators_root_entry"),
    std::bind_front(
      &AdministrationWebServlet::on_load_administrators_root_entry, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/load_services_root_entry"), std::bind_front(
      &AdministrationWebServlet::on_load_services_root_entry, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/load_trading_groups_root_entry"),
    std::bind_front(
      &AdministrationWebServlet::on_load_trading_groups_root_entry, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/load_trading_group"),
    std::bind_front(&AdministrationWebServlet::on_load_trading_group, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/load_managed_trading_groups"),
    std::bind_front(
      &AdministrationWebServlet::on_load_managed_trading_groups, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/load_parent_trading_group"),
    std::bind_front(
      &AdministrationWebServlet::on_load_parent_trading_group, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/load_account_roles"),
    std::bind_front(&AdministrationWebServlet::on_load_account_roles, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/store_account_roles"),
    std::bind_front(&AdministrationWebServlet::on_store_account_roles, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/load_account_identity"),
    std::bind_front(&AdministrationWebServlet::on_load_account_identity, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/store_account_identity"), std::bind_front(
      &AdministrationWebServlet::on_store_account_identity, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/load_entitlements_database"),
    std::bind_front(
      &AdministrationWebServlet::on_load_entitlements_database, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/load_account_entitlements"),
    std::bind_front(
      &AdministrationWebServlet::on_load_account_entitlements, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/load_risk_parameters"),
    std::bind_front(&AdministrationWebServlet::on_load_risk_parameters, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/store_risk_parameters"),
    std::bind_front(&AdministrationWebServlet::on_store_risk_parameters, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/load_account_modification_request"),
    std::bind_front(
      &AdministrationWebServlet::on_load_account_modification_request, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/load_account_modification_request_ids"),
    std::bind_front(
      &AdministrationWebServlet::on_load_account_modification_request_ids,
      this));
  slots.emplace_back(matches_path(HttpMethod::POST, "/api/"
    "administration_service/load_managed_account_modification_request_ids"),
    std::bind_front(&AdministrationWebServlet::
      on_load_managed_account_modification_request_ids, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/load_entitlement_modification"),
    std::bind_front(
      &AdministrationWebServlet::on_load_entitlement_modification, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/submit_entitlement_modification_request"),
    std::bind_front(
      &AdministrationWebServlet::on_submit_entitlement_modification_request,
      this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/load_risk_modification"),
    std::bind_front(
      &AdministrationWebServlet::on_load_risk_modification, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/submit_risk_modification_request"),
    std::bind_front(
      &AdministrationWebServlet::on_submit_risk_modification_request, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/load_account_modification_request_status"),
    std::bind_front(
      &AdministrationWebServlet::on_load_account_modification_request_status,
      this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/load_account_modification_request_updates"),
    std::bind_front(
      &AdministrationWebServlet::on_load_account_modification_request_updates,
      this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/approve_account_modification_request"),
    std::bind_front(
      &AdministrationWebServlet::on_approve_account_modification_request,
      this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/reject_account_modification_request"),
    std::bind_front(
      &AdministrationWebServlet::on_reject_account_modification_request,
      this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/load_message"),
    std::bind_front(&AdministrationWebServlet::on_load_message, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/load_message_ids"),
    std::bind_front(&AdministrationWebServlet::on_load_message_ids, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/send_account_modification_request_message"),
    std::bind_front(
      &AdministrationWebServlet::on_send_account_modification_request_message,
      this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/send_notification"),
    std::bind_front(&AdministrationWebServlet::on_send_notification, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/administration_service/load_notifications"),
    std::bind_front(&AdministrationWebServlet::on_load_notifications, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/administration_service/mark_notification_as_read"), std::bind_front(
      &AdministrationWebServlet::on_mark_notification_as_read, this));
  return slots;
}

std::vector<HttpUpgradeSlot<AdministrationWebServlet::WebSocketChannel>>
    AdministrationWebServlet::get_web_socket_slots() {
  auto slots = std::vector<HttpUpgradeSlot<WebSocketChannel>>();
  slots.emplace_back(
    matches_path(HttpMethod::GET, "/api/administration_service/websocket"),
    std::bind_front(&AdministrationWebServlet::on_websocket_upgrade, this));
  return slots;
}

void AdministrationWebServlet::close() {
  if(m_open_state.set_closing()) {
    return;
  }
  m_tasks.close();
  m_websocket_server.close();
  m_protocol_server.close();
  {
    auto lock = std::lock_guard(m_websocket_session_mutex);
    m_pending_websocket_session.reset();
    m_websocket_session_condition.notify_one();
  }
  m_open_state.close();
}

void AdministrationWebServlet::on_websocket_upgrade(
    const HttpRequest& request, std::unique_ptr<WebSocketChannel> channel) {
  auto session = m_sessions->find(request);
  if(!session) {
    channel->get_connection().close();
    return;
  }
  channel->get_socket().set_binary_mode();
  auto lock = std::unique_lock(m_websocket_session_mutex);
  m_pending_websocket_session = session;
  m_websocket_server.push(std::move(channel));
  while(m_pending_websocket_session) {
    m_websocket_session_condition.wait(lock);
  }
}

void AdministrationWebServlet::on_client_accepted(
    WebServiceProtocolServer::ServiceProtocolClient& client) {
  auto lock = std::lock_guard(m_websocket_session_mutex);
  client.get_session().m_session = std::move(m_pending_websocket_session);
  m_websocket_session_condition.notify_one();
}

void AdministrationWebServlet::on_client_closed(
    WebServiceProtocolServer::ServiceProtocolClient& client) {
  if(client.get_session().m_notification_queue) {
    client.get_session().m_notification_queue->close();
  }
}

HttpResponse AdministrationWebServlet::on_load_accounts_by_roles(
    const HttpRequest& request) {
  struct Parameters {
    AccountRoles m_roles;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("roles", m_roles);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto accounts =
    clients.get_administration_client().load_accounts_by_roles(params.m_roles);
  session->shuttle_response(accounts, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_administrators_root_entry(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto root =
    clients.get_administration_client().load_administrators_root_entry();
  session->shuttle_response(root, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_services_root_entry(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto root = clients.get_administration_client().load_services_root_entry();
  session->shuttle_response(root, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_trading_groups_root_entry(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto root =
    clients.get_administration_client().load_trading_groups_root_entry();
  session->shuttle_response(root, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_trading_group(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_directory_entry;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("directory_entry", m_directory_entry);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto trading_group = clients.get_administration_client().load_trading_group(
    params.m_directory_entry);
  session->shuttle_response(trading_group, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_managed_trading_groups(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto trading_groups =
    clients.get_administration_client().load_managed_trading_groups(
      params.m_account);
  session->shuttle_response(trading_groups, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_parent_trading_group(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto trading_group =
    clients.get_administration_client().load_parent_trading_group(
      params.m_account);
  session->shuttle_response(trading_group, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_account_roles(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto roles =
    clients.get_administration_client().load_account_roles(params.m_account);
  session->shuttle_response(roles, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_store_account_roles(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountRoles m_roles;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("roles", m_roles);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto groups = clients.get_administration_client().load_managed_trading_groups(
    session->get_account());
  auto member_group = TradingGroup();
  for(auto& group : groups) {
    auto trading_group =
      clients.get_administration_client().load_trading_group(group);
    if(std::find(trading_group.get_managers().begin(),
        trading_group.get_managers().end(), params.m_account) !=
          trading_group.get_managers().end() ||
        std::find(trading_group.get_traders().begin(),
          trading_group.get_traders().end(), params.m_account) !=
            trading_group.get_traders().end()) {
      member_group = trading_group;
      break;
    }
  }
  auto previous_roles =
    clients.get_administration_client().load_account_roles(params.m_account);
  if(member_group.get_entry().m_type != DirectoryEntry::Type::DIRECTORY) {
    session->shuttle_response(previous_roles, out(response));
    return response;
  }
  if(params.m_roles.test(AccountRole::MANAGER) !=
      previous_roles.test(AccountRole::MANAGER)) {
    if(params.m_roles.test(AccountRole::MANAGER)) {
      clients.get_service_locator_client().store(
        params.m_account, member_group.get_entry(), Permission::READ);
      clients.get_service_locator_client().associate(
        params.m_account, member_group.get_managers_directory());
    } else {
      clients.get_service_locator_client().store(
        params.m_account, member_group.get_entry(), Permissions(0));
      clients.get_service_locator_client().detach(
        params.m_account, member_group.get_managers_directory());
    }
  }
  if(params.m_roles.test(AccountRole::TRADER) !=
      previous_roles.test(AccountRole::TRADER)) {
    if(params.m_roles.test(AccountRole::TRADER)) {
      clients.get_service_locator_client().associate(
        params.m_account, member_group.get_traders_directory());
    } else {
      clients.get_service_locator_client().detach(
        params.m_account, member_group.get_traders_directory());
    }
  }
  auto new_roles =
    clients.get_administration_client().load_account_roles(params.m_account);
  session->shuttle_response(new_roles, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_account_identity(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto identity =
    clients.get_administration_client().load_identity(params.m_account);
  session->shuttle_response(identity, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_store_account_identity(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountIdentity m_identity;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("identity", m_identity);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  clients.get_administration_client().store(
    params.m_account, params.m_identity);
  return response;
}

auto AdministrationWebServlet::on_load_entitlements_database(
    const HttpRequest& request) -> HttpResponse {
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto database = clients.get_administration_client().load_entitlements();
  session->shuttle_response(database, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_account_entitlements(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto entitlements =
    clients.get_administration_client().load_entitlements(params.m_account);
  session->shuttle_response(entitlements, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_risk_parameters(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto queue = std::make_shared<Queue<RiskParameters>>();
  clients.get_administration_client().get_risk_parameters_publisher(
    params.m_account).monitor(queue);
  auto risk_parameters = queue->pop();
  session->shuttle_response(risk_parameters, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_store_risk_parameters(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    RiskParameters m_risk_parameters;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("risk_parameters", m_risk_parameters);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  clients.get_administration_client().store(
    params.m_account, params.m_risk_parameters);
  return response;
}

HttpResponse AdministrationWebServlet::on_load_account_modification_request(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto modification =
    clients.get_administration_client().load_account_modification_request(
      params.m_id);
  session->shuttle_response(modification, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_account_modification_request_ids(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountModificationRequest::Id m_start_id;
    int m_max_count;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("start_id", m_start_id);
      shuttle.shuttle("max_count", m_max_count);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto request_ids =
    clients.get_administration_client().load_account_modification_request_ids(
      params.m_account, params.m_start_id, params.m_max_count);
  session->shuttle_response(request_ids, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::
    on_load_managed_account_modification_request_ids(
      const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountModificationRequest::Id m_start_id;
    int m_max_count;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("start_id", m_start_id);
      shuttle.shuttle("max_count", m_max_count);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto request_ids = clients.get_administration_client().
    load_managed_account_modification_request_ids(
      params.m_account, params.m_start_id, params.m_max_count);
  session->shuttle_response(request_ids, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_entitlement_modification(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto entitlement =
    clients.get_administration_client().load_entitlement_modification(
      params.m_id);
  session->shuttle_response(entitlement, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::
    on_submit_entitlement_modification_request(const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    EntitlementModification m_modification;
    Message m_comment;
    posix_time::ptime m_effective_date;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("modification", m_modification);
      shuttle.shuttle("comment", m_comment);
      shuttle.shuttle("effective_date", m_effective_date);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto modification = clients.get_administration_client().submit(
    params.m_account, params.m_modification, params.m_effective_date,
    params.m_comment);
  session->shuttle_response(modification, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_risk_modification(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto risk_modification =
    clients.get_administration_client().load_risk_modification(params.m_id);
  session->shuttle_response(risk_modification, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_submit_risk_modification_request(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    RiskModification m_modification;
    Message m_comment;
    posix_time::ptime m_effective_date;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("modification", m_modification);
      shuttle.shuttle("comment", m_comment);
      shuttle.shuttle("effective_date", m_effective_date);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto modification = clients.get_administration_client().submit(
    params.m_account, params.m_modification, params.m_effective_date,
    params.m_comment);
  session->shuttle_response(modification, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::
    on_load_account_modification_request_status(const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto status = clients.get_administration_client().
    load_account_modification_request_status(params.m_id);
  session->shuttle_response(status, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::
    on_load_account_modification_request_updates(const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto updates = clients.get_administration_client().
    load_account_modification_request_updates(params.m_id);
  session->shuttle_response(updates, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_approve_account_modification_request(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;
    Message m_comment;
    posix_time::ptime m_effective_date;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
      shuttle.shuttle("comment", m_comment);
      shuttle.shuttle("effective_date", m_effective_date);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto update =
    clients.get_administration_client().approve_account_modification_request(
      params.m_id, params.m_effective_date, params.m_comment);
  session->shuttle_response(update, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_reject_account_modification_request(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;
    Message m_comment;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
      shuttle.shuttle("comment", m_comment);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto update =
    clients.get_administration_client().reject_account_modification_request(
      params.m_id, params.m_comment);
  session->shuttle_response(update, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_message(
    const HttpRequest& request) {
  struct Parameters {
    Message::Id m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto message = clients.get_administration_client().load_message(params.m_id);
  session->shuttle_response(message, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_message_ids(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto message_ids =
    clients.get_administration_client().load_message_ids(params.m_id);
  session->shuttle_response(message_ids, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::
    on_send_account_modification_request_message(const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;
    Message m_message;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
      shuttle.shuttle("message", m_message);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto message = clients.get_administration_client().
    send_account_modification_request_message(params.m_id, params.m_message);
  session->shuttle_response(message, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_send_notification(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    std::string m_description;
    std::string m_data;
    Notification::Category m_category;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("description", m_description);
      shuttle.shuttle("data", m_data);
      shuttle.shuttle("category", m_category);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto notification = clients.get_administration_client().send_notification(
    params.m_account, params.m_description, params.m_data, params.m_category);
  session->shuttle_response(notification, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_notifications(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    Notification::Id m_id;
    SnapshotLimit m_limit;
    Notification::ReadState m_read_state;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("id", m_id);
      shuttle.shuttle("limit", m_limit);
      shuttle.shuttle("read_state", m_read_state);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto notifications = clients.get_administration_client().load_notifications(
    params.m_account, params.m_id, params.m_limit, params.m_read_state);
  session->shuttle_response(notifications, out(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_mark_notification_as_read(
    const HttpRequest& request) {
  struct Parameters {
    Notification::Id m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  clients.get_administration_client().mark_notification_as_read(params.m_id);
  return response;
}

Notification::Id AdministrationWebServlet::on_monitor_notifications(
    WebServiceProtocolServer::ServiceProtocolClient& client,
    const DirectoryEntry& account) {
  auto& session = client.get_session();
  auto& clients = session.m_session->get_clients();
  session.m_notification_queue = std::make_shared<
    ScopedQueueWriter<Notification>>(
    m_tasks.get_slot<Notification>(std::bind_front(
      &AdministrationWebServlet::on_notification, this, std::ref(client))));
  return clients.get_administration_client().monitor_notifications(
    account, session.m_notification_queue);
}

void AdministrationWebServlet::on_notification(
    WebServiceProtocolServer::ServiceProtocolClient& client,
    const Notification& notification) {
  send_record_message<NotificationMessage>(client, notification);
}
