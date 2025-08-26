#include "WebPortal/ServiceLocatorWebServlet.hpp"
#include <Beam/Collections/Trie.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::WebServices;
using namespace boost;
using namespace Nexus;

ServiceLocatorWebServlet::ServiceLocatorWebServlet(
  Ref<SessionStore<WebPortalSession>> sessions, ClientsBuilder clients_builder)
  : m_sessions(sessions.Get()),
    m_clients_builder(std::move(clients_builder)) {}

ServiceLocatorWebServlet::~ServiceLocatorWebServlet() {
  close();
}

std::vector<HttpRequestSlot> ServiceLocatorWebServlet::get_slots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/login"),
    std::bind_front(&ServiceLocatorWebServlet::on_login, this));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/logout"),
    std::bind_front(&ServiceLocatorWebServlet::on_logout, this));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/load_current_account"),
    std::bind_front(&ServiceLocatorWebServlet::on_load_current_account, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/service_locator/load_directory_entry_from_id"),
    std::bind_front(
      &ServiceLocatorWebServlet::on_load_directory_entry_from_id, this));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/store_password"),
    std::bind_front(&ServiceLocatorWebServlet::on_store_password, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/service_locator/search_directory_entry"),
    std::bind_front(
      &ServiceLocatorWebServlet::on_search_directory_entry, this));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/create_account"),
    std::bind_front(&ServiceLocatorWebServlet::on_create_account, this));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/create_group"),
    std::bind_front(&ServiceLocatorWebServlet::on_create_group, this));
  return slots;
}

void ServiceLocatorWebServlet::close() {
  m_open_state.Close();
}

