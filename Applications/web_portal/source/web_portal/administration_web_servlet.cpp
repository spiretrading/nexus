#include "web_portal/web_portal/administration_web_servlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "web_portal/web_portal/web_portal_session.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::WebServices;
using namespace boost;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::RiskService;
using namespace Nexus::WebPortal;
using namespace std;

namespace {
  bool IsAdministrator(ApplicationServiceClients& serviceClients,
      const DirectoryEntry& account) {
    return serviceClients.GetAdministrationClient().LoadAccountRoles(
      account).Test(AccountRole::ADMINISTRATOR);
  }

  bool HasReadPermission(ApplicationServiceClients& serviceClients,
      const DirectoryEntry& parent, const DirectoryEntry& child) {
    if(parent == child) {
      return true;
    }
    auto roles = serviceClients.GetAdministrationClient().LoadAccountRoles(
      parent, child);
    return !roles.GetBitset().none();
  }

  bool HasRequestPermission(ApplicationServiceClients& serviceClients,
      const DirectoryEntry& parent, const DirectoryEntry& child) {
    return HasReadPermission(serviceClients, parent, child);
  }

  bool HasReviewPermission(ApplicationServiceClients& serviceClients,
      const DirectoryEntry& parent, const DirectoryEntry& child) {
    auto roles = serviceClients.GetAdministrationClient().LoadAccountRoles(
      parent, child);
    return roles.Test(AccountRole::ADMINISTRATOR) ||
      roles.Test(AccountRole::MANAGER);
  }

  bool HasApprovePermission(ApplicationServiceClients& serviceClients,
      const DirectoryEntry& parent, const DirectoryEntry& child) {
    auto roles = serviceClients.GetAdministrationClient().LoadAccountRoles(
      parent, child);
    return roles.Test(AccountRole::ADMINISTRATOR);
  }

  bool HasRejectPermission(ApplicationServiceClients& serviceClients,
      const DirectoryEntry& parent, const DirectoryEntry& child) {
    return HasReadPermission(serviceClients, parent, child);
  }
}

AdministrationWebServlet::AdministrationWebServlet(
    Ref<SessionStore<WebPortalSession>> sessions,
    Ref<ApplicationServiceClients> serviceClients)
    : m_sessions{sessions.Get()},
      m_serviceClients{serviceClients.Get()} {}

AdministrationWebServlet::~AdministrationWebServlet() {
  Close();
}

