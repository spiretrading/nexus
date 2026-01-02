#ifndef NEXUS_PYTHON_MARKET_DATA_SERVICE_HPP
#define NEXUS_PYTHON_MARKET_DATA_SERVICE_HPP
#include <string_view>
#include <pybind11/pybind11.h>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported HistoricalDataStore. */
  NEXUS_EXPORT_DLL pybind11::class_<HistoricalDataStore>&
    get_exported_historical_data_store();

  /** Returns the exported MarketDataClient. */
  NEXUS_EXPORT_DLL pybind11::class_<MarketDataClient>&
    get_exported_market_data_client();

  /** Returns the exported MarketDataFeedClient. */
  NEXUS_EXPORT_DLL pybind11::class_<MarketDataFeedClient>&
    get_exported_market_data_feed_client();

  /**
   * Exports the AsyncHistoricalDataStore class.
   * @param module The module to export to.
   */
  void export_async_historical_data_store(pybind11::module& module);

  /**
   * Exports the CachedHistoricalDataStore class.
   * @param module The module to export to.
   */
  void export_cached_historical_data_store(pybind11::module& module);

  /**
   * Exports the ClientHistoricalDataStore class.
   * @param module The module to export to.
   */
  void export_client_historical_data_store(pybind11::module& module);

  /**
   * Exports the DataStoreMarketDataClient class.
   * @param module The module to export to.
   */
  void export_data_store_market_data_client(pybind11::module& module);

  /**
   * Exports the EntitlementDatabase class.
   * @param module The module to export to.
   */
  void export_entitlement_database(pybind11::module& module);

  /**
   * Exports the EntitlementSet class.
   * @param module The module to export to.
   */
  void export_entitlement_set(pybind11::module& module);

  /**
   * Exports the HistoricalDataStore class.
   * @param <D> The type of HistoricalDataStore to export.
   * @param module The module to export to.
   * @param name The name of the class to export.
   * @return The exported class.
   */
  template<IsHistoricalDataStore D>
  auto export_historical_data_store(
      pybind11::module& module, std::string_view name) {
    auto data_store = pybind11::class_<D>(module, name.data()).
      def("load_security_info", &D::load_security_info).
      def("store", pybind11::overload_cast<const SecurityInfo&>(&D::store)).
      def("load_order_imbalances", &D::load_order_imbalances).
      def("store", pybind11::overload_cast<
        const SequencedVenueOrderImbalance&>(&D::store)).
      def("store", pybind11::overload_cast<
        const std::vector<SequencedVenueOrderImbalance>&>(&D::store)).
      def("load_bbo_quotes", &D::load_bbo_quotes).
      def("store",
        pybind11::overload_cast<const SequencedSecurityBboQuote&>(&D::store)).
      def("store", pybind11::overload_cast<
        const std::vector<SequencedSecurityBboQuote>&>(&D::store)).
      def("load_book_quotes", &D::load_book_quotes).
      def("store",
        pybind11::overload_cast<const SequencedSecurityBookQuote&>(&D::store)).
      def("store", pybind11::overload_cast<
        const std::vector<SequencedSecurityBookQuote>&>(&D::store)).
      def("load_time_and_sales", &D::load_time_and_sales).
      def("store", pybind11::overload_cast<
        const SequencedSecurityTimeAndSale&>(&D::store)).
      def("store", pybind11::overload_cast<
        const std::vector<SequencedSecurityTimeAndSale>&>(&D::store)).
      def("close", &D::close);
    if constexpr(!std::is_same_v<D, HistoricalDataStore>) {
      pybind11::implicitly_convertible<D, HistoricalDataStore>();
      get_exported_historical_data_store().
        def(pybind11::init<D*>(), pybind11::keep_alive<1, 2>());
    }
    return data_store;
  }

  /**
   * Exports the HistoricalDataStoreException class.
   * @param module The module to export to.
   */
  void export_historical_data_store_exception(pybind11::module& module);

  /**
   * Exports the LocalHistoricalDataStore class.
   * @param module The module to export to.
   */
  void export_local_historical_data_store(pybind11::module& module);

  /**
   * Exports a MarketDataClient class.
   * @param <C> The type of MarketDataClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported MarketDataClient.
   */
  template<IsMarketDataClient C>
  auto export_market_data_client(
      pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<C>(module, name.data()).
      def("query_sequenced_order_imbalances",
        pybind11::overload_cast<const VenueMarketDataQuery&,
          Beam::ScopedQueueWriter<SequencedOrderImbalance>>(&C::query)).
      def("query_order_imbalances",
        pybind11::overload_cast<const VenueMarketDataQuery&,
          Beam::ScopedQueueWriter<OrderImbalance>>(&C::query)).
      def("query_sequenced_bbo_quotes",
        pybind11::overload_cast<const SecurityMarketDataQuery&,
          Beam::ScopedQueueWriter<SequencedBboQuote>>(&C::query)).
      def("query_bbo_quotes",
        pybind11::overload_cast<const SecurityMarketDataQuery&,
          Beam::ScopedQueueWriter<BboQuote>>(&C::query)).
      def("query_sequenced_book_quotes",
        pybind11::overload_cast<const SecurityMarketDataQuery&,
          Beam::ScopedQueueWriter<SequencedBookQuote>>(&C::query)).
      def("query_book_quotes",
        pybind11::overload_cast<const SecurityMarketDataQuery&,
          Beam::ScopedQueueWriter<BookQuote>>(&C::query)).
      def("query_sequenced_time_and_sales",
        pybind11::overload_cast<const SecurityMarketDataQuery&,
          Beam::ScopedQueueWriter<SequencedTimeAndSale>>(&C::query)).
      def("query_time_and_sales",
        pybind11::overload_cast<const SecurityMarketDataQuery&,
          Beam::ScopedQueueWriter<TimeAndSale>>(&C::query)).
      def("query_security_info",
        pybind11::overload_cast<const SecurityInfoQuery&>(&C::query)).
      def("load_snapshot", &C::load_snapshot).
      def("load_technicals", &C::load_technicals).
      def("load_security_info_from_prefix", &C::load_security_info_from_prefix).
      def("close", &C::close);
    if constexpr(!std::is_same_v<C, MarketDataClient>) {
      pybind11::implicitly_convertible<C, MarketDataClient>();
      get_exported_market_data_client().
        def(pybind11::init<C*>(), pybind11::keep_alive<1, 2>());
    }
    return client;
  }

  /**
   * Exports a MarketDataFeedClient class.
   * @param <C> The type of MarketDataFeedClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported MarketDataFeedClient.
   */
  template<IsMarketDataFeedClient C>
  auto export_market_data_feed_client(
      pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<C>(module, name.data()).
      def("add", &C::add).
      def("publish",
        pybind11::overload_cast<const VenueOrderImbalance&>(&C::publish)).
      def("publish",
        pybind11::overload_cast<const SecurityBboQuote&>(&C::publish)).
      def("publish",
        pybind11::overload_cast<const SecurityBookQuote&>(&C::publish)).
      def("publish",
        pybind11::overload_cast<const SecurityTimeAndSale&>(&C::publish)).
      def("add_order", &C::add_order).
      def("modify_order_size", &C::modify_order_size).
      def("offset_order_size", &C::offset_order_size).
      def("modify_order_price", &C::modify_order_price).
      def("remove_order", &C::remove_order).
      def("close", &C::close);
    if constexpr(!std::is_same_v<C, MarketDataFeedClient>) {
      pybind11::implicitly_convertible<C, MarketDataFeedClient>();
      get_exported_market_data_feed_client().
        def(pybind11::init<C*>(), pybind11::keep_alive<1, 2>());
    }
    return client;
  }

  /**
   * Exports the market data reactors.
   * @param module The module to export to.
   */
  void export_market_data_reactors(pybind11::module& module);

  /**
   * Exports the MarketDataService namespace.
   * @param module The module to export to.
   */
  void export_market_data_service(pybind11::module& module);

  /**
   * Exports the application definitions.
   * @param module The module to export to.
   */
  void export_market_data_service_application_definitions(
    pybind11::module& module);

  /**
   * Exports the MarketDataServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void export_market_data_service_test_environment(pybind11::module& module);

  /**
   * Exports the MarketDataType enum.
   * @param module The module to export to.
   */
  void export_market_data_type(pybind11::module& module);

  /**
   * Exports the SqlHistoricalDataStore class connecting to MySQL.
   * @param module The module to export to.
   */
  void export_mysql_historical_data_store(pybind11::module& module);

  /**
   * Exports the SecuritySnapshot class.
   * @param module The module to export to.
   */
  void export_security_snapshot(pybind11::module& module);

  /**
   * Exports the SqlHistoricalDataStore class connecting to SQLite.
   * @param module The module to export to.
   */
  void export_sqlite_historical_data_store(pybind11::module& module);
}

#endif
