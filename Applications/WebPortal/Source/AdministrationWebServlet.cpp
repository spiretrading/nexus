#include "WebPortal/AdministrationWebServlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::WebServices;
using namespace boost;
using namespace Nexus;

AdministrationWebServlet::AdministrationWebServlet(
  Ref<SessionStore<WebPortalSession>> sessions)
  : m_sessions(sessions.Get()) {}

AdministrationWebServlet::~AdministrationWebServlet() {
  close();
}

auto AdministrationWebServlet::get_slots() -> std::vector<HttpRequestSlot> {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_accounts_by_roles"), std::bind_front(
      &AdministrationWebServlet::on_load_accounts_by_roles, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_administrators_root_entry"),
    std::bind_front(
      &AdministrationWebServlet::on_load_administrators_root_entry, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_services_root_entry"), std::bind_front(
      &AdministrationWebServlet::on_load_services_root_entry, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_trading_groups_root_entry"),
    std::bind_front(
      &AdministrationWebServlet::on_load_trading_groups_root_entry, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/load_trading_group"),
    std::bind_front(&AdministrationWebServlet::on_load_trading_group, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_managed_trading_groups"),
    std::bind_front(
      &AdministrationWebServlet::on_load_managed_trading_groups, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/load_parent_trading_group"),
    std::bind_front(
      &AdministrationWebServlet::on_load_parent_trading_group, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/load_account_roles"),
    std::bind_front(&AdministrationWebServlet::on_load_account_roles, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/store_account_roles"),
    std::bind_front(&AdministrationWebServlet::on_store_account_roles, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/load_account_identity"),
    std::bind_front(&AdministrationWebServlet::on_load_account_identity, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/store_account_identity"), std::bind_front(
      &AdministrationWebServlet::on_store_account_identity, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/load_entitlements_database"),
    std::bind_front(
      &AdministrationWebServlet::on_load_entitlements_database, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/load_account_entitlements"),
    std::bind_front(
      &AdministrationWebServlet::on_load_account_entitlements, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/store_account_entitlements"),
    std::bind_front(
      &AdministrationWebServlet::on_store_account_entitlements, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/load_risk_parameters"),
    std::bind_front(&AdministrationWebServlet::on_load_risk_parameters, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/store_risk_parameters"),
    std::bind_front(&AdministrationWebServlet::on_store_risk_parameters, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_modification_request"),
    std::bind_front(
      &AdministrationWebServlet::on_load_account_modification_request, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_modification_request_ids"),
    std::bind_front(
      &AdministrationWebServlet::on_load_account_modification_request_ids,
      this));
  slots.emplace_back(MatchesPath(HttpMethod::POST, "/api/"
    "administration_service/load_managed_account_modification_request_ids"),
    std::bind_front(&AdministrationWebServlet::
      on_load_managed_account_modification_request_ids, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_entitlement_modification"),
    std::bind_front(
      &AdministrationWebServlet::on_load_entitlement_modification, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/submit_entitlement_modification_request"),
    std::bind_front(
      &AdministrationWebServlet::on_submit_entitlement_modification_request,
      this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/load_risk_modification"),
    std::bind_front(
      &AdministrationWebServlet::on_load_risk_modification, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/submit_risk_modification_request"),
    std::bind_front(
      &AdministrationWebServlet::on_submit_risk_modification_request, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_modification_request_status"),
    std::bind_front(
      &AdministrationWebServlet::on_load_account_modification_request_status,
      this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/approve_account_modification_request"),
    std::bind_front(
      &AdministrationWebServlet::on_approve_account_modification_request,
      this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/reject_account_modification_request"),
    std::bind_front(
      &AdministrationWebServlet::on_reject_account_modification_request,
      this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/load_message"),
    std::bind_front(&AdministrationWebServlet::on_load_message, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/administration_service/load_message_ids"),
    std::bind_front(&AdministrationWebServlet::on_load_message_ids, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/send_account_modification_request_message"),
    std::bind_front(
      &AdministrationWebServlet::on_send_account_modification_request_message,
      this));
  return slots;
}

void AdministrationWebServlet::close() {
  m_open_state.Close();
}

HttpResponse AdministrationWebServlet::on_load_accounts_by_roles(
    const HttpRequest& request) {
  struct Parameters {
    AccountRoles m_roles;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("roles", m_roles);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto accounts =
    clients.get_administration_client().load_accounts_by_roles(params.m_roles);
  session->shuttle_response(accounts, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_administrators_root_entry(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto root =
    clients.get_administration_client().load_administrators_root_entry();
  session->shuttle_response(root, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_services_root_entry(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto root = clients.get_administration_client().load_services_root_entry();
  session->shuttle_response(root, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_trading_groups_root_entry(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto root =
    clients.get_administration_client().load_trading_groups_root_entry();
  session->shuttle_response(root, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_trading_group(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_directory_entry;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directory_entry);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto trading_group = clients.get_administration_client().load_trading_group(
    params.m_directory_entry);
  session->shuttle_response(trading_group, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_managed_trading_groups(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto trading_groups =
    clients.get_administration_client().load_managed_trading_groups(
      params.m_account);
  session->shuttle_response(trading_groups, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_parent_trading_group(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto trading_group =
    clients.get_administration_client().load_parent_trading_group(
      params.m_account);
  session->shuttle_response(trading_group, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_account_roles(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto roles =
    clients.get_administration_client().load_account_roles(params.m_account);
  session->shuttle_response(roles, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_store_account_roles(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountRoles m_roles;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("roles", m_roles);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto groups = clients.get_administration_client().load_managed_trading_groups(
    session->GetAccount());
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
    session->shuttle_response(previous_roles, Store(response));
    return response;
  }
  if(params.m_roles.Test(AccountRole::MANAGER) !=
      previous_roles.Test(AccountRole::MANAGER)) {
    if(params.m_roles.Test(AccountRole::MANAGER)) {
      clients.get_service_locator_client().StorePermissions(
        params.m_account, member_group.get_entry(), Permission::READ);
      clients.get_service_locator_client().Associate(
        params.m_account, member_group.get_managers_directory());
    } else {
      clients.get_service_locator_client().StorePermissions(
        params.m_account, member_group.get_entry(), Permissions(0));
      clients.get_service_locator_client().Detach(
        params.m_account, member_group.get_managers_directory());
    }
  }
  if(params.m_roles.Test(AccountRole::TRADER) !=
      previous_roles.Test(AccountRole::TRADER)) {
    if(params.m_roles.Test(AccountRole::TRADER)) {
      clients.get_service_locator_client().Associate(
        params.m_account, member_group.get_traders_directory());
    } else {
      clients.get_service_locator_client().Detach(
        params.m_account, member_group.get_traders_directory());
    }
  }
  auto new_roles =
    clients.get_administration_client().load_account_roles(params.m_account);
  session->shuttle_response(new_roles, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_account_identity(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto identity =
    clients.get_administration_client().load_identity(params.m_account);
  session->shuttle_response(identity, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_store_account_identity(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountIdentity m_identity;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("identity", m_identity);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
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
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto database = clients.get_administration_client().load_entitlements();
  session->shuttle_response(database, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_account_entitlements(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto entitlements =
    clients.get_administration_client().load_entitlements(params.m_account);
  session->shuttle_response(entitlements, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_store_account_entitlements(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    std::vector<DirectoryEntry> m_entitlements;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("entitlements", m_entitlements);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
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

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto queue = std::make_shared<Queue<RiskParameters>>();
  clients.get_administration_client().get_risk_parameters_publisher(
    params.m_account).Monitor(queue);
  auto risk_parameters = queue->Pop();
  session->shuttle_response(risk_parameters, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_store_risk_parameters(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    RiskParameters m_risk_parameters;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("risk_parameters", m_risk_parameters);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
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

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto modification =
    clients.get_administration_client().load_account_modification_request(
      params.m_id);
  session->shuttle_response(modification, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_account_modification_request_ids(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountModificationRequest::Id m_start_id;
    int m_max_count;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("start_id", m_start_id);
      shuttle.Shuttle("max_count", m_max_count);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto request_ids =
    clients.get_administration_client().load_account_modification_request_ids(
      params.m_account, params.m_start_id, params.m_max_count);
  session->shuttle_response(request_ids, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::
    on_load_managed_account_modification_request_ids(
      const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountModificationRequest::Id m_start_id;
    int m_max_count;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("start_id", m_start_id);
      shuttle.Shuttle("max_count", m_max_count);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto request_ids = clients.get_administration_client().
    load_managed_account_modification_request_ids(
      params.m_account, params.m_start_id, params.m_max_count);
  session->shuttle_response(request_ids, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_entitlement_modification(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto entitlement =
    clients.get_administration_client().load_entitlement_modification(
      params.m_id);
  session->shuttle_response(entitlement, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::
    on_submit_entitlement_modification_request(const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    EntitlementModification m_modification;
    Message m_comment;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("modification", m_modification);
      shuttle.Shuttle("comment", m_comment);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto modification = clients.get_administration_client().submit(
    params.m_account, params.m_modification, params.m_comment);
  session->shuttle_response(modification, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_risk_modification(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto risk_modification =
    clients.get_administration_client().load_risk_modification(params.m_id);
  session->shuttle_response(risk_modification, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_submit_risk_modification_request(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    RiskModification m_modification;
    Message m_comment;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("modification", m_modification);
      shuttle.Shuttle("comment", m_comment);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto modification = clients.get_administration_client().submit(
    params.m_account, params.m_modification, params.m_comment);
  session->shuttle_response(modification, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::
    on_load_account_modification_request_status(const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto status = clients.get_administration_client().
    load_account_modification_request_status(params.m_id);
  session->shuttle_response(status, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_approve_account_modification_request(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;
    Message m_comment;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
      shuttle.Shuttle("comment", m_comment);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto update =
    clients.get_administration_client().approve_account_modification_request(
      params.m_id, params.m_comment);
  session->shuttle_response(update, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_reject_account_modification_request(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;
    Message m_comment;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
      shuttle.Shuttle("comment", m_comment);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto update =
    clients.get_administration_client().reject_account_modification_request(
      params.m_id, params.m_comment);
  session->shuttle_response(update, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_message(
    const HttpRequest& request) {
  struct Parameters {
    Message::Id m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto message = clients.get_administration_client().load_message(params.m_id);
  session->shuttle_response(message, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::on_load_message_ids(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto message_ids =
    clients.get_administration_client().load_message_ids(params.m_id);
  session->shuttle_response(message_ids, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::
    on_send_account_modification_request_message(const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;
    Message m_message;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
      shuttle.Shuttle("message", m_message);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto message = clients.get_administration_client().
    send_account_modification_request_message(params.m_id, params.m_message);
  session->shuttle_response(message, Store(response));
  return response;
}
