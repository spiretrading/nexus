#include "ClientWebPortal/ClientWebPortal/ClientWebPortalServlet.hpp"
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <Beam/WebServices/HttpServerRequest.hpp>
#include <Beam/WebServices/HttpServerResponse.hpp>
#include "ClientWebPortal/ClientWebPortal/ServiceClients.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace Nexus;
using namespace Nexus::ClientWebPortal;
using namespace std;

ClientWebPortalServlet::ClientWebPortalServlet(
    RefType<ServiceClients> serviceClients)
    : m_serviceClients{serviceClients.Get()} {}

ClientWebPortalServlet::~ClientWebPortalServlet() {
  Close();
}

vector<HttpRequestSlot> ClientWebPortalServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/login"),
    bind(&ClientWebPortalServlet::OnLogin, this, std::placeholders::_1));
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

HttpServerResponse ClientWebPortalServlet::OnLogin(
    const HttpServerRequest& request) {
  return HttpServerResponse{HttpStatusCode::OK};
}
