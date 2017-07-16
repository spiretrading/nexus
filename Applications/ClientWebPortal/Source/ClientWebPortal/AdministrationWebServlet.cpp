#include "ClientWebPortal/ClientWebPortal/AdministrationWebServlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "ClientWebPortal/ClientWebPortal/ClientWebPortalSession.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::WebServices;
using namespace boost;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::ClientWebPortal;
using namespace Nexus::RiskService;
using namespace std;

AdministrationWebServlet::AdministrationWebServlet(
    RefType<SessionStore<ClientWebPortalSession>> sessions,
    RefType<ApplicationServiceClients> serviceClients)
    : m_sessions{sessions.Get()},
      m_serviceClients{serviceClients.Get()} {}

AdministrationWebServlet::~AdministrationWebServlet() {
  Close();
}

vector<HttpRequestSlot> AdministrationWebServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_trading_group"),
    std::bind(&AdministrationWebServlet::OnLoadTradingGroup, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_managed_trading_groups"),
    std::bind(&AdministrationWebServlet::OnLoadManagedTradingGroups, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_roles"),
    std::bind(&AdministrationWebServlet::OnLoadAccountRoles, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/store_account_roles"),
    std::bind(&AdministrationWebServlet::OnStoreAccountRoles, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_identity"),
    std::bind(&AdministrationWebServlet::OnLoadAccountIdentity, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/store_account_identity"),
    std::bind(&AdministrationWebServlet::OnStoreAccountIdentity, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_entitlements_database"),
    std::bind(&AdministrationWebServlet::OnLoadEntitlementsDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_entitlements"),
    std::bind(&AdministrationWebServlet::OnLoadAccountEntitlements, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/store_account_entitlements"),
    std::bind(&AdministrationWebServlet::OnStoreAccountEntitlements, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_risk_parameters"),
    std::bind(&AdministrationWebServlet::OnLoadRiskParameters, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/store_risk_parameters"),
    std::bind(&AdministrationWebServlet::OnStoreRiskParameters, this,
    std::placeholders::_1));
  return slots;
}

void AdministrationWebServlet::Open() {
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

void AdministrationWebServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void AdministrationWebServlet::Shutdown() {
  m_openState.SetClosed();
}

HttpResponse AdministrationWebServlet::OnLoadTradingGroup(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_directoryEntry;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directoryEntry);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
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

HttpResponse AdministrationWebServlet::OnLoadManagedTradingGroups(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
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

HttpResponse AdministrationWebServlet::OnLoadAccountRoles(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
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

HttpResponse AdministrationWebServlet::OnStoreAccountRoles(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountRoles m_roles;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("roles", m_roles);
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
  auto groups =
    m_serviceClients->GetAdministrationClient().LoadManagedTradingGroups(
    m_serviceClients->GetServiceLocatorClient().GetAccount());
  TradingGroup memberGroup;
  for(auto& group : groups) {
    auto tradingGroup =
      m_serviceClients->GetAdministrationClient().LoadTradingGroup(group);
    if(std::find(tradingGroup.GetManagers().begin(),
        tradingGroup.GetManagers().end(), parameters.m_account) !=
        tradingGroup.GetManagers().end() ||
        std::find(tradingGroup.GetTraders().begin(),
        tradingGroup.GetTraders().end(), parameters.m_account) !=
        tradingGroup.GetTraders().end()) {
      memberGroup = tradingGroup;
      break;
    }
  }
  auto previousRoles =
    m_serviceClients->GetAdministrationClient().LoadAccountRoles(
    parameters.m_account);
  if(memberGroup.GetEntry().m_type != DirectoryEntry::Type::DIRECTORY) {
    session->ShuttleResponse(previousRoles, Store(response));
    return response;
  }
  if(parameters.m_roles.Test(AccountRole::MANAGER) !=
      previousRoles.Test(AccountRole::MANAGER)) {
    if(parameters.m_roles.Test(AccountRole::MANAGER)) {
      m_serviceClients->GetServiceLocatorClient().StorePermissions(
        parameters.m_account, memberGroup.GetEntry(), Permission::READ);
      m_serviceClients->GetServiceLocatorClient().Associate(
        parameters.m_account, memberGroup.GetManagersDirectory());
    } else {
      m_serviceClients->GetServiceLocatorClient().StorePermissions(
        parameters.m_account, memberGroup.GetEntry(), Permissions{0});
      m_serviceClients->GetServiceLocatorClient().Detach(
        parameters.m_account, memberGroup.GetManagersDirectory());
    }
  }
  if(parameters.m_roles.Test(AccountRole::TRADER) !=
      previousRoles.Test(AccountRole::TRADER)) {
    if(parameters.m_roles.Test(AccountRole::TRADER)) {
      m_serviceClients->GetServiceLocatorClient().Associate(
        parameters.m_account, memberGroup.GetTradersDirectory());
    } else {
      m_serviceClients->GetServiceLocatorClient().Detach(
        parameters.m_account, memberGroup.GetTradersDirectory());
    }
  }
  auto newRoles = m_serviceClients->GetAdministrationClient().LoadAccountRoles(
    parameters.m_account);
  session->ShuttleResponse(newRoles, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadAccountIdentity(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
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

HttpResponse AdministrationWebServlet::OnStoreAccountIdentity(
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
  m_serviceClients->GetAdministrationClient().StoreIdentity(
    parameters.m_account, parameters.m_identity);
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadEntitlementsDatabase(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto database =
    m_serviceClients->GetAdministrationClient().LoadEntitlements();
  session->ShuttleResponse(database, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadAccountEntitlements(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
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

HttpResponse AdministrationWebServlet::OnStoreAccountEntitlements(
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
  m_serviceClients->GetAdministrationClient().StoreEntitlements(
    parameters.m_account, parameters.m_entitlements);
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadRiskParameters(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
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

HttpResponse AdministrationWebServlet::OnStoreRiskParameters(
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
  m_serviceClients->GetAdministrationClient().StoreRiskParameters(
    parameters.m_account, parameters.m_riskParameters);
  return response;
}
