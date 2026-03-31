#include "WebPortal/ReportingWebServlet.hpp"
#include <Beam/Queues/QueueReader.hpp>
#include <Beam/Routines/Scheduler.hpp>
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

ReportingWebServlet::AccountReports::AccountReports()
  : m_next_id(0),
    m_is_generating(false) {}

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
      account->m_pending_requests.close();
    }
  }
  m_open_state.close();
}

void ReportingWebServlet::generate_reports(
    std::shared_ptr<AccountReports> account,
    std::shared_ptr<WebPortalSession> session) {
  while(true) {
    auto try_request = [&] {
      auto lock = std::lock_guard(m_mutex);
      auto request = account->m_pending_requests.try_pop();
      if(!request) {
        account->m_is_generating = false;
      }
      return request;
    }();
    if(!try_request) {
      return;
    }
    auto& request = *try_request;
    if(request.m_is_cancelled->load()) {
      continue;
    }
    auto& clients = session->get_clients();
    auto venues = clients.get_definitions_client().load_venue_database();
    auto time_zones =
      clients.get_definitions_client().load_time_zone_database();
    auto bookkeeper = TrueAverageBookkeeper();
    for(auto day = request.m_start; day <= request.m_end; day += days(1)) {
      if(request.m_is_cancelled->load()) {
        break;
      }
      auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
      query_daily_order_submissions(request.m_account, ptime(day),
        ptime(day + days(1)), venues, time_zones,
        clients.get_order_execution_client(), order_queue);
      auto orders = std::vector<std::shared_ptr<Order>>();
      flush(order_queue, std::back_inserter(orders));
      for(auto& order : orders) {
        auto& fields = order->get_info().m_fields;
        auto reports = order->get_publisher().get_snapshot();
        if(!reports) {
          continue;
        }
        for(auto& report : *reports) {
          if(report.m_last_quantity == 0) {
            continue;
          }
          auto quantity = get_direction(fields.m_side) * report.m_last_quantity;
          bookkeeper.record(fields.m_security, fields.m_currency, quantity,
            report.m_last_quantity * report.m_last_price,
            get_fee_total(report));
        }
      }
    }
    if(request.m_is_cancelled->load()) {
      continue;
    }
    auto& definitions_client = clients.get_definitions_client();
    auto exchange_rates =
      ExchangeRateTable(definitions_client.load_exchange_rates());
    auto risk_parameters = load_risk_parameters(
      clients.get_administration_client(), request.m_account);
    auto account_currency = risk_parameters.m_currency;
    auto report = ProfitAndLossReport();
    for(auto& total : bookkeeper.get_totals_range()) {
      auto currency_entry = CurrencyReportEntry();
      currency_entry.m_currency = total.m_position.m_currency;
      currency_entry.m_total_profit_and_loss =
        total.m_gross_profit_and_loss - total.m_fees;
      currency_entry.m_total_volume = total.m_volume;
      currency_entry.m_total_fees = total.m_fees;
      for(auto& inventory : bookkeeper.get_inventory_range()) {
        if(inventory.m_position.m_currency != total.m_position.m_currency) {
          continue;
        }
        if(is_empty(inventory)) {
          continue;
        }
        auto security_entry = SecurityReportEntry();
        security_entry.m_security = inventory.m_position.m_security;
        security_entry.m_volume = inventory.m_volume;
        security_entry.m_fees = inventory.m_fees;
        security_entry.m_profit_and_loss =
          inventory.m_gross_profit_and_loss - inventory.m_fees;
        currency_entry.m_securities.push_back(std::move(security_entry));
      }
      if(auto rate = exchange_rates.find(
          CurrencyPair(total.m_position.m_currency, account_currency))) {
        report.m_total_profit_and_loss +=
          convert(currency_entry.m_total_profit_and_loss, *rate);
        report.m_total_fees += convert(currency_entry.m_total_fees, *rate);
        report.m_total_volume += rate->m_rate * currency_entry.m_total_volume;
        if(total.m_position.m_currency != account_currency) {
          report.m_exchange_rates.push_back(*rate);
        }
      } else {
        report.m_total_profit_and_loss +=
          currency_entry.m_total_profit_and_loss;
        report.m_total_fees += currency_entry.m_total_fees;
        report.m_total_volume += currency_entry.m_total_volume;
      }
      report.m_currencies.push_back(std::move(currency_entry));
    }
    {
      auto lock = std::lock_guard(m_mutex);
      account->m_cancel_tokens.erase(request.m_id);
      account->m_completed_reports.insert(
        std::pair(request.m_id, std::move(report)));
    }
  }
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
    if(!account) {
      account = std::make_shared<AccountReports>();
    }
    auto id = account->m_next_id;
    ++account->m_next_id;
    auto cancel_token = std::make_shared<std::atomic_bool>(false);
    account->m_cancel_tokens.insert(std::pair(id, cancel_token));
    account->m_pending_requests.push(
      {id, params.m_account, params.m_start, params.m_end, cancel_token});
    if(!account->m_is_generating) {
      account->m_is_generating = true;
      spawn(std::bind_front(&ReportingWebServlet::generate_reports, this,
        account, session));
    }
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
  auto completed = account->second->m_completed_reports.find(params.m_id);
  if(completed == account->second->m_completed_reports.end()) {
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
      auto token = account->second->m_cancel_tokens.find(params.m_id);
      if(token != account->second->m_cancel_tokens.end()) {
        token->second->store(true);
      }
    }
  }
  response.set_header({"Content-Type", "application/json"});
  response.set_body(from<SharedBuffer>("{}"));
  return response;
}
