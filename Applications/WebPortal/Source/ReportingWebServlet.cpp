#include "WebPortal/ReportingWebServlet.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <Beam/Queues/QueueReader.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
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

ReportingWebServlet::GroupReports::GroupReports()
  : m_next_id(0),
    m_is_generating(false) {}

ReportingWebServlet::ReportingWebServlet(
  Ref<WebSessionStore<WebPortalSession>> sessions, ReportService reports)
  : m_sessions(sessions.get()),
    m_reports(std::move(reports)) {}

ReportingWebServlet::~ReportingWebServlet() {
  close();
}

auto ReportingWebServlet::get_slots() -> std::vector<HttpRequestSlot> {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/reporting_service/load_report_definitions"), std::bind_front(
      &ReportingWebServlet::on_load_report_definitions, this));
  slots.emplace_back(
    matches_path(HttpMethod::POST, "/api/reporting_service/submit_report"),
      std::bind_front(&ReportingWebServlet::on_submit_report, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/reporting_service/query_report_activities"),
    std::bind_front(&ReportingWebServlet::on_query_report_activities, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/reporting_service/query_generated_reports"),
    std::bind_front(&ReportingWebServlet::on_query_generated_reports, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/reporting_service/query_scheduled_reports"),
    std::bind_front(&ReportingWebServlet::on_query_scheduled_reports, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/reporting_service/load_scheduled_report"),
    std::bind_front(&ReportingWebServlet::on_load_scheduled_report, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/reporting_service/update_scheduled_report"),
    std::bind_front(&ReportingWebServlet::on_update_scheduled_report, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/reporting_service/duplicate_scheduled_report"),
    std::bind_front(&ReportingWebServlet::on_duplicate_scheduled_report, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/reporting_service/delete_scheduled_report"),
    std::bind_front(&ReportingWebServlet::on_delete_scheduled_report, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/reporting_service/run_scheduled_report"),
    std::bind_front(&ReportingWebServlet::on_run_scheduled_report, this));
  slots.emplace_back(
    matches_path(HttpMethod::POST, "/api/reporting_service/load_report"),
    std::bind_front(&ReportingWebServlet::on_load_report, this));
  slots.emplace_back(
    matches_path(HttpMethod::GET, "/api/reporting_service/download_report"),
    std::bind_front(&ReportingWebServlet::on_download_report, this));
  slots.emplace_back(
    matches_path(HttpMethod::POST, "/api/reporting_service/share_reports"),
    std::bind_front(&ReportingWebServlet::on_share_reports, this));
  slots.emplace_back(
    matches_path(HttpMethod::POST, "/api/reporting_service/delete_reports"),
    std::bind_front(&ReportingWebServlet::on_delete_reports, this));
  slots.emplace_back(
    matches_path(HttpMethod::POST, "/api/reporting_service/cancel_report_jobs"),
    std::bind_front(&ReportingWebServlet::on_cancel_report_jobs, this));
  slots.emplace_back(
    matches_path(HttpMethod::POST, "/api/reporting_service/retry_report_jobs"),
    std::bind_front(&ReportingWebServlet::on_retry_report_jobs, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/reporting_service/start_profit_and_loss_report"), std::bind_front(
      &ReportingWebServlet::on_start_profit_and_loss_report, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/reporting_service/load_profit_and_loss_report"), std::bind_front(
      &ReportingWebServlet::on_load_profit_and_loss_report, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/reporting_service/cancel_profit_and_loss_report"), std::bind_front(
      &ReportingWebServlet::on_cancel_profit_and_loss_report, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/reporting_service/start_group_profit_and_loss_report"),
      std::bind_front(
        &ReportingWebServlet::on_start_group_profit_and_loss_report, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/reporting_service/load_group_profit_and_loss_report"),
      std::bind_front(
        &ReportingWebServlet::on_load_group_profit_and_loss_report, this));
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/reporting_service/cancel_group_profit_and_loss_report"),
      std::bind_front(
        &ReportingWebServlet::on_cancel_group_profit_and_loss_report, this));
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
    for(auto& [id, group] : m_groups) {
      group->m_pending_requests.close();
    }
  }
  m_reports.close();
  m_open_state.close();
}

ProfitAndLossReport ReportingWebServlet::build_account_report(
    const DirectoryEntry& account, date start, date end,
    const std::atomic_bool& is_cancelled, WebPortalSession& session) {
  auto& clients = session.get_clients();
  auto time_zones = clients.get_definitions_client().load_time_zone_database();
  auto portfolio = Portfolio(TrueAverageBookkeeper());
  for(auto day = start; day <= end; day += days(1)) {
    if(is_cancelled.load()) {
      return {};
    }
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto noon = ptime(day, hours(12));
    query_daily_order_submissions(account, noon, noon, time_zones,
      clients.get_order_execution_client(), order_queue);
    auto orders = std::vector<std::shared_ptr<Order>>();
    flush(order_queue, std::back_inserter(orders));
    for(auto& order : orders) {
      auto reports = order->get_publisher().get_snapshot();
      if(!reports) {
        continue;
      }
      for(auto& report : *reports) {
        portfolio.update(order->get_info().m_fields, report);
      }
    }
  }
  if(is_cancelled.load()) {
    return {};
  }
  auto& definitions_client = clients.get_definitions_client();
  auto exchange_rates =
    ExchangeRateTable(definitions_client.load_exchange_rates());
  auto risk_parameters =
    load_risk_parameters(clients.get_administration_client(), account);
  auto account_currency = risk_parameters.m_currency;
  auto report = ProfitAndLossReport();
  for(auto& total : portfolio.get_bookkeeper().get_totals_range()) {
    auto currency_entry = CurrencyReportEntry();
    currency_entry.m_currency = total.m_position.m_currency;
    currency_entry.m_total_profit_and_loss =
      total.m_gross_profit_and_loss - total.m_fees;
    currency_entry.m_total_volume = total.m_volume;
    currency_entry.m_total_fees = total.m_fees;
    for(auto& inventory : portfolio.get_bookkeeper().get_inventory_range()) {
      if(inventory.m_position.m_currency != total.m_position.m_currency) {
        continue;
      }
      if(is_empty(inventory)) {
        continue;
      }
      auto ticker_entry = TickerReportEntry();
      ticker_entry.m_ticker = inventory.m_position.m_ticker;
      ticker_entry.m_volume = inventory.m_volume;
      ticker_entry.m_fees = inventory.m_fees;
      ticker_entry.m_profit_and_loss =
        inventory.m_gross_profit_and_loss - inventory.m_fees;
      currency_entry.m_tickers.push_back(std::move(ticker_entry));
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
      report.m_total_profit_and_loss += currency_entry.m_total_profit_and_loss;
      report.m_total_fees += currency_entry.m_total_fees;
      report.m_total_volume += currency_entry.m_total_volume;
    }
    report.m_currencies.push_back(std::move(currency_entry));
  }
  return report;
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
    auto report = build_account_report(request.m_account, request.m_start,
      request.m_end, *request.m_is_cancelled, *session);
    if(request.m_is_cancelled->load()) {
      continue;
    }
    {
      auto lock = std::lock_guard(m_mutex);
      account->m_cancel_tokens.erase(request.m_id);
      account->m_completed_reports.insert(
        std::pair(request.m_id, std::move(report)));
    }
  }
}

void ReportingWebServlet::generate_group_reports(
    std::shared_ptr<GroupReports> group,
    std::shared_ptr<WebPortalSession> session) {
  while(true) {
    auto try_request = [&] {
      auto lock = std::lock_guard(m_mutex);
      auto request = group->m_pending_requests.try_pop();
      if(!request) {
        group->m_is_generating = false;
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
    auto trading_group =
      clients.get_administration_client().load_trading_group(request.m_account);
    auto members = std::vector<DirectoryEntry>();
    for(auto& manager : trading_group.get_managers()) {
      members.push_back(manager);
    }
    for(auto& trader : trading_group.get_traders()) {
      if(!std::ranges::contains(members, trader.m_id, &DirectoryEntry::m_id)) {
        members.push_back(trader);
      }
    }
    auto group_report = GroupProfitAndLossReport();
    auto exchange_rate_set = std::unordered_map<CurrencyPair, ExchangeRate>();
    for(auto& member : members) {
      if(request.m_is_cancelled->load()) {
        break;
      }
      auto account_report = build_account_report(member, request.m_start,
        request.m_end, *request.m_is_cancelled, *session);
      if(request.m_is_cancelled->load()) {
        break;
      }
      if(account_report.m_currencies.empty()) {
        continue;
      }
      group_report.m_total_profit_and_loss +=
        account_report.m_total_profit_and_loss;
      group_report.m_total_fees += account_report.m_total_fees;
      group_report.m_total_volume += account_report.m_total_volume;
      for(auto& rate : account_report.m_exchange_rates) {
        exchange_rate_set.insert(std::pair(rate.m_pair, rate));
      }
      auto account_entry = AccountReportEntry();
      account_entry.m_account = member;
      account_entry.m_total_profit_and_loss =
        account_report.m_total_profit_and_loss;
      account_entry.m_currencies = std::move(account_report.m_currencies);
      group_report.m_accounts.push_back(std::move(account_entry));
    }
    if(request.m_is_cancelled->load()) {
      continue;
    }
    for(auto& [pair, rate] : exchange_rate_set) {
      group_report.m_exchange_rates.push_back(rate);
    }
    {
      auto lock = std::lock_guard(m_mutex);
      group->m_cancel_tokens.erase(request.m_id);
      group->m_completed_reports.insert(
        std::pair(request.m_id, std::move(group_report)));
    }
  }
}

HttpResponse ReportingWebServlet::on_load_report_definitions(
    const HttpRequest& request) {
  struct Response {
    const ReportDefinition* m_definition;

    void shuttle(JsonSender<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_definition->m_id);
      shuttle.shuttle("name", m_definition->m_name);
      shuttle.shuttle("description", m_definition->m_description);
      shuttle.shuttle("parameters", m_definition->m_parameters);
      shuttle.shuttle("output", m_definition->m_output);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto definitions = m_reports.load_definitions(session->get_account());
  auto values = std::vector<Response>();
  for(auto& definition : definitions) {
    values.emplace_back(&definition);
  }
  session->shuttle_response(values, out(response));
  return response;
}

HttpResponse ReportingWebServlet::on_query_report_activities(
    const HttpRequest& request) {
  struct Sort {
    double m_column;
    double m_order;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("column", m_column);
      shuttle.shuttle("order", m_order);
    }
  };
  struct Parameters {
    Sort m_sort;
    double m_page_index;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("sort", m_sort);
      shuttle.shuttle("page_index", m_page_index);
    }
  };
  struct Response {
    const ReportActivities* m_page;

    void shuttle(JsonSender<SharedBuffer>& shuttle, unsigned int version) {
      constexpr auto READY = 1;
      auto is_empty = m_page->m_total_count == 0;
      shuttle.shuttle("status", READY);
      shuttle.shuttle("is_empty", is_empty);
      auto total_count = static_cast<double>(m_page->m_total_count);
      shuttle.shuttle("total_count", total_count);
      shuttle.shuttle("activities", m_page->m_activities);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto query = ReportActivityQuery();
  try {
    auto parameters = session->shuttle_parameters<Parameters>(request);
    auto read_index = [] (double value, double maximum) {
      if(!std::isfinite(value) || std::trunc(value) != value || value < 0 ||
          value > maximum) {
        throw std::invalid_argument("Invalid report activity query.");
      }
      return static_cast<std::uint32_t>(value);
    };
    query.m_column = ReportActivityQuery::Column(read_index(
      parameters.m_sort.m_column,
      static_cast<int>(ReportActivityQuery::Column::DATE_MODIFIED)));
    query.m_order = ReportActivityQuery::Order(read_index(
      parameters.m_sort.m_order,
      static_cast<int>(ReportActivityQuery::Order::DESCENDING)));
    query.m_page_index = read_index(parameters.m_page_index,
      std::numeric_limits<std::uint32_t>::max());
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    auto page = m_reports.query(session->get_account(), query);
    session->shuttle_response(Response(&page), out(response));
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_submit_report(const HttpRequest& request) {
  struct Parameters {
    std::string m_report_type;
    JsonValue m_parameters;
    std::vector<JsonValue> m_recipients;
    bool m_is_scheduled;
    JsonValue m_start_time;
    bool m_is_repeating;
    JsonValue m_interval;
    std::string m_time_zone;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("report_type", m_report_type);
      shuttle.shuttle("parameters", m_parameters);
      shuttle.shuttle("recipients", m_recipients);
      shuttle.shuttle("scheduled", m_is_scheduled);
      if(m_is_scheduled) {
        shuttle.shuttle("schedule_date_time", m_start_time);
        shuttle.shuttle("repeats", m_is_repeating);
        shuttle.shuttle("repeat_interval", m_interval);
        shuttle.shuttle("time_zone", m_time_zone);
      }
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto submission = ReportSubmission();
  auto schedule = std::optional<ReportScheduleSubmission>();
  try {
    auto parameters = session->shuttle_parameters<Parameters>(request);
    submission.m_report_type = std::move(parameters.m_report_type);
    submission.m_parameters = get<JsonObject>(parameters.m_parameters);
    for(auto& recipient : parameters.m_recipients) {
      submission.m_recipients.push_back(parse_report_entry(recipient));
    }
    if(parameters.m_is_scheduled) {
      auto interval = [&] () -> std::optional<ReportSchedule::Interval> {
        if(parameters.m_is_repeating) {
          return parse_report_interval(parameters.m_interval);
        } else if(!std::get_if<JsonNull>(&parameters.m_interval)) {
          throw std::invalid_argument("Unexpected repeat interval.");
        }
        return std::nullopt;
      }();
      schedule.emplace(std::move(submission),
        parse_report_datetime(parameters.m_start_time), interval,
        std::move(parameters.m_time_zone));
    }
  } catch(const std::exception& e) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    auto error = JsonObject();
    error.set("error", e.what());
    session->shuttle_response(JsonValue(error), out(response));
    return response;
  }
  try {
    auto id = [&] {
      if(schedule) {
        return m_reports.submit(session->get_account(), *schedule);
      }
      return m_reports.submit(session->get_account(), submission);
    }();
    session->shuttle_response(id, out(response));
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument& e) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    auto error = JsonObject();
    error.set("error", e.what());
    session->shuttle_response(JsonValue(error), out(response));
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_cancel_report_jobs(
    const HttpRequest& request) {
  struct Parameters {
    std::vector<std::string> m_ids;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("ids", m_ids);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = Parameters();
  try {
    parameters = session->shuttle_parameters<Parameters>(request);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    m_reports.cancel(session->get_account(), parameters.m_ids);
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_retry_report_jobs(
    const HttpRequest& request) {
  struct Parameters {
    std::vector<std::string> m_ids;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("ids", m_ids);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = Parameters();
  try {
    parameters = session->shuttle_parameters<Parameters>(request);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    m_reports.retry(session->get_account(), parameters.m_ids);
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_query_generated_reports(
    const HttpRequest& request) {
  struct Filters {
    std::string m_query;
    std::optional<std::string> m_start_date;
    std::optional<std::string> m_end_date;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("query", m_query);
      shuttle.shuttle("start_date", m_start_date);
      shuttle.shuttle("end_date", m_end_date);
    }
  };
  struct Sort {
    double m_column;
    double m_order;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("column", m_column);
      shuttle.shuttle("order", m_order);
    }
  };
  struct Parameters {
    Filters m_filters;
    Sort m_sort;
    double m_page_index;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("filters", m_filters);
      shuttle.shuttle("sort", m_sort);
      shuttle.shuttle("page_index", m_page_index);
    }
  };
  struct Response {
    const GeneratedReports* m_page;

    void shuttle(JsonSender<SharedBuffer>& shuttle, unsigned int version) {
      constexpr auto READY = 1;
      shuttle.shuttle("status", READY);
      shuttle.shuttle("is_empty", m_page->m_is_empty);
      auto count = static_cast<double>(m_page->m_filtered_count);
      shuttle.shuttle("filtered_count", count);
      shuttle.shuttle("reports", m_page->m_reports);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto query = GeneratedReportQuery();
  try {
    auto parameters = session->shuttle_parameters<Parameters>(request);
    auto read_index = [] (double value, double maximum) {
      if(!std::isfinite(value) || std::trunc(value) != value || value < 0 ||
          value > maximum) {
        throw std::invalid_argument("Invalid generated report query.");
      }
      return static_cast<std::uint32_t>(value);
    };
    query.m_column =
      GeneratedReportQuery::Column(read_index(parameters.m_sort.m_column,
        static_cast<int>(GeneratedReportQuery::Column::DATE_CREATED)));
    query.m_order =
      GeneratedReportQuery::Order(read_index(parameters.m_sort.m_order,
        static_cast<int>(GeneratedReportQuery::Order::DESCENDING)));
    query.m_page_index = read_index(
      parameters.m_page_index, std::numeric_limits<std::uint32_t>::max());
    query.m_query = std::move(parameters.m_filters.m_query);
    auto read_date =
      [] (const std::optional<std::string>& value) -> std::optional<date> {
        if(!value) {
          return std::nullopt;
        }
        constexpr auto DATE_LENGTH = 8;
        auto is_valid = value->size() == DATE_LENGTH &&
          std::ranges::all_of(*value, [] (auto digit) {
            return digit >= '0' && digit <= '9';
          });
        if(!is_valid) {
          throw std::invalid_argument("Invalid generated report date.");
        }
        return from_undelimited_string(*value);
      };
    query.m_start_date = read_date(parameters.m_filters.m_start_date);
    query.m_end_date = read_date(parameters.m_filters.m_end_date);
    if((query.m_start_date && query.m_start_date->is_special()) ||
        (query.m_end_date && query.m_end_date->is_special()) ||
        (query.m_start_date && query.m_end_date &&
          *query.m_start_date > *query.m_end_date)) {
      throw std::invalid_argument("Invalid generated report dates.");
    }
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    auto page = m_reports.query(session->get_account(), query);
    session->shuttle_response(Response(&page), out(response));
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_load_report(const HttpRequest& request) {
  struct Parameters {
    std::string m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  response.set_header({"Cache-Control", "private, no-store"});
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = Parameters();
  try {
    parameters = session->shuttle_parameters<Parameters>(request);
    if(parameters.m_id.empty()) {
      throw std::invalid_argument("Missing report identifier.");
    }
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    auto report =
      m_reports.load_report(session->get_account(), parameters.m_id);
    session->shuttle_response(report, out(response));
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_download_report(
    const HttpRequest& request) {
  auto response = HttpResponse();
  response.set_header({"Cache-Control", "private, no-store"});
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = parse_query(request.get_uri());
  if(parameters.count("id") != 1 || parameters.find("id")->second.empty()) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    auto file = m_reports.load_file(
      session->get_account(), parameters.find("id")->second);
    auto is_valid_type = !file.m_media_type.empty() &&
      std::ranges::all_of(file.m_media_type, [] (auto character) {
        return character >= ' ' && character <= '~';
      });
    if(!is_valid_type) {
      throw std::runtime_error("Invalid report media type.");
    }
    response.set_header({"Content-Type", file.m_media_type});
    auto name = file.m_name.u8string();
    response.set_header({"Content-Disposition",
      "attachment; filename*=UTF-8''" +
        uri_encode(std::string(name.begin(), name.end()))});
    response.set_header({"X-Content-Type-Options", "nosniff"});
    response.set_body(file.m_content);
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_share_reports(const HttpRequest& request) {
  struct Parameters {
    std::vector<std::string> m_ids;
    std::vector<JsonValue> m_recipients;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("ids", m_ids);
      shuttle.shuttle("recipients", m_recipients);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = Parameters();
  auto recipients = std::vector<DirectoryEntry>();
  try {
    parameters = session->shuttle_parameters<Parameters>(request);
    for(auto& value : parameters.m_recipients) {
      recipients.push_back(parse_report_entry(value));
    }
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    m_reports.share(session->get_account(), parameters.m_ids, recipients);
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_delete_reports(
    const HttpRequest& request) {
  struct Parameters {
    std::vector<std::string> m_ids;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("ids", m_ids);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = Parameters();
  try {
    parameters = session->shuttle_parameters<Parameters>(request);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    m_reports.remove(session->get_account(), parameters.m_ids);
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_query_scheduled_reports(
    const HttpRequest& request) {
  struct Filters {
    std::string m_query;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("query", m_query);
    }
  };
  struct Parameters {
    Filters m_filters;
    double m_page_index;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("filters", m_filters);
      shuttle.shuttle("page_index", m_page_index);
    }
  };
  struct Response {
    const ScheduledReports* m_page;

    void shuttle(JsonSender<SharedBuffer>& shuttle, unsigned int version) {
      constexpr auto READY = 1;
      shuttle.shuttle("status", READY);
      shuttle.shuttle("is_empty", m_page->m_is_empty);
      auto count = static_cast<double>(m_page->m_filtered_count);
      shuttle.shuttle("filtered_count", count);
      shuttle.shuttle("schedules", m_page->m_schedules);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto query = ScheduledReportQuery();
  try {
    auto parameters = session->shuttle_parameters<Parameters>(request);
    auto index = parameters.m_page_index;
    if(!std::isfinite(index) || std::trunc(index) != index || index < 0 ||
        index > std::numeric_limits<std::uint32_t>::max()) {
      throw std::invalid_argument("Invalid scheduled report page index.");
    }
    query.m_page_index = static_cast<std::uint32_t>(index);
    query.m_query = std::move(parameters.m_filters.m_query);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    auto page = m_reports.query(session->get_account(), query);
    session->shuttle_response(Response(&page), out(response));
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_load_scheduled_report(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_id;
    std::string m_time_zone;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
      shuttle.shuttle("time_zone", m_time_zone);
    }
  };
  struct Response {
    const ReportSchedule* m_schedule;
    ptime m_start_time;

    void shuttle(JsonSender<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("report_type", m_schedule->m_definition.m_id);
      auto parameters = JsonValue(m_schedule->m_parameters);
      shuttle.shuttle("parameters", parameters);
      shuttle.shuttle("recipients", m_schedule->m_recipients);
      shuttle.shuttle("scheduled", true);
      shuttle.shuttle("schedule_date_time", m_start_time);
      shuttle.shuttle("repeats", m_schedule->m_repeat_interval.has_value());
      if(m_schedule->m_repeat_interval) {
        shuttle.shuttle("repeat_interval", *m_schedule->m_repeat_interval);
      } else {
        shuttle.shuttle("repeat_interval", JsonValue(JsonNull()));
      }
    }
  };
  auto response = HttpResponse();
  response.set_header({"Cache-Control", "private, no-store"});
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = Parameters();
  try {
    parameters = session->shuttle_parameters<Parameters>(request);
    if(parameters.m_id.empty()) {
      throw std::invalid_argument("Missing schedule identifier.");
    }
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    auto schedule =
      m_reports.load_schedule(session->get_account(), parameters.m_id);
    auto start = convert_report_time(
      schedule.m_start_time, schedule.m_time_zone, parameters.m_time_zone);
    session->shuttle_response(Response(&schedule, start), out(response));
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_update_scheduled_report(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_id;
    std::string m_report_type;
    JsonValue m_parameters;
    std::vector<JsonValue> m_recipients;
    bool m_is_scheduled;
    JsonValue m_start_time;
    bool m_is_repeating;
    std::optional<JsonValue> m_interval;
    std::string m_time_zone;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
      shuttle.shuttle("report_type", m_report_type);
      shuttle.shuttle("parameters", m_parameters);
      shuttle.shuttle("recipients", m_recipients);
      shuttle.shuttle("scheduled", m_is_scheduled);
      shuttle.shuttle("schedule_date_time", m_start_time);
      shuttle.shuttle("repeats", m_is_repeating);
      shuttle.shuttle("repeat_interval", m_interval);
      shuttle.shuttle("time_zone", m_time_zone);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = Parameters();
  auto submission = ReportScheduleSubmission();
  try {
    parameters = session->shuttle_parameters<Parameters>(request);
    if(parameters.m_id.empty() || !parameters.m_is_scheduled) {
      throw std::invalid_argument("Invalid scheduled report update.");
    }
    submission.m_report.m_report_type = std::move(parameters.m_report_type);
    submission.m_report.m_parameters = get<JsonObject>(parameters.m_parameters);
    for(auto& value : parameters.m_recipients) {
      submission.m_report.m_recipients.push_back(parse_report_entry(value));
    }
    submission.m_start_time = parse_report_datetime(parameters.m_start_time);
    submission.m_time_zone = std::move(parameters.m_time_zone);
    if(parameters.m_is_repeating) {
      if(!parameters.m_interval) {
        throw std::invalid_argument("Missing repeat interval.");
      }
      submission.m_repeat_interval =
        parse_report_interval(*parameters.m_interval);
    } else if(parameters.m_interval &&
        !std::get_if<JsonNull>(&*parameters.m_interval)) {
      throw std::invalid_argument("Unexpected repeat interval.");
    }
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    m_reports.update_schedule(
      session->get_account(), parameters.m_id, submission);
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_duplicate_scheduled_report(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = Parameters();
  try {
    parameters = session->shuttle_parameters<Parameters>(request);
    if(parameters.m_id.empty()) {
      throw std::invalid_argument("Missing schedule identifier.");
    }
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    auto schedule =
      duplicate_schedule(m_reports, session->get_account(), parameters.m_id);
    session->shuttle_response(make_scheduled_report(schedule), out(response));
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_run_scheduled_report(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = Parameters();
  try {
    parameters = session->shuttle_parameters<Parameters>(request);
    if(parameters.m_id.empty()) {
      throw std::invalid_argument("Missing schedule identifier.");
    }
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    run_schedule(m_reports, session->get_account(), parameters.m_id);
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
}

HttpResponse ReportingWebServlet::on_delete_scheduled_report(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("id", m_id);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in()) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = Parameters();
  try {
    parameters = session->shuttle_parameters<Parameters>(request);
    if(parameters.m_id.empty()) {
      throw std::invalid_argument("Missing schedule identifier.");
    }
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  try {
    m_reports.remove_schedule(session->get_account(), parameters.m_id);
  } catch(const ReportNotFoundException&) {
    response.set_status_code(HttpStatusCode::NOT_FOUND);
  } catch(const std::invalid_argument&) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
  } catch(const std::exception&) {
    response.set_status_code(HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
  return response;
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
      m_routines.spawn(std::bind_front(
        &ReportingWebServlet::generate_reports, this, account, session));
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
  auto ready = ReadyResponse("ready", std::move(completed->second));
  account->second->m_completed_reports.erase(completed);
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

HttpResponse ReportingWebServlet::on_start_group_profit_and_loss_report(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_group;
    date m_start;
    date m_end;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("group", m_group);
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
    auto& group = m_groups[params.m_group.m_id];
    if(!group) {
      group = std::make_shared<GroupReports>();
    }
    auto id = group->m_next_id;
    ++group->m_next_id;
    auto cancel_token = std::make_shared<std::atomic_bool>(false);
    group->m_cancel_tokens.insert(std::pair(id, cancel_token));
    group->m_pending_requests.push(
      {id, params.m_group, params.m_start, params.m_end, cancel_token});
    if(!group->m_is_generating) {
      group->m_is_generating = true;
      m_routines.spawn(std::bind_front(
        &ReportingWebServlet::generate_group_reports, this, group, session));
    }
    return id;
  }();
  session->shuttle_response(id, out(response));
  return response;
}

HttpResponse ReportingWebServlet::on_load_group_profit_and_loss_report(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_group;
    int m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("group", m_group);
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
  auto group = m_groups.find(params.m_group.m_id);
  if(group == m_groups.end()) {
    response.set_status_code(HttpStatusCode::BAD_REQUEST);
    return response;
  }
  auto completed = group->second->m_completed_reports.find(params.m_id);
  if(completed == group->second->m_completed_reports.end()) {
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
    GroupProfitAndLossReport m_report;

    void shuttle(JsonSender<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("status", m_status);
      shuttle.shuttle("report", m_report);
    }
  };
  auto ready = ReadyResponse("ready", std::move(completed->second));
  group->second->m_completed_reports.erase(completed);
  session->shuttle_response(ready, out(response));
  return response;
}

HttpResponse ReportingWebServlet::on_cancel_group_profit_and_loss_report(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_group;
    int m_id;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("group", m_group);
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
    auto group = m_groups.find(params.m_group.m_id);
    if(group != m_groups.end()) {
      auto token = group->second->m_cancel_tokens.find(params.m_id);
      if(token != group->second->m_cancel_tokens.end()) {
        token->second->store(true);
      }
    }
  }
  response.set_header({"Content-Type", "application/json"});
  response.set_body(from<SharedBuffer>("{}"));
  return response;
}
