#include "WebPortal/AdministrationWebServlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

AdministrationWebServlet::AdministrationWebServlet(
  Ref<WebSessionStore<WebPortalSession>> sessions)
  : m_sessions(sessions.get()) {}

AdministrationWebServlet::~AdministrationWebServlet() {
  close();
}

auto AdministrationWebServlet::get_slots() -> std::vector<HttpRequestSlot> {
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
    HttpMethod::POST, "/api/administration_service/store_account_entitlements"),
    std::bind_front(
      &AdministrationWebServlet::on_store_account_entitlements, this));
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
  return slots;
}

void AdministrationWebServlet::close() {
  m_open_state.close();
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

HttpResponse AdministrationWebServlet::on_store_account_entitlements(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    std::vector<DirectoryEntry> m_entitlements;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("entitlements", m_entitlements);
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
  clients.get_administration_client().store_entitlements(
    params.m_account, params.m_entitlements);
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

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("modification", m_modification);
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
  auto modification = clients.get_administration_client().submit(
    params.m_account, params.m_modification, params.m_comment);
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

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("modification", m_modification);
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
  auto modification = clients.get_administration_client().submit(
    params.m_account, params.m_modification, params.m_comment);
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

HttpResponse AdministrationWebServlet::on_approve_account_modification_request(
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
    clients.get_administration_client().approve_account_modification_request(
      params.m_id, params.m_comment);
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
