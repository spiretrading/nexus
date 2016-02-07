#include "ClientWebPortal/ClientWebPortal/ClientWebPortalServlet.hpp"
#include <Beam/Json/JsonParser.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <Beam/WebServices/HttpServerRequest.hpp>
#include <Beam/WebServices/HttpServerResponse.hpp>
#include "ClientWebPortal/ClientWebPortal/ServiceClients.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Serialization;
using namespace Beam::WebServices;
using namespace Nexus;
using namespace Nexus::ClientWebPortal;
using namespace std;

ClientWebPortalServlet::ClientWebPortalServlet(
    RefType<ServiceClients> serviceClients)
    : m_fileStore{"files"},
      m_serviceClients{serviceClients.Get()} {}

ClientWebPortalServlet::~ClientWebPortalServlet() {
  Close();
}

vector<HttpRequestSlot> ClientWebPortalServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(
    MatchesPath(HttpMethod::POST, "/api/service_locator/login"),
    bind(&ClientWebPortalServlet::OnLogin, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, "/"),
    bind(&ClientWebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, ""),
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

HttpServerResponse ClientWebPortalServlet::OnIndex(
    const HttpServerRequest& request) {
  return m_fileStore.Serve("index.html");
}

HttpServerResponse ClientWebPortalServlet::OnServeFile(
    const HttpServerRequest& request) {
  return m_fileStore.Serve(request);
}

HttpServerResponse ClientWebPortalServlet::OnLogin(
    const HttpServerRequest& request) {
  auto parameters = boost::get<JsonObject>(
    Parse<JsonParser>(request.GetBody()));
  auto& username = boost::get<string>(parameters["username"]);
  auto& password = boost::get<string>(parameters["password"]);
  auto account =
    m_serviceClients->GetServiceLocatorClient().AuthenticateAccount(username,
    password);
  HttpServerResponse response{HttpStatusCode::OK};
  response.SetBody(Encode<SharedBuffer>(m_sender, account));
  return response;
}