HttpResponse ServiceLocatorWebServlet::on_login(const HttpRequest& request) {
  struct Parameters {
    std::string m_username;
    std::string m_password;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("username", m_username);
      shuttle.Shuttle("password", m_password);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Get(request, Store(response));
  if(session->IsLoggedIn()) {
    response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  auto parameters = session->shuttle_parameters<Parameters>(request);
  try {
    auto clients = m_clients_builder(parameters.m_username, parameters.m_password);
    auto account = clients.get_service_locator_client().GetAccount();
    session->set_clients(std::move(clients));
    session->shuttle_response(account, Store(response));
    session->SetAccount(account);
  } catch(const std::exception&) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  return response;
}

HttpResponse ServiceLocatorWebServlet::on_logout(const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session || !session->IsLoggedIn()) {
    response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  m_sessions->End(*session);
  return response;
}

HttpResponse ServiceLocatorWebServlet::on_load_current_account(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  auto account = [&] {
    if(!session || !session->IsLoggedIn()) {
      return DirectoryEntry();
    }
    return session->GetAccount();
  }();
  if(session) {
    session->shuttle_response(account, Store(response));
  } else {
    response.SetHeader({"Content-Type", "application/json"});
    auto sender = JsonSender<SharedBuffer>();
    response.SetBody(Encode<SharedBuffer>(sender, account));
  }
  return response;
}

HttpResponse ServiceLocatorWebServlet::on_load_directory_entry_from_id(
    const HttpRequest& request) {
  struct Parameters {
    unsigned int m_id;

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
  auto parameters = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto result =
    clients.get_service_locator_client().LoadDirectoryEntry(parameters.m_id);
  session->shuttle_response(result, Store(response));
  return response;
}

HttpResponse ServiceLocatorWebServlet::on_store_password(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    std::string m_password;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("password", m_password);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  clients.get_service_locator_client().StorePassword(
    parameters.m_account, parameters.m_password);
  return response;
}

HttpResponse ServiceLocatorWebServlet::on_search_directory_entry(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_name;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("name", m_name);
    }
  };
  struct ResultEntry {
    DirectoryEntry m_directory_entry;
    AccountRoles m_roles;
    DirectoryEntry m_group;

    void Shuttle(JsonSender<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directory_entry);
      shuttle.Shuttle("roles", m_roles);
      shuttle.Shuttle("group", m_group);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  to_lower(parameters.m_name);
  trim(parameters.m_name);
  auto result = std::vector<ResultEntry>();
  if(parameters.m_name.empty()) {
    session->shuttle_response(result, Store(response));
    return response;
  }
  auto managed_trading_groups =
    clients.get_administration_client().load_managed_trading_groups(
      session->GetAccount());
  for(auto& managed_trading_group : managed_trading_groups) {
    auto group = clients.get_administration_client().load_trading_group(
      managed_trading_group);
    if(starts_with(
        to_lower_copy(group.get_entry().m_name), parameters.m_name)) {
      result.emplace_back(
        group.get_entry(), AccountRoles(0), group.get_entry());
    }
    for(auto& manager : group.get_managers()) {
      if(starts_with(to_lower_copy(manager.m_name), parameters.m_name)) {
        auto roles =
          clients.get_administration_client().load_account_roles(manager);
        result.emplace_back(manager, roles, group.get_entry());
      }
    }
    for(auto& trader : group.get_traders()) {
      if(starts_with(to_lower_copy(trader.m_name), parameters.m_name)) {
        auto roles =
          clients.get_administration_client().load_account_roles(trader);
        result.emplace_back(trader, roles, group.get_entry());
      }
    }
  }
  auto roles = clients.get_administration_client().load_account_roles(
    session->GetAccount());
  if(roles.Test(AccountRole::ADMINISTRATOR)) {
    auto organization_roles = AccountRoles();
    organization_roles.Set(AccountRole::SERVICE);
    organization_roles.Set(AccountRole::ADMINISTRATOR);
    auto organization_entries =
      clients.get_administration_client().load_accounts_by_roles(
        organization_roles);
    auto organization_entry =
      clients.get_administration_client().load_trading_groups_root_entry();
    organization_entry.m_name =
      clients.get_definitions_client().load_organization_name();
    for(auto& entry : organization_entries) {
      if(starts_with(to_lower_copy(entry.m_name), parameters.m_name)) {
        auto roles =
          clients.get_administration_client().load_account_roles(entry);
        result.emplace_back(entry, roles, organization_entry);
      }
    }
  }
  session->shuttle_response(result, Store(response));
  return response;
}

HttpResponse ServiceLocatorWebServlet::on_create_account(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_name;
    DirectoryEntry m_group;
    AccountIdentity m_identity;
    AccountRoles m_account_roles;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("name", m_name);
      shuttle.Shuttle("group", m_group);
      shuttle.Shuttle("identity", m_identity);
      shuttle.Shuttle("roles", m_account_roles);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto validated_group =
    clients.get_service_locator_client().LoadDirectoryEntry(
      parameters.m_group.m_id);
  if(validated_group != parameters.m_group) {
    response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  auto new_account = DirectoryEntry();
  auto group_children =
    clients.get_service_locator_client().LoadChildren(validated_group);
  if(parameters.m_account_roles.Test(AccountRole::MANAGER)) {
    auto manager_group = std::find_if(
      group_children.begin(), group_children.end(), [] (const auto& child) {
        return child.m_name == "managers";
      });
    if(manager_group == group_children.end()) {
      response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
      return response;
    }
    new_account = clients.get_service_locator_client().MakeAccount(
      parameters.m_name, "1234", *manager_group);
    clients.get_service_locator_client().StorePermissions(
      new_account, validated_group, Permission::READ);
  }
  if(parameters.m_account_roles.Test(AccountRole::TRADER)) {
    auto trader_group = std::find_if(
      group_children.begin(), group_children.end(), [] (const auto& child) {
        return child.m_name == "traders";
      });
    if(trader_group == group_children.end()) {
      response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
      return response;
    }
    if(new_account.m_id == -1) {
      new_account = clients.get_service_locator_client().MakeAccount(
        parameters.m_name, "1234", *trader_group);
    } else {
      clients.get_service_locator_client().Associate(
        new_account, *trader_group);
    }
  }
  clients.get_administration_client().store(new_account, parameters.m_identity);
  session->shuttle_response(new_account, Store(response));
  return response;
}

HttpResponse ServiceLocatorWebServlet::on_create_group(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_name;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("name", m_name);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto trading_groups_directory =
    clients.get_service_locator_client().LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "trading_groups");
  auto parameters = session->shuttle_parameters<Parameters>(request);
  auto new_group = clients.get_service_locator_client().MakeDirectory(
    parameters.m_name, trading_groups_directory);
  auto managers_group =
    clients.get_service_locator_client().MakeDirectory("managers", new_group);
  auto traders_group =
    clients.get_service_locator_client().MakeDirectory("traders", new_group);
  session->shuttle_response(new_group, Store(response));
  return response;
}
