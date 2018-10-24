#include "web_portal/web_portal/service_locator_web_servlet.hpp"
#include <Beam/Utilities/Trie.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "web_portal/web_portal/web_portal_session.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::WebServices;
using namespace boost;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::WebPortal;
using namespace std;

ServiceLocatorWebServlet::ServiceLocatorWebServlet(
    Ref<SessionStore<WebPortalSession>> sessions,
    Ref<ApplicationServiceClients> serviceClients)
    : m_sessions{sessions.Get()},
      m_serviceClients{serviceClients.Get()} {}

ServiceLocatorWebServlet::~ServiceLocatorWebServlet() {
  Close();
}

vector<HttpRequestSlot> ServiceLocatorWebServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/login"),
    std::bind(&ServiceLocatorWebServlet::OnLogin, this, std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/logout"),
    std::bind(&ServiceLocatorWebServlet::OnLogout, this,
    std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/load_current_account"),
    std::bind(&ServiceLocatorWebServlet::OnLoadCurrentAccount, this,
    std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/store_password"),
    std::bind(&ServiceLocatorWebServlet::OnStorePassword, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/service_locator/search_directory_entry"),
    std::bind(&ServiceLocatorWebServlet::OnSearchDirectoryEntry, this,
    std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/create_account"),
    std::bind(&ServiceLocatorWebServlet::OnCreateAccount, this,
    std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/create_group"),
    std::bind(&ServiceLocatorWebServlet::OnCreateGroup, this,
    std::placeholders::_1));
  return slots;
}

void ServiceLocatorWebServlet::Open() {
  if(m_openState.SetOpening()) {
    return;
  }
  try {
    m_serviceClients->Open();
  } catch(const std::exception&) {
    m_openState.SetOpenFailure();
    Shutdown();
  }
  m_openState.SetOpen();
}

void ServiceLocatorWebServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void ServiceLocatorWebServlet::Shutdown() {
  m_openState.SetClosed();
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
  HttpResponse response;
  auto session = m_sessions->Get(request, Store(response));
  if(session->IsLoggedIn()) {
    response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto account =
    m_serviceClients->GetServiceLocatorClient().AuthenticateAccount(
    parameters.m_username, parameters.m_password);
  if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  session->ShuttleResponse(account, Store(response));
  session->SetAccount(account);
  return response;
}

HttpResponse ServiceLocatorWebServlet::OnLogout(const HttpRequest& request) {
  HttpResponse response;
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
  HttpResponse response;
  auto session = m_sessions->Find(request);
  auto account = [&] {
    if(session == nullptr || !session->IsLoggedIn()) {
      return DirectoryEntry{};
    }
    return session->GetAccount();
  }();
  if(session != nullptr) {
    session->ShuttleResponse(account, Store(response));
  } else {
    response.SetHeader({"Content-Type", "application/json"});
    JsonSender<SharedBuffer> sender;
    response.SetBody(Encode<SharedBuffer>(sender, account));
  }
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
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  if(parameters.m_account != session->GetAccount()) {
    auto roles = m_serviceClients->GetAdministrationClient().LoadAccountRoles(
      session->GetAccount());
    if(!roles.Test(AccountRole::ADMINISTRATOR)) {
      response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
      return response;
    }
  }
  m_serviceClients->GetServiceLocatorClient().StorePassword(
    parameters.m_account, parameters.m_password);
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
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  to_lower(parameters.m_name);
  trim(parameters.m_name);
  vector<ResultEntry> result;
  if(parameters.m_name.empty()) {
    session->ShuttleResponse(result, Store(response));
    return response;
  }
  auto managedTradingGroups =
    m_serviceClients->GetAdministrationClient().LoadManagedTradingGroups(
    session->GetAccount());
  for(auto& managedTradingGroup : managedTradingGroups) {
    auto group = m_serviceClients->GetAdministrationClient().LoadTradingGroup(
      managedTradingGroup);
    if(starts_with(to_lower_copy(group.GetEntry().m_name), parameters.m_name)) {
      result.push_back(
        ResultEntry{group.GetEntry(), AccountRoles{0}, group.GetEntry()});
    }
    for(auto& manager : group.GetManagers()) {
      if(starts_with(to_lower_copy(manager.m_name), parameters.m_name)) {
        auto roles =
          m_serviceClients->GetAdministrationClient().LoadAccountRoles(manager);
        result.push_back(
          ResultEntry{manager, roles, group.GetEntry()});
      }
    }
    for(auto& trader : group.GetTraders()) {
      if(starts_with(to_lower_copy(trader.m_name), parameters.m_name)) {
        auto roles =
          m_serviceClients->GetAdministrationClient().LoadAccountRoles(trader);
        result.push_back(
          ResultEntry{trader, roles, group.GetEntry()});
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
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto roles = m_serviceClients->GetAdministrationClient().LoadAccountRoles(
    session->GetAccount());
  if(!roles.Test(AccountRole::ADMINISTRATOR)) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto validatedGroup =
    m_serviceClients->GetServiceLocatorClient().LoadDirectoryEntry(
    parameters.m_group.m_id);
  if(validatedGroup != parameters.m_group) {
    response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  DirectoryEntry newAccount;
  auto groupChildren = m_serviceClients->GetServiceLocatorClient().LoadChildren(
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
    newAccount = m_serviceClients->GetServiceLocatorClient().MakeAccount(
      parameters.m_name, "1234", *managerGroup);
    m_serviceClients->GetServiceLocatorClient().StorePermissions(newAccount,
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
      newAccount = m_serviceClients->GetServiceLocatorClient().MakeAccount(
        parameters.m_name, "1234", *traderGroup);
    } else {
      m_serviceClients->GetServiceLocatorClient().Associate(newAccount,
        *traderGroup);
    }
  }
  m_serviceClients->GetAdministrationClient().StoreIdentity(newAccount,
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
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto roles = m_serviceClients->GetAdministrationClient().LoadAccountRoles(
    session->GetAccount());
  if(!roles.Test(AccountRole::ADMINISTRATOR)) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto tradingGroupsDirectory =
    m_serviceClients->GetServiceLocatorClient().LoadDirectoryEntry(
    DirectoryEntry::GetStarDirectory(), "trading_groups");
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto newGroup = m_serviceClients->GetServiceLocatorClient().MakeDirectory(
    parameters.m_name, tradingGroupsDirectory);
  auto managersGroup =
    m_serviceClients->GetServiceLocatorClient().MakeDirectory("managers",
    newGroup);
  auto tradersGroup =
    m_serviceClients->GetServiceLocatorClient().MakeDirectory("traders",
    newGroup);
  session->ShuttleResponse(newGroup, Store(response));
  return response;
}
