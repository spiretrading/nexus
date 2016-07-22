#include "ClientWebPortal/ClientWebPortal/ClientWebPortalServlet.hpp"
#include <Beam/Json/JsonParser.hpp>
#include <Beam/Serialization/ShuttleBitset.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "ClientWebPortal/ClientWebPortal/ServiceClients.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::WebServices;
using namespace Nexus;
using namespace Nexus::ClientWebPortal;
using namespace std;

ClientWebPortalServlet::ClientWebPortalServlet(
    RefType<ServiceClients> serviceClients)
    : m_fileStore{"webapp"},
      m_serviceClients{serviceClients.Get()} {}

ClientWebPortalServlet::~ClientWebPortalServlet() {
  Close();
}

vector<HttpRequestSlot> ClientWebPortalServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/login"),
    bind(&ClientWebPortalServlet::OnLogin, this, std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/logout"),
    bind(&ClientWebPortalServlet::OnLogout, this, std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/load_current_account"),
    bind(&ClientWebPortalServlet::OnLoadCurrentAccount, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_trading_group"),
    bind(&ClientWebPortalServlet::OnLoadTradingGroup, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_managed_trading_groups"),
    bind(&ClientWebPortalServlet::OnLoadManagedTradingGroups, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_roles"),
    bind(&ClientWebPortalServlet::OnLoadAccountRoles, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_identity"),
    bind(&ClientWebPortalServlet::OnLoadAccountIdentity, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, "/"),
    bind(&ClientWebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, ""),
    bind(&ClientWebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, "/index.html"),
    bind(&ClientWebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchAny(HttpMethod::GET),
    bind(&ClientWebPortalServlet::OnServeFile, this, std::placeholders::_1));
  return slots;
}

void ClientWebPortalServlet::Open() {
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

void ClientWebPortalServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void ClientWebPortalServlet::Shutdown() {
  m_serviceClients->Close();
  m_openState.SetClosed();
}

HttpResponse ClientWebPortalServlet::OnIndex(const HttpRequest& request) {
  HttpResponse response;
  m_fileStore.Serve("index.html", Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnServeFile(const HttpRequest& request) {
  return m_fileStore.Serve(request);
}

HttpResponse ClientWebPortalServlet::OnLoadCurrentAccount(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
  auto account = [&] {
    if(session == nullptr || !session->IsLoggedIn()) {
      return DirectoryEntry{};
    }
    return session->GetAccount();
  }();
  response.SetHeader({"Content-Type", "application/json"});
  response.SetBody(Encode<SharedBuffer>(m_sender, account));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLogin(const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Get(request, Store(response));
  if(session->IsLoggedIn()) {
    response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  auto parameters = boost::get<JsonObject>(
    Parse<JsonParser>(request.GetBody()));
  auto& username = boost::get<string>(parameters["username"]);
  auto& password = boost::get<string>(parameters["password"]);
  auto account =
    m_serviceClients->GetServiceLocatorClient().AuthenticateAccount(username,
    password);
  if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  response.SetHeader({"Content-Type", "application/json"});
  response.SetBody(Encode<SharedBuffer>(m_sender, account));
  session->SetAccount(account);
  return response;
}

HttpResponse ClientWebPortalServlet::OnLogout(const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr || !session->IsLoggedIn()) {
    response.SetStatusCode(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  m_sessions.End(*session);
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadTradingGroup(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = boost::get<JsonObject>(
    Parse<JsonParser>(request.GetBody()));
  auto& directoryEntryParameter = boost::get<JsonObject>(
    parameters["directory_entry"]);
  DirectoryEntry directoryEntry;
  directoryEntry.m_name = boost::get<string>(directoryEntryParameter["name"]);
  directoryEntry.m_id = static_cast<int>(boost::get<int64_t>(
    directoryEntryParameter["id"]));
  directoryEntry.m_type = static_cast<DirectoryEntry::Type>(
    static_cast<int>(boost::get<int64_t>(directoryEntryParameter["type"])));
  response.SetHeader({"Content-Type", "application/json"});
  auto tradingGroup =
    m_serviceClients->GetAdministrationClient().LoadTradingGroup(
    directoryEntry);
  response.SetBody(Encode<SharedBuffer>(m_sender, tradingGroup));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadManagedTradingGroups(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = boost::get<JsonObject>(
    Parse<JsonParser>(request.GetBody()));
  auto& accountParameter = boost::get<JsonObject>(parameters["account"]);
  DirectoryEntry account;
  account.m_name = boost::get<string>(accountParameter["name"]);
  account.m_id = static_cast<int>(boost::get<int64_t>(accountParameter["id"]));
  account.m_type = static_cast<DirectoryEntry::Type>(
    static_cast<int>(boost::get<int64_t>(accountParameter["type"])));
  response.SetHeader({"Content-Type", "application/json"});
  auto tradingGroups =
    m_serviceClients->GetAdministrationClient().LoadManagedTradingGroups(
    account);
  response.SetBody(Encode<SharedBuffer>(m_sender, tradingGroups));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadAccountRoles(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = boost::get<JsonObject>(
    Parse<JsonParser>(request.GetBody()));
  auto& accountParameter = boost::get<JsonObject>(parameters["account"]);
  DirectoryEntry account;
  account.m_name = boost::get<string>(accountParameter["name"]);
  account.m_id = static_cast<int>(boost::get<int64_t>(accountParameter["id"]));
  account.m_type = static_cast<DirectoryEntry::Type>(
    static_cast<int>(boost::get<int64_t>(accountParameter["type"])));
  response.SetHeader({"Content-Type", "application/json"});
  auto roles = m_serviceClients->GetAdministrationClient().LoadAccountRoles(
    account);
  response.SetBody(Encode<SharedBuffer>(m_sender, roles));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadAccountIdentity(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = boost::get<JsonObject>(
    Parse<JsonParser>(request.GetBody()));
  auto& accountParameter = boost::get<JsonObject>(parameters["account"]);
  DirectoryEntry account;
  account.m_name = boost::get<string>(accountParameter["name"]);
  account.m_id = static_cast<int>(boost::get<int64_t>(accountParameter["id"]));
  account.m_type = static_cast<DirectoryEntry::Type>(
    static_cast<int>(boost::get<int64_t>(accountParameter["type"])));
  response.SetHeader({"Content-Type", "application/json"});
  auto identity = m_serviceClients->GetAdministrationClient().LoadIdentity(
    account);
  response.SetBody(Encode<SharedBuffer>(m_sender, identity));
  return response;
}
