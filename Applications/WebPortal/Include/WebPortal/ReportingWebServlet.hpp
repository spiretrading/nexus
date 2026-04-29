#ifndef NEXUS_REPORTING_WEB_SERVLET_HPP
#define NEXUS_REPORTING_WEB_SERVLET_HPP
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/thread/mutex.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/WebSessionStore.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/ExchangeRate.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus {

  /** A per-ticker breakdown within a currency. */
  struct TickerReportEntry {

    /** The ticker traded. */
    Ticker m_ticker;

    /** The volume traded. */
    Quantity m_volume;

    /** The fees incurred. */
    Money m_fees;

    /** The profit and loss. */
    Money m_profit_and_loss;
  };

  /** A per-currency breakdown within a report. */
  struct CurrencyReportEntry {

    /** The currency. */
    CurrencyId m_currency;

    /** The total profit and loss for this currency. */
    Money m_total_profit_and_loss;

    /** The total volume traded for this currency. */
    Quantity m_total_volume;

    /** The total fees incurred for this currency. */
    Money m_total_fees;

    /** The tickers traded in this currency. */
    std::vector<TickerReportEntry> m_tickers;
  };

  /** The result of a completed profit and loss report. */
  struct ProfitAndLossReport {

    /** The total profit and loss in account currency. */
    Money m_total_profit_and_loss;

    /** The total fees in account currency. */
    Money m_total_fees;

    /** The total volume in account currency. */
    Quantity m_total_volume;

    /** The list of currencies with activity. */
    std::vector<CurrencyReportEntry> m_currencies;

    /** The exchange rates for foreign currencies. */
    std::vector<ExchangeRate> m_exchange_rates;
  };

  /** A per-account breakdown within a group report. */
  struct AccountReportEntry {

    /** The account. */
    Beam::DirectoryEntry m_account;

    /** The total profit and loss in account currency. */
    Money m_total_profit_and_loss;

    /** The list of currencies with activity. */
    std::vector<CurrencyReportEntry> m_currencies;
  };

  /** The result of a completed group profit and loss report. */
  struct GroupProfitAndLossReport {

    /** The total profit and loss in account currency. */
    Money m_total_profit_and_loss;

    /** The total fees in account currency. */
    Money m_total_fees;

    /** The total volume in account currency. */
    Quantity m_total_volume;

    /** The list of accounts with activity. */
    std::vector<AccountReportEntry> m_accounts;

    /** The exchange rates for foreign currencies. */
    std::vector<ExchangeRate> m_exchange_rates;
  };

  /** Provides a web interface to the ReportingService. */
  class ReportingWebServlet {
    public:

      /**
       * Constructs a ReportingWebServlet.
       * @param sessions The available web sessions.
       */
      explicit ReportingWebServlet(
        Beam::Ref<Beam::WebSessionStore<WebPortalSession>> sessions);

      ~ReportingWebServlet();

      std::vector<Beam::HttpRequestSlot> get_slots();
      void close();

    private:
      struct ReportRequest {
        int m_id;
        Beam::DirectoryEntry m_account;
        boost::gregorian::date m_start;
        boost::gregorian::date m_end;
        std::shared_ptr<std::atomic_bool> m_is_cancelled;
      };
      struct AccountReports {
        int m_next_id;
        bool m_is_generating;
        Beam::Queue<ReportRequest> m_pending_requests;
        std::unordered_map<int, std::shared_ptr<std::atomic_bool>>
          m_cancel_tokens;
        std::unordered_map<int, ProfitAndLossReport> m_completed_reports;

        AccountReports();
      };
      struct GroupReports {
        int m_next_id;
        bool m_is_generating;
        Beam::Queue<ReportRequest> m_pending_requests;
        std::unordered_map<int, std::shared_ptr<std::atomic_bool>>
          m_cancel_tokens;
        std::unordered_map<int, GroupProfitAndLossReport> m_completed_reports;

        GroupReports();
      };
      Beam::WebSessionStore<WebPortalSession>* m_sessions;
      mutable boost::mutex m_mutex;
      std::unordered_map<int, std::shared_ptr<AccountReports>> m_accounts;
      std::unordered_map<int, std::shared_ptr<GroupReports>> m_groups;
      Beam::OpenState m_open_state;
      Beam::RoutineHandlerGroup m_routines;

      ReportingWebServlet(const ReportingWebServlet&) = delete;
      ReportingWebServlet& operator=(const ReportingWebServlet&) = delete;
      ProfitAndLossReport build_account_report(
        const Beam::DirectoryEntry& account,
        boost::gregorian::date start, boost::gregorian::date end,
        const std::atomic_bool& is_cancelled,
        WebPortalSession& session);
      void generate_reports(std::shared_ptr<AccountReports> account,
        std::shared_ptr<WebPortalSession> session);
      void generate_group_reports(std::shared_ptr<GroupReports> group,
        std::shared_ptr<WebPortalSession> session);
      Beam::HttpResponse on_start_profit_and_loss_report(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_profit_and_loss_report(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_cancel_profit_and_loss_report(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_start_group_profit_and_loss_report(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_group_profit_and_loss_report(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_cancel_group_profit_and_loss_report(
        const Beam::HttpRequest& request);
  };
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::TickerReportEntry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::TickerReportEntry& value,
        unsigned int version) const {
      shuttle.shuttle("ticker", value.m_ticker);
      shuttle.shuttle("volume", value.m_volume);
      shuttle.shuttle("fees", value.m_fees);
      shuttle.shuttle("profit_and_loss", value.m_profit_and_loss);
    }
  };

  template<>
  struct Shuttle<Nexus::CurrencyReportEntry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::CurrencyReportEntry& value,
        unsigned int version) const {
      shuttle.shuttle("currency", value.m_currency);
      shuttle.shuttle("total_profit_and_loss", value.m_total_profit_and_loss);
      shuttle.shuttle("total_volume", value.m_total_volume);
      shuttle.shuttle("total_fees", value.m_total_fees);
      shuttle.shuttle("tickers", value.m_tickers);
    }
  };

  template<>
  struct Shuttle<Nexus::ProfitAndLossReport> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::ProfitAndLossReport& value,
        unsigned int version) const {
      shuttle.shuttle("total_profit_and_loss", value.m_total_profit_and_loss);
      shuttle.shuttle("total_fees", value.m_total_fees);
      shuttle.shuttle("total_volume", value.m_total_volume);
      shuttle.shuttle("currencies", value.m_currencies);
      shuttle.shuttle("exchange_rates", value.m_exchange_rates);
    }
  };

  template<>
  struct Shuttle<Nexus::AccountReportEntry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::AccountReportEntry& value,
        unsigned int version) const {
      shuttle.shuttle("account", value.m_account);
      shuttle.shuttle("total_profit_and_loss", value.m_total_profit_and_loss);
      shuttle.shuttle("currencies", value.m_currencies);
    }
  };

  template<>
  struct Shuttle<Nexus::GroupProfitAndLossReport> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::GroupProfitAndLossReport& value,
        unsigned int version) const {
      shuttle.shuttle("total_profit_and_loss", value.m_total_profit_and_loss);
      shuttle.shuttle("total_fees", value.m_total_fees);
      shuttle.shuttle("total_volume", value.m_total_volume);
      shuttle.shuttle("accounts", value.m_accounts);
      shuttle.shuttle("exchange_rates", value.m_exchange_rates);
    }
  };
}

#endif
