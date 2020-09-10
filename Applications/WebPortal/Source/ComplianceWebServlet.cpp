#include "WebPortal/ComplianceWebServlet.hpp"
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
using namespace Nexus::AdministrationService;
using namespace Nexus::Compliance;
using namespace Nexus::WebPortal;

ComplianceWebServlet::ComplianceWebServlet(
  Ref<SessionStore<WebPortalSession>> sessions)
  : m_sessions(sessions.Get()) {}

ComplianceWebServlet::~ComplianceWebServlet() {
  Close();
}

std::vector<HttpRequestSlot> ComplianceWebServlet::GetSlots() {
  auto slots = std::vector<HttpRequestSlot>();
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

void ComplianceWebServlet::Close() {
  m_openState.Close();
}

HttpResponse ComplianceWebServlet::OnLoadDirectoryEntryComplianceRuleEntry(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_directoryEntry;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directoryEntry);
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
  auto rules = serviceClients.GetComplianceClient().Load(
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
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto& serviceClients = session->GetServiceClients();
  auto id = serviceClients.GetComplianceClient().Add(
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
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto& serviceClients = session->GetServiceClients();
  serviceClients.GetComplianceClient().Update(parameters.m_ruleEntry);
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
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto& serviceClients = session->GetServiceClients();
  serviceClients.GetComplianceClient().Delete(parameters.m_id);
  return response;
}
