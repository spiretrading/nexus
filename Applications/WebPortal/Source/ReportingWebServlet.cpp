#include "WebPortal/ReportingWebServlet.hpp"
#include <Beam/Routines/Scheduler.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace Nexus;

ReportingWebServlet::AccountReports::AccountReports()
  : m_next_id(0),
    m_pending(std::make_shared<Queue<ReportRequest>>()) {}

ReportingWebServlet::ReportingWebServlet(
  Ref<WebSessionStore<WebPortalSession>> sessions)
  : m_sessions(sessions.get()) {}

ReportingWebServlet::~ReportingWebServlet() {
  close();
}

auto ReportingWebServlet::get_slots() -> std::vector<HttpRequestSlot> {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/reporting_service/start_profit_and_loss_report"), std::bind_front(
      &ReportingWebServlet::on_start_profit_and_loss_report, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/reporting_service/load_profit_and_loss_report"), std::bind_front(
      &ReportingWebServlet::on_load_profit_and_loss_report, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/reporting_service/cancel_profit_and_loss_report"), std::bind_front(
      &ReportingWebServlet::on_cancel_profit_and_loss_report, this));
  return slots;
}

void ReportingWebServlet::close() {
  if(m_open_state.set_closing()) {
    return;
  }
  {
    auto lock = std::lock_guard(m_mutex);
    for(auto& [id, account] : m_accounts) {
      account.m_pending->close();
    }
  }
  m_open_state.close();
}

void ReportingWebServlet::generate_reports(int account_id,
    std::shared_ptr<WebPortalSession> session,
    std::shared_ptr<Queue<ReportRequest>> pending) {
  try {
    while(true) {
      auto request = pending->pop();
      if(request.m_is_cancelled->load()) {
        continue;
      }
      // TODO: Generate the actual profit and loss report here using the
      // request's date range and account to query the OrderExecutionService.
      auto report = ProfitAndLossReport();
      if(request.m_is_cancelled->load()) {
        continue;
      }
      {
        auto lock = std::lock_guard(m_mutex);
        auto account = m_accounts.find(account_id);
        if(account != m_accounts.end()) {
          account->second.m_cancel_tokens.erase(request.m_id);
          account->second.m_completed.insert(
            std::pair(request.m_id, std::move(report)));
        }
      }
    }
  } catch(const std::exception&) {}
}

HttpResponse ReportingWebServlet::on_start_profit_and_loss_report(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    date m_start;
    date m_end;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("start", m_start);
      shuttle.shuttle("end", m_end);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto id = [&] {
    auto lock = std::lock_guard(m_mutex);
    auto& account = m_accounts[params.m_account.m_id];
    auto id = account.m_next_id;
    ++account.m_next_id;
    if(!account.m_pending) {
      account.m_pending = std::make_shared<Queue<ReportRequest>>();
      spawn(std::bind_front(&ReportingWebServlet::generate_reports, this,
        params.m_account.m_id, session, account.m_pending));
    }
    auto cancel_token = std::make_shared<std::atomic_bool>(false);
    account.m_cancel_tokens.insert(std::pair(id, cancel_token));
    account.m_pending->push(
      {id, params.m_account, params.m_start, params.m_end, cancel_token});
    return id;
  }();
  session->shuttle_response(id, out(response));
  return response;
}

HttpResponse ReportingWebServlet::on_load_profit_and_loss_report(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    int m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  auto lock = std::lock_guard(m_mutex);
  auto account = m_accounts.find(params.m_account.m_id);
  if(account == m_accounts.end()) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  auto completed = account->second.m_completed.find(params.m_id);
  if(completed == account->second.m_completed.end()) {
    struct PendingResponse {
      std::string m_status;

      void shuttle(JsonSender<SharedBuffer>& shuttle, unsigned int version) {
        shuttle.shuttle("status", m_status);
      }
    };
    auto pending = PendingResponse("pending");
    session->shuttle_response(pending, out(response));
    return response;
  }
  struct ReadyResponse {
    std::string m_status;
    ProfitAndLossReport m_report;

    void shuttle(JsonSender<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("status", m_status);
      shuttle.shuttle("report", m_report);
    }
  };
  auto ready = ReadyResponse("ready", completed->second);
  session->shuttle_response(ready, out(response));
  return response;
}

HttpResponse ReportingWebServlet::on_cancel_profit_and_loss_report(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_account;
    int m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("account", m_account);
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto params = session->shuttle_parameters<Parameters>(request);
  {
    auto lock = std::lock_guard(m_mutex);
    auto account = m_accounts.find(params.m_account.m_id);
    if(account != m_accounts.end()) {
      auto token = account->second.m_cancel_tokens.find(params.m_id);
      if(token != account->second.m_cancel_tokens.end()) {
        token->second->store(true);
      }
    }
  }
  response.set_header({"Content-Type", "application/json"});
  response.set_body(from<SharedBuffer>("{}"));
  return response;
}
