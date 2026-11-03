#ifndef NEXUS_REPORTING_WEB_SERVLET_TESTS_HPP
#define NEXUS_REPORTING_WEB_SERVLET_TESTS_HPP
#include "WebPortal/ReportingWebServlet.hpp"

namespace Nexus::Tests {

  /**
   * Dispatches a request to its matching servlet handler.
   * @param servlet The servlet handling the request.
   * @param request The request to dispatch.
   * @return The handler's response.
   */
  Beam::HttpResponse dispatch(
    ReportingWebServlet& servlet, const Beam::HttpRequest& request);

  /**
   * Dispatches a request with a session cookie.
   * @param servlet The servlet handling the request.
   * @param session The session to attach to the request.
   * @param request The request to dispatch.
   * @return The handler's response.
   */
  Beam::HttpResponse dispatch(ReportingWebServlet& servlet,
    const WebPortalSession& session, Beam::HttpRequest request);

  /**
   * Posts a raw body to a servlet route with a session cookie.
   * @param servlet The servlet handling the request.
   * @param session The session to attach to the request.
   * @param route The route receiving the request.
   * @param body The request body, sent verbatim.
   * @return The handler's response.
   */
  Beam::HttpResponse post(ReportingWebServlet& servlet,
    const WebPortalSession& session, const std::string& route,
    const std::string& body);

  /**
   * Posts a JSON body to a servlet route with a session cookie.
   * @param servlet The servlet handling the request.
   * @param session The session to attach to the request.
   * @param route The route receiving the request.
   * @param body The JSON request body.
   * @return The handler's response.
   */
  Beam::HttpResponse post(ReportingWebServlet& servlet,
    const WebPortalSession& session, const std::string& route,
    const Beam::JsonValue& body);

}

#endif
