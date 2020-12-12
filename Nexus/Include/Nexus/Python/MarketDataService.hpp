#ifndef NEXUS_PYTHON_MARKET_DATA_SERVICE_HPP
#define NEXUS_PYTHON_MARKET_DATA_SERVICE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClientBox.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported MarketDataClientBox. */
  NEXUS_EXPORT_DLL pybind11::class_<MarketDataService::MarketDataClientBox>&
    GetExportedMarketDataClientBox();

  /** Returns the exported MarketDataFeedClientBox. */
  NEXUS_EXPORT_DLL pybind11::class_<MarketDataService::MarketDataFeedClientBox>&
    GetExportedMarketDataFeedClientBox();

  /**
   * Exports the ApplicationMarketDataClient class.
   * @param module The module to export to.
   */
  void ExportApplicationMarketDataClient(pybind11::module& module);

  /**
   * Exports the ApplicationMarketDataFeedClient class.
   * @param module The module to export to.
   */
  void ExportApplicationMarketDataFeedClient(pybind11::module& module);

  /**
   * Exports the HistoricalDataStore class.
   * @param module The module to export to.
   */
  void ExportHistoricalDataStore(pybind11::module& module);

  /**
   * Exports the market data reactors.
   * @param module The module to export to.
   */
  void ExportMarketDataReactors(pybind11::module& module);

  /**
   * Exports the MarketDataService namespace.
   * @param module The module to export to.
   */
  void ExportMarketDataService(pybind11::module& module);

  /**
   * Exports the MarketDataServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportMarketDataServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports the SqlHistoricalDataStore class connecting to MySQL.
   * @param module The module to export to.
   */
  void ExportMySqlHistoricalDataStore(pybind11::module& module);

  /**
   * Exports the SecuritySnapshot class.
   * @param module The module to export to.
   */
  void ExportSecuritySnapshot(pybind11::module& module);

  /**
   * Exports the SqlHistoricalDataStore class connecting to SQLite.
   * @param module The module to export to.
   */
  void ExportSqliteHistoricalDataStore(pybind11::module& module);

  /**
   * Exports a MarketDataClient class.
   * @param <Client> The type of MarketDataClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported MarketDataClient.
   */
  template<typename Client>
  auto ExportMarketDataClient(pybind11::module& module,
      const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("query_sequenced_order_imbalances", static_cast<void (Client::*)(
        const MarketDataService::MarketWideDataQuery&,
        Beam::ScopedQueueWriter<SequencedOrderImbalance>)>(
          &Client::QueryOrderImbalances)).
      def("query_order_imbalances", static_cast<void (Client::*)(
        const MarketDataService::MarketWideDataQuery&,
        Beam::ScopedQueueWriter<OrderImbalance>)>(
          &Client::QueryOrderImbalances)).
      def("query_sequenced_bbo_quotes", static_cast<void (Client::*)(
        const MarketDataService::SecurityMarketDataQuery&,
        Beam::ScopedQueueWriter<SequencedBboQuote>)>(&Client::QueryBboQuotes)).
      def("query_bbo_quotes", static_cast<void (Client::*)(
        const MarketDataService::SecurityMarketDataQuery&,
        Beam::ScopedQueueWriter<BboQuote>)>(&Client::QueryBboQuotes)).
      def("query_sequenced_book_quotes", static_cast<void (Client::*)(
        const MarketDataService::SecurityMarketDataQuery&,
        Beam::ScopedQueueWriter<SequencedBookQuote>)>(
          &Client::QueryBookQuotes)).
      def("query_book_quotes", static_cast<void (Client::*)(
        const MarketDataService::SecurityMarketDataQuery&,
        Beam::ScopedQueueWriter<BookQuote>)>(&Client::QueryBookQuotes)).
      def("query_sequenced_market_quotes", static_cast<void (Client::*)(
        const MarketDataService::SecurityMarketDataQuery&,
        Beam::ScopedQueueWriter<SequencedMarketQuote>)>(
          &Client::QueryMarketQuotes)).
      def("query_market_quotes", static_cast<void (Client::*)(
        const MarketDataService::SecurityMarketDataQuery&,
        Beam::ScopedQueueWriter<MarketQuote>)>(&Client::QueryMarketQuotes)).
      def("query_sequenced_time_and_sales", static_cast<void (Client::*)(
        const MarketDataService::SecurityMarketDataQuery&,
        Beam::ScopedQueueWriter<SequencedTimeAndSale>)>(
          &Client::QueryTimeAndSales)).
      def("query_time_and_sales", static_cast<void (Client::*)(
        const MarketDataService::SecurityMarketDataQuery&,
        Beam::ScopedQueueWriter<TimeAndSale>)>(&Client::QueryTimeAndSales)).
      def("load_security_snapshot", &Client::LoadSecuritySnapshot).
      def("load_security_technicals", &Client::LoadSecurityTechnicals).
      def("load_security_info", &Client::LoadSecurityInfo).
      def("query_security_info", &Client::QuerySecurityInfo).
      def("load_security_info_from_prefix",
        &Client::LoadSecurityInfoFromPrefix).
      def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client,
        MarketDataService::MarketDataClientBox>) {
      pybind11::implicitly_convertible<Client,
        MarketDataService::MarketDataClientBox>();
      GetExportedMarketDataClientBox().def(
        pybind11::init<std::shared_ptr<Client>>());
    }
    return client;
  }

  /**
   * Exports a MarketDataFeedClient class.
   * @param <Client> The type of MarketDataFeedClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported MarketDataFeedClient.
   */
  template<typename Client>
  auto ExportMarketDataFeedClient(pybind11::module& module,
      const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("add", &Client::Add).
      def("publish_order_imbalance", &Client::PublishOrderImbalance).
      def("publish_bbo_quote", &Client::PublishBboQuote).
      def("publish_market_quote", &Client::PublishMarketQuote).
      def("set_book_quote", &Client::SetBookQuote).
      def("add_order", &Client::AddOrder).
      def("modify_order_size", &Client::ModifyOrderSize).
      def("offset_order_size", &Client::OffsetOrderSize).
      def("modify_order_price", &Client::ModifyOrderPrice).
      def("delete_order", &Client::DeleteOrder).
      def("publish_time_and_sale", &Client::PublishTimeAndSale).
      def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client,
        MarketDataService::MarketDataFeedClientBox>) {
      pybind11::implicitly_convertible<Client,
        MarketDataService::MarketDataFeedClientBox>();
      GetExportedMarketDataFeedClientBox().def(
        pybind11::init<std::shared_ptr<Client>>());
    }
    return client;
  }
}

#endif
