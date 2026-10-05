#include "WebPortal/WebPortalServlet.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <unordered_set>
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/OrderExecutionService/StandardQueries.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  std::vector<ReportDefinition> load_report_definitions() {
    auto directory = std::filesystem::path("report_definitions");
    if(!std::filesystem::exists(directory)) {
      return {};
    }
    auto files = std::vector<std::filesystem::path>();
    for(auto& entry : std::filesystem::directory_iterator(directory)) {
      auto extension = entry.path().extension();
      if(entry.is_regular_file() &&
          (extension == ".yml" || extension == ".yaml")) {
        files.push_back(entry.path());
      }
    }
    std::sort(files.begin(), files.end());
    auto definitions = std::vector<ReportDefinition>();
    auto identifiers = std::unordered_set<std::string>();
    for(auto& file : files) {
      auto definition = try_or_nest([&] {
        auto stream = std::ifstream(file);
        if(!stream) {
          throw std::runtime_error("Unable to open report definition.");
        }
        return load_report_definition(stream);
      }, std::runtime_error(
        "Failed to load report definition: " + file.string()));
      if(!identifiers.insert(definition.m_id).second) {
        throw std::runtime_error("Duplicate report identifier: " +
          definition.m_id + " in " + file.string());
      }
      definitions.push_back(std::move(definition));
    }
    return definitions;
  }
}

WebPortalServlet::WebPortalServlet(
  ServiceLocatorWebServlet::ClientsBuilder clients_builder,
  ServiceLocatorWebServlet::SessionClientsBuilder session_clients_builder,
  Clients clients)
  : m_file_store("web_app"),
    m_service_locator_servlet(Ref(m_sessions), std::move(clients_builder),
      std::move(session_clients_builder)),
    m_definitions_servlet(Ref(m_sessions)),
    m_administration_servlet(Ref(m_sessions)),
    m_market_data_servlet(Ref(m_sessions)),
    m_compliance_servlet(Ref(m_sessions)),
    m_reporting_servlet(Ref(m_sessions), clients.get_service_locator_client(),
      load_report_definitions),
    m_risk_servlet(Ref(m_sessions), std::move(clients)) {}

WebPortalServlet::~WebPortalServlet() {
  close();
}

std::vector<HttpRequestSlot> WebPortalServlet::get_slots() {
  auto slots = std::vector<HttpRequestSlot>();
  auto service_locator_slots = m_service_locator_servlet.get_slots();
  slots.insert(
    slots.end(), service_locator_slots.begin(), service_locator_slots.end());
  auto definitions_slots = m_definitions_servlet.get_slots();
  slots.insert(slots.end(), definitions_slots.begin(), definitions_slots.end());
  auto administration_slots = m_administration_servlet.get_slots();
  slots.insert(
    slots.end(), administration_slots.begin(), administration_slots.end());
  auto market_data_slots = m_market_data_servlet.get_slots();
  slots.insert(slots.end(), market_data_slots.begin(), market_data_slots.end());
  auto compliance_slots = m_compliance_servlet.get_slots();
  slots.insert(slots.end(), compliance_slots.begin(), compliance_slots.end());
  auto reporting_slots = m_reporting_servlet.get_slots();
  slots.insert(slots.end(), reporting_slots.begin(), reporting_slots.end());
  auto risk_slots = m_risk_servlet.get_slots();
  slots.insert(slots.end(), risk_slots.begin(), risk_slots.end());
  slots.emplace_back(matches_path(HttpMethod::GET, "/"),
    std::bind_front(&WebPortalServlet::on_index, this));
  slots.emplace_back(matches_path(HttpMethod::GET, ""),
    std::bind_front(&WebPortalServlet::on_index, this));
  slots.emplace_back(matches_path(HttpMethod::GET, "/index.html"),
    std::bind_front(&WebPortalServlet::on_index, this));
  slots.emplace_back(match_any(HttpMethod::GET),
    std::bind_front(&WebPortalServlet::on_serve_file, this));
  return slots;
}

std::vector<HttpUpgradeSlot<WebPortalServlet::WebSocketChannel>>
    WebPortalServlet::get_web_socket_slots() {
  auto slots = std::vector<HttpUpgradeSlot<WebSocketChannel>>();
  auto administration_slots =
    m_administration_servlet.get_web_socket_slots();
  slots.insert(
    slots.end(), administration_slots.begin(), administration_slots.end());
  auto risk_slots = m_risk_servlet.get_web_socket_slots();
  slots.insert(slots.end(), risk_slots.begin(), risk_slots.end());
  return slots;
}

void WebPortalServlet::close() {
  if(m_open_state.set_closing()) {
    return;
  }
  m_risk_servlet.close();
  m_reporting_servlet.close();
  m_compliance_servlet.close();
  m_market_data_servlet.close();
  m_administration_servlet.close();
  m_definitions_servlet.close();
  m_service_locator_servlet.close();
  m_open_state.close();
}

HttpResponse WebPortalServlet::on_index(const HttpRequest& request) {
  auto response = HttpResponse();
  m_file_store.serve("index.html", out(response));
  return response;
}

HttpResponse WebPortalServlet::on_serve_file(const HttpRequest& request) {
  auto response = m_file_store.serve(request);
  if(response.get_status_code() == HttpStatusCode::NOT_FOUND) {
    return on_index(request);
  }
  return response;
}