vector<HttpRequestSlot> AdministrationWebServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_organization_name"),
    std::bind(&AdministrationWebServlet::OnLoadOrganizationName, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_accounts_by_roles"),
    std::bind(&AdministrationWebServlet::OnLoadAccountsByRoles, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_administrators_root_entry"),
    std::bind(&AdministrationWebServlet::OnLoadAdministratorsRootEntry, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_services_root_entry"),
    std::bind(&AdministrationWebServlet::OnLoadServicesRootEntry, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_trading_groups_root_entry"),
    std::bind(&AdministrationWebServlet::OnLoadTradingGroupsRootEntry, this,
    std::placeholders::_1));
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
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_modification_request"),
    std::bind(&AdministrationWebServlet::OnLoadAccountModificationRequest, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_modification_request_ids"),
    std::bind(&AdministrationWebServlet::OnLoadAccountModificationRequestIds,
    this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_managed_account_modification_request_ids"),
    std::bind(
    &AdministrationWebServlet::OnLoadManagedAccountModificationRequestIds, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_entitlement_modification"),
    std::bind(&AdministrationWebServlet::OnLoadEntitlementModification, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/submit_entitlement_modification_request"),
    std::bind(&AdministrationWebServlet::OnSubmitEntitlementModificationRequest,
    this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_risk_modification"),
    std::bind(&AdministrationWebServlet::OnLoadRiskModification, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/submit_risk_modification_request"),
    std::bind(&AdministrationWebServlet::OnSubmitRiskModificationRequest,
    this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_account_modification_request_status"),
    std::bind(&AdministrationWebServlet::OnLoadAccountModificationRequestStatus, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/approve_account_modification_request"),
    std::bind(&AdministrationWebServlet::OnApproveAccountModificationRequest,
    this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/reject_account_modification_request"),
    std::bind(&AdministrationWebServlet::OnRejectAccountModificationRequest,
    this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_message"), std::bind(
    &AdministrationWebServlet::OnLoadMessage, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/load_message_ids"), std::bind(
    &AdministrationWebServlet::OnLoadMessageIds, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/administration_service/send_account_modification_request_message"),
    std::bind(
    &AdministrationWebServlet::OnSendAccountModificationRequestMessage, this,
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

HttpResponse AdministrationWebServlet::OnLoadOrganizationName(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto organizationName =
    m_serviceClients->GetAdministrationClient().LoadOrganizationName();
  session->ShuttleResponse(organizationName, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadAccountsByRoles(
    const HttpRequest& request) {
  struct Parameters {
    AccountRoles m_roles;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("roles", m_roles);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr ||
      !IsAdministrator(*m_serviceClients, session->GetAccount())) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto accounts =
    m_serviceClients->GetAdministrationClient().LoadAccountsByRoles(
    parameters.m_roles);
  session->ShuttleResponse(accounts, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadAdministratorsRootEntry(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto root =
    m_serviceClients->GetAdministrationClient().LoadAdministratorsRootEntry();
  session->ShuttleResponse(root, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadServicesRootEntry(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto root =
    m_serviceClients->GetAdministrationClient().LoadServicesRootEntry();
  session->ShuttleResponse(root, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadTradingGroupsRootEntry(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto root =
    m_serviceClients->GetAdministrationClient().LoadTradingGroupsRootEntry();
  session->ShuttleResponse(root, Store(response));
  return response;
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
  if(session == nullptr ||
      !IsAdministrator(*m_serviceClients, session->GetAccount())) {
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
  if(session == nullptr ||
      !IsAdministrator(*m_serviceClients, session->GetAccount())) {
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
  if(session == nullptr ||
      !IsAdministrator(*m_serviceClients, session->GetAccount())) {
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
  if(session == nullptr ||
      !IsAdministrator(*m_serviceClients, session->GetAccount())) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  m_serviceClients->GetAdministrationClient().StoreRiskParameters(
    parameters.m_account, parameters.m_riskParameters);
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadAccountModificationRequest(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto modification =
    m_serviceClients->GetAdministrationClient().LoadAccountModificationRequest(
    parameters.m_id);
  if(!HasReadPermission(*m_serviceClients, session->GetAccount(),
      modification.GetAccount())) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  session->ShuttleResponse(modification, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadAccountModificationRequestIds(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountModificationRequest::Id m_startId;
    int m_maxCount;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("start_id", m_startId);
      shuttle.Shuttle("max_count", m_maxCount);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  if(!HasReadPermission(*m_serviceClients, session->GetAccount(),
      parameters.m_account)) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto requestIds = m_serviceClients->GetAdministrationClient().
    LoadAccountModificationRequestIds(parameters.m_account,
    parameters.m_startId, parameters.m_maxCount);
  session->ShuttleResponse(requestIds, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::
    OnLoadManagedAccountModificationRequestIds(const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    AccountModificationRequest::Id m_startId;
    int m_maxCount;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("account", m_account);
      shuttle.Shuttle("start_id", m_startId);
      shuttle.Shuttle("max_count", m_maxCount);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  if(!HasReadPermission(*m_serviceClients, session->GetAccount(),
      parameters.m_account)) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto requestIds = m_serviceClients->GetAdministrationClient().
    LoadManagedAccountModificationRequestIds(parameters.m_account,
    parameters.m_startId, parameters.m_maxCount);
  session->ShuttleResponse(requestIds, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadEntitlementModification(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto modification =
    m_serviceClients->GetAdministrationClient().LoadAccountModificationRequest(
    parameters.m_id);
  if(!HasReadPermission(*m_serviceClients, session->GetAccount(),
      modification.GetAccount())) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto entitlement = m_serviceClients->GetAdministrationClient().
    LoadEntitlementModification(parameters.m_id);
  session->ShuttleResponse(entitlement, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnSubmitEntitlementModificationRequest(
    const HttpRequest& request) {
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
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  if(!HasRequestPermission(*m_serviceClients, session->GetAccount(),
      parameters.m_account)) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto modification = m_serviceClients->GetAdministrationClient().
    SubmitAccountModificationRequest(parameters.m_account,
    session->GetAccount(), parameters.m_modification, parameters.m_comment);
  session->ShuttleResponse(modification, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadRiskModification(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto modificationRequest =
    m_serviceClients->GetAdministrationClient().LoadAccountModificationRequest(
    parameters.m_id);
  if(!HasReadPermission(*m_serviceClients, session->GetAccount(),
      modificationRequest.GetAccount())) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto riskModification =
    m_serviceClients->GetAdministrationClient().LoadRiskModification(
    parameters.m_id);
  session->ShuttleResponse(riskModification, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnSubmitRiskModificationRequest(
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
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  if(!HasRequestPermission(*m_serviceClients, session->GetAccount(),
      parameters.m_account)) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto modification = m_serviceClients->GetAdministrationClient().
    SubmitAccountModificationRequest(parameters.m_account,
    session->GetAccount(), parameters.m_modification, parameters.m_comment);
  session->ShuttleResponse(modification, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadAccountModificationRequestStatus(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto modification =
    m_serviceClients->GetAdministrationClient().LoadAccountModificationRequest(
    parameters.m_id);
  if(!HasReadPermission(*m_serviceClients, session->GetAccount(),
      modification.GetAccount())) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto status = m_serviceClients->GetAdministrationClient().
    LoadAccountModificationRequestStatus(parameters.m_id);
  session->ShuttleResponse(status, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnApproveAccountModificationRequest(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;
    Message m_comment;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
      shuttle.Shuttle("comment", m_comment);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto modification =
    m_serviceClients->GetAdministrationClient().LoadAccountModificationRequest(
    parameters.m_id);
  if(!HasApprovePermission(*m_serviceClients, session->GetAccount(),
      modification.GetAccount())) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto update = m_serviceClients->GetAdministrationClient().
    ApproveAccountModificationRequest(parameters.m_id, session->GetAccount(),
    parameters.m_comment);
  session->ShuttleResponse(update, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnRejectAccountModificationRequest(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;
    Message m_comment;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
      shuttle.Shuttle("comment", m_comment);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto modification =
    m_serviceClients->GetAdministrationClient().LoadAccountModificationRequest(
    parameters.m_id);
  if(!HasRejectPermission(*m_serviceClients, session->GetAccount(),
      modification.GetAccount())) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto update = m_serviceClients->GetAdministrationClient().
    RejectAccountModificationRequest(parameters.m_id, session->GetAccount(),
    parameters.m_comment);
  session->ShuttleResponse(update, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadMessage(
    const HttpRequest& request) {
  struct Parameters {
    Message::Id m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto message = m_serviceClients->GetAdministrationClient().LoadMessage(
    parameters.m_id);
  session->ShuttleResponse(message, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnLoadMessageIds(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto modification =
    m_serviceClients->GetAdministrationClient().LoadAccountModificationRequest(
    parameters.m_id);
  if(!HasReadPermission(*m_serviceClients, session->GetAccount(),
      modification.GetAccount())) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto messageIds = m_serviceClients->GetAdministrationClient().LoadMessageIds(
    parameters.m_id);
  session->ShuttleResponse(messageIds, Store(response));
  return response;
}

HttpResponse AdministrationWebServlet::OnSendAccountModificationRequestMessage(
    const HttpRequest& request) {
  struct Parameters {
    AccountModificationRequest::Id m_id;
    Message m_message;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
      shuttle.Shuttle("message", m_message);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto modification =
    m_serviceClients->GetAdministrationClient().LoadAccountModificationRequest(
    parameters.m_id);
  if(!HasReadPermission(*m_serviceClients, session->GetAccount(),
      modification.GetAccount())) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto message = m_serviceClients->GetAdministrationClient().
    SendAccountModificationRequestMessage(parameters.m_id,
    parameters.m_message);
  session->ShuttleResponse(message, Store(response));
  return response;
}
