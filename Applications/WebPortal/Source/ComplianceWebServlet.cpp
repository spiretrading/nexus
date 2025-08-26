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

ComplianceWebServlet::ComplianceWebServlet(
  Ref<SessionStore<WebPortalSession>> sessions)
  : m_sessions(sessions.Get()) {}

ComplianceWebServlet::~ComplianceWebServlet() {
  close();
}

std::vector<HttpRequestSlot> ComplianceWebServlet::get_slots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/compliance_service/load_directory_entry_compliance_rule_entry"),
    std::bind_front(
      &ComplianceWebServlet::on_load_directory_entry_compliance_rule_entry,
      this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/compliance_service/add_compliance_rule_entry"),
    std::bind_front(&ComplianceWebServlet::on_add_compliance_rule_entry, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/compliance_service/update_compliance_rule_entry"),
    std::bind_front(
      &ComplianceWebServlet::on_update_compliance_rule_entry, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/compliance_service/delete_compliance_rule_entry"),
    std::bind_front(
      &ComplianceWebServlet::on_delete_compliance_rule_entry, this));
  return slots;
}

void ComplianceWebServlet::close() {
  m_open_state.Close();
}

HttpResponse ComplianceWebServlet::
    on_load_directory_entry_compliance_rule_entry(const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_directory_entry;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directory_entry);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto rules = clients.get_compliance_client().load(params.m_directory_entry);
  session->shuttle_response(rules, Store(response));
  return response;
}

HttpResponse ComplianceWebServlet::on_add_compliance_rule_entry(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_directory_entry;
    ComplianceRuleEntry::State m_state;
    ComplianceRuleSchema m_schema;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directory_entry);
      shuttle.Shuttle("state", m_state);
      shuttle.Shuttle("schema", m_schema);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto id = clients.get_compliance_client().add(
    params.m_directory_entry, params.m_state, params.m_schema);
  session->shuttle_response(id, Store(response));
  return response;
}

HttpResponse ComplianceWebServlet::on_update_compliance_rule_entry(
    const HttpRequest& request) {
  struct Parameters {
    ComplianceRuleEntry m_rule_entry;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("rule_entry", m_rule_entry);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  clients.get_compliance_client().update(params.m_rule_entry);
  return response;
}

HttpResponse ComplianceWebServlet::on_delete_compliance_rule_entry(
    const HttpRequest& request) {
  struct Parameters {
    ComplianceRuleEntry::Id m_id;

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
  auto params = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  clients.get_compliance_client().remove(params.m_id);
  return response;
}
