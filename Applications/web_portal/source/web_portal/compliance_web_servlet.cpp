#include "ClientWebPortal/ClientWebPortal/ComplianceWebServlet.hpp"
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
using namespace Nexus::Compliance;
using namespace std;

ComplianceWebServlet::ComplianceWebServlet(
    RefType<SessionStore<ClientWebPortalSession>> sessions,
    RefType<ApplicationServiceClients> serviceClients)
    : m_sessions{sessions.Get()},
      m_serviceClients{serviceClients.Get()} {}

ComplianceWebServlet::~ComplianceWebServlet() {
  Close();
}

vector<HttpRequestSlot> ComplianceWebServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/compliance_service/load_directory_entry_compliance_rule_entry"),
    std::bind(&ComplianceWebServlet::OnLoadDirectoryEntryComplianceRuleEntry,
    this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/compliance_service/add_compliance_rule_entry"),
    std::bind(&ComplianceWebServlet::OnAddComplianceRuleEntry, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/compliance_service/update_compliance_rule_entry"),
    std::bind(&ComplianceWebServlet::OnUpdateComplianceRuleEntry, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/compliance_service/delete_compliance_rule_entry"),
    std::bind(&ComplianceWebServlet::OnDeleteComplianceRuleEntry, this,
    std::placeholders::_1));
  return slots;
}

void ComplianceWebServlet::Open() {
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

void ComplianceWebServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void ComplianceWebServlet::Shutdown() {
  m_openState.SetClosed();
}

HttpResponse ComplianceWebServlet::OnLoadDirectoryEntryComplianceRuleEntry(
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
  auto rules = m_serviceClients->GetComplianceClient().Load(
    parameters.m_directoryEntry);
  session->ShuttleResponse(rules, Store(response));
  return response;
}

HttpResponse ComplianceWebServlet::OnAddComplianceRuleEntry(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_directoryEntry;
    ComplianceRuleEntry::State m_state;
    ComplianceRuleSchema m_schema;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directoryEntry);
      shuttle.Shuttle("state", m_state);
      shuttle.Shuttle("schema", m_schema);
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
  auto id = m_serviceClients->GetComplianceClient().Add(
    parameters.m_directoryEntry, parameters.m_state, parameters.m_schema);
  session->ShuttleResponse(id, Store(response));
  return response;
}

HttpResponse ComplianceWebServlet::OnUpdateComplianceRuleEntry(
    const HttpRequest& request) {
  struct Parameters {
    ComplianceRuleEntry m_ruleEntry;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("rule_entry", m_ruleEntry);
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
  m_serviceClients->GetComplianceClient().Update(parameters.m_ruleEntry);
  return response;
}

HttpResponse ComplianceWebServlet::OnDeleteComplianceRuleEntry(
    const HttpRequest& request) {
  struct Parameters {
    ComplianceRuleId m_id;

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
  auto roles = m_serviceClients->GetAdministrationClient().LoadAccountRoles(
    session->GetAccount());
  if(!roles.Test(AccountRole::ADMINISTRATOR)) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  m_serviceClients->GetComplianceClient().Delete(parameters.m_id);
  return response;
}
