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
using namespace Nexus::AdministrationService;
using namespace Nexus::WebPortal;

ServiceLocatorWebServlet::ServiceLocatorWebServlet(
  Ref<SessionStore<WebPortalSession>> sessions,
  ServiceClientsBuilder serviceClientsBuilder)
  : m_sessions(sessions.Get()),
    m_serviceClientsBuilder(std::move(serviceClientsBuilder)) {}

ServiceLocatorWebServlet::~ServiceLocatorWebServlet() {
  Close();
}

std::vector<HttpRequestSlot> ServiceLocatorWebServlet::GetSlots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/login"),
    std::bind_front(&ServiceLocatorWebServlet::OnLogin, this));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/logout"),
    std::bind_front(&ServiceLocatorWebServlet::OnLogout, this));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/load_current_account"),
    std::bind_front(&ServiceLocatorWebServlet::OnLoadCurrentAccount, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/service_locator/load_directory_entry_from_id"), std::bind_front(
      &ServiceLocatorWebServlet::OnLoadDirectoryEntryFromId, this));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/store_password"),
    std::bind_front(&ServiceLocatorWebServlet::OnStorePassword, this));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/service_locator/search_directory_entry"),
    std::bind_front(&ServiceLocatorWebServlet::OnSearchDirectoryEntry, this));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/create_account"),
    std::bind_front(&ServiceLocatorWebServlet::OnCreateAccount, this));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/create_group"),
    std::bind_front(&ServiceLocatorWebServlet::OnCreateGroup, this));
  return slots;
}

void ServiceLocatorWebServlet::Close() {
  m_openState.Close();
}

HttpResponse ServiceLocatorWebServlet::OnLogin(const HttpRequest& request) {
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
  auto parameters = session->ShuttleParameters<Parameters>(request);
  try {
    auto serviceClients = m_serviceClientsBuilder(parameters.m_username,
      parameters.m_password);
    auto account = serviceClients.GetServiceLocatorClient().GetAccount();
    session->SetServiceClients(std::move(serviceClients));
    session->ShuttleResponse(account, Store(response));
    session->SetAccount(account);
  } catch(const std::exception&) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  return response;
}

HttpResponse ServiceLocatorWebServlet::OnLogout(const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr || !session->IsLoggedIn()) {
    response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  m_sessions->End(*session);
  return response;
}

HttpResponse ServiceLocatorWebServlet::OnLoadCurrentAccount(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  auto account = [&] {
    if(session == nullptr || !session->IsLoggedIn()) {
      return DirectoryEntry();
    }
    return session->GetAccount();
  }();
  if(session != nullptr) {
    session->ShuttleResponse(account, Store(response));
  } else {
    response.SetHeader({"Content-Type", "application/json"});
    auto sender = JsonSender<SharedBuffer>();
    response.SetBody(Encode<SharedBuffer>(sender, account));
  }
  return response;
}

HttpResponse ServiceLocatorWebServlet::OnLoadDirectoryEntryFromId(
    const HttpRequest& request) {
  struct Parameters {
    unsigned int m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto& serviceClients = session->GetServiceClients();
  auto result = serviceClients.GetServiceLocatorClient().LoadDirectoryEntry(
    parameters.m_id);
  session->ShuttleResponse(result, Store(response));
  return response;
}

HttpResponse ServiceLocatorWebServlet::OnStorePassword(
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
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto& serviceClients = session->GetServiceClients();
  serviceClients.GetServiceLocatorClient().StorePassword(parameters.m_account,
    parameters.m_password);
  return response;
}

HttpResponse ServiceLocatorWebServlet::OnSearchDirectoryEntry(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_name;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("name", m_name);
    }
  };
  struct ResultEntry {
    DirectoryEntry m_directoryEntry;
    AccountRoles m_roles;
    DirectoryEntry m_group;

    void Shuttle(JsonSender<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directoryEntry);
      shuttle.Shuttle("roles", m_roles);
      shuttle.Shuttle("group", m_group);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto& serviceClients = session->GetServiceClients();
  to_lower(parameters.m_name);
  trim(parameters.m_name);
  auto result = std::vector<ResultEntry>();
  if(parameters.m_name.empty()) {
    session->ShuttleResponse(result, Store(response));
    return response;
  }
  auto managedTradingGroups =
    serviceClients.GetAdministrationClient().LoadManagedTradingGroups(
      session->GetAccount());
  for(auto& managedTradingGroup : managedTradingGroups) {
    auto group = serviceClients.GetAdministrationClient().LoadTradingGroup(
      managedTradingGroup);
    if(starts_with(to_lower_copy(group.GetEntry().m_name), parameters.m_name)) {
      result.push_back(
        ResultEntry(group.GetEntry(), AccountRoles(0), group.GetEntry()));
    }
    for(auto& manager : group.GetManagers()) {
      if(starts_with(to_lower_copy(manager.m_name), parameters.m_name)) {
        auto roles =
          serviceClients.GetAdministrationClient().LoadAccountRoles(manager);
        result.push_back(ResultEntry(manager, roles, group.GetEntry()));
      }
    }
    for(auto& trader : group.GetTraders()) {
      if(starts_with(to_lower_copy(trader.m_name), parameters.m_name)) {
        auto roles =
          serviceClients.GetAdministrationClient().LoadAccountRoles(trader);
        result.push_back(ResultEntry(trader, roles, group.GetEntry()));
      }
    }
  }
  auto roles = serviceClients.GetAdministrationClient().LoadAccountRoles(
    session->GetAccount());
  if(roles.Test(AccountRole::ADMINISTRATOR)) {
    auto organizationRoles = AccountRoles();
    organizationRoles.Set(AccountRole::SERVICE);
    organizationRoles.Set(AccountRole::ADMINISTRATOR);
    auto organizationEntries =
      serviceClients.GetAdministrationClient().LoadAccountsByRoles(
        organizationRoles);
    auto organizationEntry =
      serviceClients.GetAdministrationClient().LoadTradingGroupsRootEntry();
    organizationEntry.m_name =
      serviceClients.GetDefinitionsClient().LoadOrganizationName();
    for(auto& entry : organizationEntries) {
      if(starts_with(to_lower_copy(entry.m_name), parameters.m_name)) {
        auto roles =
          serviceClients.GetAdministrationClient().LoadAccountRoles(entry);
        result.push_back(ResultEntry(entry, roles, organizationEntry));
      }
    }
  }
  session->ShuttleResponse(result, Store(response));
  return response;
}

