#include "WebPortal/Tests/ReportingWebServletTests.hpp"
#include <stdexcept>

using namespace Beam;
using namespace Nexus;

HttpResponse Nexus::Tests::dispatch(
    ReportingWebServlet& servlet, const HttpRequest& request) {
  for(auto& slot : servlet.get_slots()) {
    if(slot.m_predicate(request)) {
      return slot.m_slot(request);
    }
  }
  throw std::runtime_error("Missing reporting route.");
}

HttpResponse Nexus::Tests::dispatch(ReportingWebServlet& servlet,
    const WebPortalSession& session, HttpRequest request) {
  request.add(Cookie("sessionid", session.get_id()));
  return dispatch(servlet, request);
}

HttpResponse Nexus::Tests::post(ReportingWebServlet& servlet,
    const WebPortalSession& session, const std::string& route,
    const std::string& body) {
  auto request = HttpRequest(
    HttpMethod::POST, Uri(route), from<SharedBuffer>(body));
  return dispatch(servlet, session, request);
}

HttpResponse Nexus::Tests::post(ReportingWebServlet& servlet,
    const WebPortalSession& session, const std::string& route,
    const JsonValue& body) {
  return post(servlet, session, route, to_string(body));
}
