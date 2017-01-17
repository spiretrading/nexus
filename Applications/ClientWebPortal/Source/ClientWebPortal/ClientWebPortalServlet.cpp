#include "ClientWebPortal/ClientWebPortal/ClientWebPortalServlet.hpp"
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Serialization/ShuttleBitset.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Utilities/Trie.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "ClientWebPortal/ClientWebPortal/ServiceClients.hpp"
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/Compliance/VirtualComplianceClient.hpp"
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::WebServices;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::AdministrationService;
using namespace Nexus::ClientWebPortal;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace std;

namespace {
  using WebPosition = Nexus::Accounting::Position<Nexus::Security>;
  using WebInventory = Nexus::Accounting::Inventory<WebPosition>;
  using WebBookkeeper = Nexus::Accounting::TrueAverageBookkeeper<WebInventory>;
  using WebPortfolio = Nexus::Accounting::Portfolio<WebBookkeeper>;
}

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
    std::bind(&ClientWebPortalServlet::OnLogin, this, std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/logout"),
    std::bind(&ClientWebPortalServlet::OnLogout, this, std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/create_account"),
    std::bind(&ClientWebPortalServlet::OnCreateAccount, this,
    std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/create_group"),
    std::bind(&ClientWebPortalServlet::OnCreateGroup, this,
    std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/load_current_account"),
    std::bind(&ClientWebPortalServlet::OnLoadCurrentAccount, this,
    std::placeholders::_1));
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/store_password"),
    std::bind(&ClientWebPortalServlet::OnStorePassword, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/service_locator/search_directory_entry"),
    std::bind(&ClientWebPortalServlet::OnSearchDirectoryEntry, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_trading_group"),
    std::bind(&ClientWebPortalServlet::OnLoadTradingGroup, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_managed_trading_groups"),
    std::bind(&ClientWebPortalServlet::OnLoadManagedTradingGroups, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_roles"),
    std::bind(&ClientWebPortalServlet::OnLoadAccountRoles, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_identity"),
    std::bind(&ClientWebPortalServlet::OnLoadAccountIdentity, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/store_account_identity"),
    std::bind(&ClientWebPortalServlet::OnStoreAccountIdentity, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_entitlements_database"),
    std::bind(&ClientWebPortalServlet::OnLoadEntitlementsDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_entitlements"),
    std::bind(&ClientWebPortalServlet::OnLoadAccountEntitlements, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/store_account_entitlements"),
    std::bind(&ClientWebPortalServlet::OnStoreAccountEntitlements, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_risk_parameters"),
    std::bind(&ClientWebPortalServlet::OnLoadRiskParameters, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/store_risk_parameters"),
    std::bind(&ClientWebPortalServlet::OnStoreRiskParameters, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/compliance_service/load_directory_entry_compliance_rule_entry"),
    std::bind(&ClientWebPortalServlet::OnLoadDirectoryEntryComplianceRuleEntry,
    this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_compliance_rule_schemas"),
    std::bind(&ClientWebPortalServlet::OnLoadComplianceRuleSchemas, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_country_database"),
    std::bind(&ClientWebPortalServlet::OnLoadCountryDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_currency_database"),
    std::bind(&ClientWebPortalServlet::OnLoadCurrencyDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_market_database"),
    std::bind(&ClientWebPortalServlet::OnLoadMarketDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/market_data_service/load_security_info_from_prefix"),
    std::bind(&ClientWebPortalServlet::OnLoadSecurityInfoFromPrefix, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/reporting_service/load_profit_and_loss_report"),
    std::bind(&ClientWebPortalServlet::OnLoadProfitAndLossReport, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, "/"),
    std::bind(&ClientWebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, ""),
    std::bind(&ClientWebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, "/index.html"),
    std::bind(&ClientWebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchAny(HttpMethod::GET),
    std::bind(&ClientWebPortalServlet::OnServeFile, this,
    std::placeholders::_1));
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
  auto response = m_fileStore.Serve(request);
  if(response.GetStatusCode() == HttpStatusCode::NOT_FOUND) {
    return OnIndex(request);
  }
  return response;
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
  if(session != nullptr) {
    session->ShuttleResponse(account, Store(response));
  } else {
    response.SetHeader({"Content-Type", "application/json"});
    JsonSender<SharedBuffer> sender;
    response.SetBody(Encode<SharedBuffer>(sender, account));
  }
  return response;
}

HttpResponse ClientWebPortalServlet::OnStorePassword(
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
  auto session = m_sessions.Find(request);
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

HttpResponse ClientWebPortalServlet::OnSearchDirectoryEntry(
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
  auto session = m_sessions.Find(request);
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

HttpResponse ClientWebPortalServlet::OnLogin(const HttpRequest& request) {
  struct Parameters {
    std::string m_username;
    std::string m_password;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("username", m_username);
      shuttle.Shuttle("password", m_password);
    }
  };
  HttpResponse response;
  auto session = m_sessions.Get(request, Store(response));
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

HttpResponse ClientWebPortalServlet::OnCreateAccount(
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

HttpResponse ClientWebPortalServlet::OnCreateGroup(const HttpRequest& request) {
  struct Parameters {
    std::string m_name;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("name", m_name);
    }
  };
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

HttpResponse ClientWebPortalServlet::OnLoadTradingGroup(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_directoryEntry;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directoryEntry);
    }
  };
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto tradingGroup =
    m_serviceClients->GetAdministrationClient().LoadTradingGroup(
    parameters.m_directoryEntry);
  session->ShuttleResponse(tradingGroup, Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadManagedTradingGroups(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto tradingGroups =
    m_serviceClients->GetAdministrationClient().LoadManagedTradingGroups(
    parameters.m_account);
  session->ShuttleResponse(tradingGroups, Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadAccountRoles(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto roles = m_serviceClients->GetAdministrationClient().LoadAccountRoles(
    parameters.m_account);
  session->ShuttleResponse(roles, Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadAccountIdentity(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto identity = m_serviceClients->GetAdministrationClient().LoadIdentity(
    parameters.m_account);
  session->ShuttleResponse(identity, Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnStoreAccountIdentity(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountIdentity m_identity;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("identity", m_identity);
    }
  };
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
  auto parameters = session->ShuttleParameters<Parameters>(request);
  m_serviceClients->GetAdministrationClient().StoreIdentity(
    parameters.m_account, parameters.m_identity);
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
  auto database =
    m_serviceClients->GetAdministrationClient().LoadEntitlements();
  session->ShuttleResponse(database, Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadAccountEntitlements(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto entitlements =
    m_serviceClients->GetAdministrationClient().LoadEntitlements(
    parameters.m_account);
  session->ShuttleResponse(entitlements, Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnStoreAccountEntitlements(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    vector<DirectoryEntry> m_entitlements;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("entitlements", m_entitlements);
    }
  };
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
  auto parameters = session->ShuttleParameters<Parameters>(request);
  m_serviceClients->GetAdministrationClient().StoreEntitlements(
    parameters.m_account, parameters.m_entitlements);
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadRiskParameters(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto queue = std::make_shared<Queue<RiskParameters>>();
  m_serviceClients->GetAdministrationClient().GetRiskParametersPublisher(
    parameters.m_account).Monitor(queue);
  auto riskParameters = queue->Top();
  session->ShuttleResponse(riskParameters, Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnStoreRiskParameters(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    RiskParameters m_riskParameters;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("risk_parameters", m_riskParameters);
    }
  };
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
  auto parameters = session->ShuttleParameters<Parameters>(request);
  m_serviceClients->GetAdministrationClient().StoreRiskParameters(
    parameters.m_account, parameters.m_riskParameters);
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadDirectoryEntryComplianceRuleEntry(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_directoryEntry;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directoryEntry);
    }
  };
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto rules = m_serviceClients->GetComplianceClient().Load(
    parameters.m_directoryEntry);
  session->ShuttleResponse(rules, Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadComplianceRuleSchemas(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto schemas =
    m_serviceClients->GetDefinitionsClient().LoadComplianceRuleSchemas();
  session->ShuttleResponse(schemas, Store(response));
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
  auto database =
    m_serviceClients->GetDefinitionsClient().LoadCountryDatabase();
  session->ShuttleResponse(database, Store(response));
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
  auto database =
    m_serviceClients->GetDefinitionsClient().LoadCurrencyDatabase();
  session->ShuttleResponse(database, Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadMarketDatabase(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto database =
    m_serviceClients->GetDefinitionsClient().LoadMarketDatabase();
  session->ShuttleResponse(database, Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadSecurityInfoFromPrefix(
    const HttpRequest& request) {
  struct Parameters {
    string m_prefix;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("prefix", m_prefix);
    }
  };
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto securityInfos =
    m_serviceClients->GetMarketDataClient().LoadSecurityInfoFromPrefix(
    parameters.m_prefix);
  session->ShuttleResponse(securityInfos, Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadProfitAndLossReport(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_directoryEntry;
    ptime m_startDate;
    ptime m_endDate;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directoryEntry);
      shuttle.Shuttle("start_date", m_startDate);
      shuttle.Shuttle("end_date", m_endDate);
    }
  };
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto orderQueue = std::make_shared<Queue<const Order*>>();
  auto marketDatabase =
    m_serviceClients->GetDefinitionsClient().LoadMarketDatabase();
  auto timeZoneDatabase =
    m_serviceClients->GetDefinitionsClient().LoadTimeZoneDatabase();
  QueryDailyOrderSubmissions(parameters.m_directoryEntry,
    parameters.m_startDate, parameters.m_endDate, marketDatabase,
    timeZoneDatabase, m_serviceClients->GetOrderExecutionClient(), orderQueue);
  WebPortfolio portfolio{marketDatabase};
  try {
    while(true) {
      auto order = orderQueue->Top();
      orderQueue->Pop();
      vector<ExecutionReport> executionReports;
      order->GetPublisher().WithSnapshot(
        [&] (auto snapshot) {
          if(snapshot.is_initialized()) {
            executionReports = *snapshot;
          }
        });
      for(auto& executionReport : executionReports) {
        portfolio.Update(order->GetInfo().m_fields, executionReport);
      }
    }
  } catch(const PipeBrokenException&) {}
  vector<WebInventory> inventories;
  for(auto& inventory : portfolio.GetBookkeeper().GetInventoryRange()) {
    inventories.push_back(inventory.second);
  }
  session->ShuttleResponse(inventories, Store(response));
  return response;
}