HttpResponse ServiceLocatorWebServlet::OnCreateAccount(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_name;
    DirectoryEntry m_group;
    AccountIdentity m_identity;
    AccountRoles m_accountRoles;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("name", m_name);
      shuttle.Shuttle("group", m_group);
      shuttle.Shuttle("identity", m_identity);
      shuttle.Shuttle("roles", m_accountRoles);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto& serviceClients = session->GetServiceClients();
  auto validatedGroup =
    serviceClients.GetServiceLocatorClient().LoadDirectoryEntry(
    parameters.m_group.m_id);
  if(validatedGroup != parameters.m_group) {
    response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  auto newAccount = DirectoryEntry();
  auto groupChildren = serviceClients.GetServiceLocatorClient().LoadChildren(
    validatedGroup);
  if(parameters.m_accountRoles.Test(AccountRole::MANAGER)) {
    auto managerGroup = std::find_if(groupChildren.begin(), groupChildren.end(),
      [] (const auto& child) {
        return child.m_name == "managers";
      });
    if(managerGroup == groupChildren.end()) {
      response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
      return response;
    }
    newAccount = serviceClients.GetServiceLocatorClient().MakeAccount(
      parameters.m_name, "1234", *managerGroup);
    serviceClients.GetServiceLocatorClient().StorePermissions(newAccount,
      validatedGroup, Permission::READ);
  }
  if(parameters.m_accountRoles.Test(AccountRole::TRADER)) {
    auto traderGroup = std::find_if(groupChildren.begin(), groupChildren.end(),
      [] (const auto& child) {
        return child.m_name == "traders";
      });
    if(traderGroup == groupChildren.end()) {
      response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
      return response;
    }
    if(newAccount.m_id == -1) {
      newAccount = serviceClients.GetServiceLocatorClient().MakeAccount(
        parameters.m_name, "1234", *traderGroup);
    } else {
      serviceClients.GetServiceLocatorClient().Associate(newAccount,
        *traderGroup);
    }
  }
  serviceClients.GetAdministrationClient().StoreIdentity(newAccount,
    parameters.m_identity);
  session->ShuttleResponse(newAccount, Store(response));
  return response;
}

HttpResponse ServiceLocatorWebServlet::OnCreateGroup(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_name;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("name", m_name);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& serviceClients = session->GetServiceClients();
  auto tradingGroupsDirectory =
    serviceClients.GetServiceLocatorClient().LoadDirectoryEntry(
    DirectoryEntry::GetStarDirectory(), "trading_groups");
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto newGroup = serviceClients.GetServiceLocatorClient().MakeDirectory(
    parameters.m_name, tradingGroupsDirectory);
  auto managersGroup = serviceClients.GetServiceLocatorClient().MakeDirectory(
    "managers", newGroup);
  auto tradersGroup = serviceClients.GetServiceLocatorClient().MakeDirectory(
    "traders", newGroup);
  session->ShuttleResponse(newGroup, Store(response));
  return response;
}
