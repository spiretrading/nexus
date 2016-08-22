#include "ClientWebPortal/ClientWebPortal/ClientWebPortalServlet.hpp"
#include <Beam/Json/JsonParser.hpp>
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Serialization/ShuttleBitset.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "ClientWebPortal/ClientWebPortal/ServiceClients.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::WebServices;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::ClientWebPortal;
using namespace Nexus::RiskService;
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
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/store_password"),
    bind(&ClientWebPortalServlet::OnStorePassword, this,
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
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/store_account_identity"),
    bind(&ClientWebPortalServlet::OnStoreAccountIdentity, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_entitlements_database"),
    bind(&ClientWebPortalServlet::OnLoadEntitlementsDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_entitlements"),
    bind(&ClientWebPortalServlet::OnLoadAccountEntitlements, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/store_account_entitlements"),
    bind(&ClientWebPortalServlet::OnStoreAccountEntitlements, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_risk_parameters"),
    bind(&ClientWebPortalServlet::OnLoadRiskParameters, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/store_risk_parameters"),
    bind(&ClientWebPortalServlet::OnStoreRiskParameters, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_country_database"),
    bind(&ClientWebPortalServlet::OnLoadCountryDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_currency_database"),
    bind(&ClientWebPortalServlet::OnLoadCurrencyDatabase, this,
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

HttpResponse ClientWebPortalServlet::OnStorePassword(
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
  if(account != session->GetAccount()) {
    auto roles = m_serviceClients->GetAdministrationClient().LoadAccountRoles(
      session->GetAccount());
    if(!roles.Test(AccountRole::ADMINISTRATOR)) {
      response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
      return response;
    }
  }
  auto password = boost::get<string>(parameters["password"]);
  m_serviceClients->GetServiceLocatorClient().StorePassword(account, password);
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

HttpResponse ClientWebPortalServlet::OnStoreAccountIdentity(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
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
  auto parameters = boost::get<JsonObject>(
    Parse<JsonParser>(request.GetBody()));
  auto& accountParameter = boost::get<JsonObject>(parameters["account"]);
  DirectoryEntry account;
  account.m_name = boost::get<string>(accountParameter["name"]);
  account.m_id = static_cast<int>(boost::get<int64_t>(accountParameter["id"]));
  account.m_type = static_cast<DirectoryEntry::Type>(
    static_cast<int>(boost::get<int64_t>(accountParameter["type"])));
  auto& identityParameter = boost::get<JsonObject>(parameters["identity"]);
  AccountIdentity identity;
  identity.m_firstName = boost::get<string>(identityParameter["first_name"]);
  identity.m_lastName = boost::get<string>(identityParameter["last_name"]);
  identity.m_emailAddress = boost::get<string>(identityParameter["e_mail"]);
  identity.m_addressLineOne = boost::get<string>(
    identityParameter["address_line_one"]);
  identity.m_addressLineTwo = boost::get<string>(
    identityParameter["address_line_two"]);
  identity.m_addressLineThree = boost::get<string>(
    identityParameter["address_line_three"]);
  identity.m_city = boost::get<string>(identityParameter["city"]);
  identity.m_province = boost::get<string>(identityParameter["province"]);
  identity.m_country = static_cast<uint16_t>(
    boost::get<int64_t>(identityParameter["country"]));
  identity.m_userNotes = boost::get<string>(identityParameter["user_notes"]);
  m_serviceClients->GetAdministrationClient().StoreIdentity(account, identity);
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadEntitlementsDatabase(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  response.SetHeader({"Content-Type", "application/json"});
  auto database =
    m_serviceClients->GetAdministrationClient().LoadEntitlements();
  response.SetBody(Encode<SharedBuffer>(m_sender, database));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadAccountEntitlements(
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
  auto entitlements =
    m_serviceClients->GetAdministrationClient().LoadEntitlements(account);
  response.SetBody(Encode<SharedBuffer>(m_sender, entitlements));
  return response;
}

HttpResponse ClientWebPortalServlet::OnStoreAccountEntitlements(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
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
  auto parameters = boost::get<JsonObject>(
    Parse<JsonParser>(request.GetBody()));
  auto& accountParameter = boost::get<JsonObject>(parameters["account"]);
  DirectoryEntry account;
  account.m_name = boost::get<string>(accountParameter["name"]);
  account.m_id = static_cast<int>(boost::get<int64_t>(accountParameter["id"]));
  account.m_type = static_cast<DirectoryEntry::Type>(
    static_cast<int>(boost::get<int64_t>(accountParameter["type"])));
  auto& entitlementsParameter = boost::get<std::vector<JsonValue>>(
    parameters["entitlements"]);
  vector<DirectoryEntry> entitlements;
  for(auto& entitlementValue : entitlementsParameter) {
    auto& entitlementParameter = boost::get<JsonObject>(entitlementValue);
    DirectoryEntry entitlement;
    entitlement.m_name = boost::get<string>(entitlementParameter["name"]);
    entitlement.m_id = static_cast<int>(boost::get<int64_t>(
      entitlementParameter["id"]));
    account.m_type = static_cast<DirectoryEntry::Type>(
      static_cast<int>(boost::get<int64_t>(entitlementParameter["type"])));
    entitlements.push_back(entitlement);
  }
  m_serviceClients->GetAdministrationClient().StoreEntitlements(account,
    entitlements);
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadRiskParameters(
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
  auto queue = std::make_shared<Queue<RiskParameters>>();
  m_serviceClients->GetAdministrationClient().GetRiskParametersPublisher(
    account).Monitor(queue);
  auto riskParameters = queue->Top();
  response.SetBody(Encode<SharedBuffer>(m_sender, riskParameters));
  return response;
}

HttpResponse ClientWebPortalServlet::OnStoreRiskParameters(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
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
  auto parameters = boost::get<JsonObject>(
    Parse<JsonParser>(request.GetBody()));
  auto& accountParameter = boost::get<JsonObject>(parameters["account"]);
  DirectoryEntry account;
  account.m_name = boost::get<string>(accountParameter["name"]);
  account.m_id = static_cast<int>(boost::get<int64_t>(accountParameter["id"]));
  account.m_type = static_cast<DirectoryEntry::Type>(
    static_cast<int>(boost::get<int64_t>(accountParameter["type"])));
  auto& riskParametersParameter =
    boost::get<JsonObject>(parameters["risk_parameters"]);
  RiskParameters riskParameters;
  m_serviceClients->GetAdministrationClient().StoreRiskParameters(account,
    riskParameters);
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadCountryDatabase(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  response.SetHeader({"Content-Type", "application/json"});
  auto database =
    m_serviceClients->GetDefinitionsClient().LoadCountryDatabase();
  response.SetBody(Encode<SharedBuffer>(m_sender, database));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadCurrencyDatabase(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  response.SetHeader({"Content-Type", "application/json"});
  auto database =
    m_serviceClients->GetDefinitionsClient().LoadCurrencyDatabase();
  response.SetBody(Encode<SharedBuffer>(m_sender, database));
  return response;
}
