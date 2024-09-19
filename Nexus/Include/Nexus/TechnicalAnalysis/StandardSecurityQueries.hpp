#ifndef NEXUS_STANDARD_SECURITY_QUERIES_HPP
#define NEXUS_STANDARD_SECURITY_QUERIES_HPP
#include <string>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include <Beam/Queries/StandardValues.hpp>
#include <Beam/Queues/ConverterQueueWriter.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/Routines/Routine.hpp>
#include <boost/date_time/local_time/tz_database.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/StandardValues.hpp"
#include "Nexus/TechnicalAnalysis/TechnicalAnalysis.hpp"

namespace Nexus::TechnicalAnalysis {

  /**
   * Returns the default market center for a given MarketCode.
   * @param code The market code to lookup.
   * @return The default market center for the given <i>market</i>.
   */
  inline std::string GetDefaultMarketCenter(MarketCode code) {
    if(code == DefaultMarkets::ASX()) {
      return "ASX";
    } else if(code == DefaultMarkets::CSE()) {
      return "CNQ";
    } else if(code == DefaultMarkets::HKEX()) {
      return "HKEX";
    } else if(code == DefaultMarkets::NEOE()) {
      return "AQL";
    } else if(code == DefaultMarkets::TSX()) {
      return "TSE";
    } else if(code == DefaultMarkets::TSXV()) {
      return "CDX";
    } else if(!code.IsEmpty()) {
      return code.GetData();
    }
    return "";
  }

  /**
   * Returns a query to retrieve a Security's opening trade.
   * @param security The Security to query.
   * @param date The date to retrieve the opening trade for.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @param marketCenter The market center used to determine the opening trade.
   * @return A SecurityMarketDataQuery that can be used to retrieve the
   *         <i>security</i>'s opening trade.
   */
  inline MarketDataService::SecurityMarketDataQuery MakeOpenQuery(
      Security security, boost::posix_time::ptime date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      std::string marketCenter) {
    auto marketStartOfDay = MarketDateToUtc(security.GetMarket(), date,
      marketDatabase, timeZoneDatabase);
    auto openQuery = MarketDataService::SecurityMarketDataQuery();
    openQuery.SetIndex(security);
    openQuery.SetRange(marketStartOfDay,
      Beam::Queries::Decrement(Beam::Queries::Sequence::Last()));
    openQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromHead(1));
    auto parameter =
      Beam::Queries::ParameterExpression(0, Nexus::Queries::TimeAndSaleType());
    openQuery.SetFilter(
      Beam::Queries::ConstantExpression(std::move(marketCenter)) ==
        Queries::TimeAndSaleAccessor(parameter).m_marketCenter);
    return openQuery;
  }

  /**
   * Returns a query to retrieve a Security's opening trade using the default
   * market center.
   * @param security The Security to query.
   * @param date The date to retrieve the opening trade for.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @return A SecurityMarketDataQuery that can be used to retrieve the
   *         <i>security</i>'s opening trade.
   */
  inline MarketDataService::SecurityMarketDataQuery MakeOpenQuery(
      Security security, boost::posix_time::ptime date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto code = security.GetMarket();
    return MakeOpenQuery(std::move(security), date, marketDatabase,
      timeZoneDatabase, GetDefaultMarketCenter(code));
  }

  /**
   * Queries for a Security's opening trade.
   * @param client The MarketDataClient to query.
   * @param security The Security to query.
   * @param date The date to retrieve the opening trade for.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @param marketCenter The market center used to determine the closing trade.
   * @return The opening trade for the specified <i>security</i>.
   */
  template<typename MarketDataClient>
  boost::optional<TimeAndSale> LoadOpen(MarketDataClient& client,
      Security security, boost::posix_time::ptime date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      std::string marketCenter) {
    auto query = MakeOpenQuery(std::move(security), date, marketDatabase,
      timeZoneDatabase, std::move(marketCenter));
    auto queue = std::make_shared<Beam::Queue<TimeAndSale>>();
    client.QueryTimeAndSales(query, queue);
    auto open = boost::optional<TimeAndSale>();
    try {
      open = queue->Pop();
    } catch(const std::exception&) {}
    return open;
  }

  /**
   * Queries for a Security's opening trade using the default market center.
   * @param client The MarketDataClient to query.
   * @param security The Security to query.
   * @param date The date to retrieve the opening trade for.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @return The opening trade for the specified <i>security</i>.
   */
  template<typename MarketDataClient>
  boost::optional<TimeAndSale> LoadOpen(MarketDataClient& client,
      Security security, boost::posix_time::ptime date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto code = security.GetMarket();
    return LoadOpen(client, std::move(security), date, marketDatabase,
      timeZoneDatabase, GetDefaultMarketCenter(code));
  }

  /**
   * Queries for a Security's opening trade.
   * @param client The MarketDataClient to query.
   * @param security The Security to query.
   * @param date The date to retrieve the opening trade for.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @param marketCenter The market center used to determine the closing trade.
   * @param queue The Queue to store the opening trade in.
   */
  template<typename MarketDataClient>
  void QueryOpen(MarketDataClient&& client, Security security,
      boost::posix_time::ptime date, const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      std::string marketCenter, Beam::ScopedQueueWriter<TimeAndSale> queue) {
    Beam::Routines::Spawn(
      [=, client = Beam::CapturePtr<MarketDataClient>(client),
          queue = std::move(queue)] () mutable {
        auto open = LoadOpen(*client, security, date, marketDatabase,
          timeZoneDatabase, marketCenter);
        if(open) {
          queue.Push(*open);
          return;
        }
        auto query = MakeOpenQuery(std::move(security), date, marketDatabase,
          timeZoneDatabase, std::move(marketCenter));
        query.SetRange(query.GetRange().GetStart(),
          Beam::Queries::Sequence::Last());
        query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::HEAD, 1);
        auto localQueue = std::make_shared<Beam::Queue<TimeAndSale>>();
        client->QueryTimeAndSales(query, localQueue);
        try {
          auto timeAndSale = localQueue->Pop();
          queue.Push(std::move(timeAndSale));
        } catch(const std::exception&) {}
        localQueue->Break();
      });
  }

  /**
   * Queries for a Security's opening trade using the default market center.
   * @param client The MarketDataClient to query.
   * @param security The Security to query.
   * @param date The date to retrieve the opening trade for.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @param queue The Queue to store the opening trade in.
   */
  template<typename MarketDataClient>
  void QueryOpen(MarketDataClient&& client, Security security,
      boost::posix_time::ptime date, const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    auto code = security.GetMarket();
    QueryOpen(client, std::move(security), date, marketDatabase,
      timeZoneDatabase, GetDefaultMarketCenter(code), std::move(queue));
  }

  /**
   * Returns a query to retrieve a Security's previous session's closing trade.
   * @param security The Security to query.
   * @param date The date for which the previous trading session's closing trade
   *        will be retrieved.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @param marketCenter The market center used to determine the closing trade.
   * @return A SecurityMarketDataQuery that can be used to retrieve the
   *         <i>security</i>'s previous session's closing trade.
   */
  inline MarketDataService::SecurityMarketDataQuery MakePreviousCloseQuery(
      Security security, boost::posix_time::ptime date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      std::string marketCenter) {
    auto marketStartOfDay = MarketDateToUtc(security.GetMarket(), date,
      marketDatabase, timeZoneDatabase);
    auto previousCloseQuery = MarketDataService::SecurityMarketDataQuery();
    previousCloseQuery.SetIndex(security);
    previousCloseQuery.SetRange(
      Beam::Queries::Sequence::First(), marketStartOfDay);
    previousCloseQuery.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::FromTail(1));
    auto parameter =
      Beam::Queries::ParameterExpression(0, Nexus::Queries::TimeAndSaleType());
    previousCloseQuery.SetFilter(
      Beam::Queries::ConstantExpression(std::move(marketCenter)) ==
        Queries::TimeAndSaleAccessor(parameter).m_marketCenter);
    return previousCloseQuery;
  }

  /**
   * Returns a query to retrieve a Security's previous session's closing trade
   * using the default market center.
   * @param security The Security to query.
   * @param date The date for which the previous trading session's closing trade
   *        will be retrieved.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @return A SecurityMarketDataQuery that can be used to retrieve the
   *         <i>security</i>'s previous session's closing trade.
   */
  inline MarketDataService::SecurityMarketDataQuery MakePreviousCloseQuery(
      Security security, boost::posix_time::ptime date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto code = security.GetMarket();
    return MakePreviousCloseQuery(std::move(security), date, marketDatabase,
      timeZoneDatabase, GetDefaultMarketCenter(code));
  }

  /**
   * Queries for a Security's previous session's closing trade.
   * @param client The MarketDataClient to query.
   * @param security The Security to query.
   * @param date The date for which the previous trading session's closing trade
   *        will be retrieved.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @param marketCenter The market center used to determine the closing trade.
   * @return The previous session's closing trade for the specified
   *         <i>security</i>.
   */
  template<typename MarketDataClient>
  boost::optional<TimeAndSale> LoadPreviousClose(MarketDataClient& client,
      Security security, boost::posix_time::ptime date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      std::string marketCenter) {
    auto query = MakePreviousCloseQuery(std::move(security), date,
      marketDatabase, timeZoneDatabase, std::move(marketCenter));
    auto queue = std::make_shared<Beam::Queue<TimeAndSale>>();
    client.QueryTimeAndSales(query, queue);
    auto previousClose = boost::optional<TimeAndSale>();
    try {
      previousClose = queue->Pop();
    } catch(const std::exception&) {}
    return previousClose;
  }

  /**
   * Queries for a Security's previous session's closing trade using the default
   * market center.
   * @param client The MarketDataClient to query.
   * @param security The Security to query.
   * @param date The date for which the previous trading session's closing trade
   *        will be retrieved.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @return The previous session's closing trade for the specified
   *         <i>security</i>.
   */
  template<typename MarketDataClient>
  boost::optional<TimeAndSale> LoadPreviousClose(MarketDataClient& client,
      Security security, boost::posix_time::ptime date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto code = security.GetMarket();
    return LoadPreviousClose(client, std::move(security), date, marketDatabase,
      timeZoneDatabase, GetDefaultMarketCenter(code));
  }

  /**
   * Returns a query for a Security's high price.
   * @param security The Security to query.
   * @param startDay The day to begin the high query.
   * @param endDay The day to end the high query.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @return A SecurityChartingQuery that can be used to retrieve the
   *         <i>security</i>'s high price.
   */
  inline ChartingService::SecurityChartingQuery MakeDailyHighQuery(
      Security security, boost::posix_time::ptime startDay,
      boost::posix_time::ptime endDay, const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto marketStartOfDay = MarketDateToUtc(security.GetMarket(), startDay,
      marketDatabase, timeZoneDatabase);
    auto marketEndOfDay = [&] () -> boost::posix_time::ptime {
      if(endDay == boost::posix_time::pos_infin) {
        return boost::posix_time::pos_infin;
      } else {
        return MarketDateToUtc(security.GetMarket(), endDay, marketDatabase,
          timeZoneDatabase) + boost::gregorian::days(1);
      }
    }();
    auto highQuery = ChartingService::SecurityChartingQuery();
    highQuery.SetIndex(security);
    highQuery.SetMarketDataType(
      MarketDataService::MarketDataType::TIME_AND_SALE);
    highQuery.SetRange(marketStartOfDay, marketEndOfDay);
    highQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromTail(1));
    highQuery.SetUpdatePolicy(
      Beam::Queries::ExpressionQuery::UpdatePolicy::CHANGE);
    auto max = Beam::Queries::MakeMaxExpression(
      Beam::Queries::ParameterExpression(0, Nexus::Queries::MoneyType()),
      Beam::Queries::ParameterExpression(1, Nexus::Queries::MoneyType()));
    auto high = Beam::Queries::ReduceExpression(
      max, Queries::TimeAndSaleAccessor(Beam::Queries::ParameterExpression(
        0, Queries::TimeAndSaleType())).m_price,
      Queries::MoneyValue(Money::ZERO));
    highQuery.SetExpression(high);
    return highQuery;
  }

  /**
   * Submits a query for a Security's high price.
   * @param client The ChartingClient to submit the query to.
   * @param security The Security to query.
   * @param startDay The day to begin the high query.
   * @param endDay The day to end the high query.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @param queue The Queue to store the high price in.
   */
  template<typename ChartingClient>
  void QueryDailyHigh(ChartingClient& client, Security security,
      boost::posix_time::ptime startDay, boost::posix_time::ptime endDay,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      Beam::ScopedQueueWriter<Money> queue) {
    client.QuerySecurity(MakeDailyHighQuery(std::move(security), startDay,
      endDay, marketDatabase, timeZoneDatabase),
      Beam::MakeConverterQueueWriter<Queries::QueryVariant>(std::move(queue),
        [] (const Queries::QueryVariant& value) {
          return boost::get<Money>(value);
        }));
  }

  /**
   * Returns a query for a Security's low price.
   * @param security The Security to query.
   * @param startDay The day to begin the low query.
   * @param startDay The day to end the low query.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @return A SecurityChartingQuery that can be used to retrieve the
   *         <i>security</i>'s low price.
   */
  inline ChartingService::SecurityChartingQuery MakeDailyLowQuery(
      Security security, boost::posix_time::ptime startDay,
      boost::posix_time::ptime endDay, const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto marketStartOfDay = MarketDateToUtc(security.GetMarket(), startDay,
      marketDatabase, timeZoneDatabase);
    auto marketEndOfDay = [&] () -> boost::posix_time::ptime {
      if(endDay == boost::posix_time::pos_infin) {
        return boost::posix_time::pos_infin;
      } else {
        return MarketDateToUtc(security.GetMarket(), endDay, marketDatabase,
          timeZoneDatabase) + boost::gregorian::days(1);
      }
    }();
    auto lowQuery = ChartingService::SecurityChartingQuery();
    lowQuery.SetIndex(security);
    lowQuery.SetMarketDataType(
      MarketDataService::MarketDataType::TIME_AND_SALE);
    lowQuery.SetRange(marketStartOfDay, marketEndOfDay);
    lowQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromTail(1));
    lowQuery.SetUpdatePolicy(
      Beam::Queries::ExpressionQuery::UpdatePolicy::CHANGE);
    auto min = Beam::Queries::MakeMinExpression(
      Beam::Queries::ParameterExpression(0, Nexus::Queries::MoneyType()),
      Beam::Queries::ParameterExpression(1, Nexus::Queries::MoneyType()));
    auto low = Beam::Queries::ReduceExpression(
      min, Queries::TimeAndSaleAccessor(Beam::Queries::ParameterExpression(
        0, Queries::TimeAndSaleType())).m_price,
      Queries::MoneyValue(99999999 * Money::ONE));
    lowQuery.SetExpression(low);
    return lowQuery;
  }

  /**
   * Submits a query for a Security's low price.
   * @param client The ChartingClient to submit the query to.
   * @param security The Security to query.
   * @param startDay The day to begin the low query.
   * @param endDay The day to end the low query.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @param queue The Queue to store the low price in.
   */
  template<typename ChartingClient>
  void QueryDailyLow(ChartingClient& client, Security security,
      boost::posix_time::ptime startDay, boost::posix_time::ptime endDay,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      Beam::ScopedQueueWriter<Money> queue) {
    client.QuerySecurity(MakeDailyLowQuery(std::move(security), startDay,
      endDay, marketDatabase, timeZoneDatabase),
      Beam::MakeConverterQueueWriter<Queries::QueryVariant>(std::move(queue),
        [] (const Queries::QueryVariant& value) {
          return boost::get<Money>(value);
        }));
  }

  /**
   * Returns a query over a Security's volume.
   * @param security The Security to query.
   * @param startDay The day to begin the volume query.
   * @param endDay The day to end the volume query.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @return A SecurityChartingQuery that can be used to retrieve the
   *         <i>security</i>'s volume.
   */
  inline ChartingService::SecurityChartingQuery MakeDailyVolumeQuery(
      Security security, boost::posix_time::ptime startDay,
      boost::posix_time::ptime endDay, const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto marketStartOfDay = MarketDateToUtc(security.GetMarket(), startDay,
      marketDatabase, timeZoneDatabase);
    auto marketEndOfDay = [&] () -> boost::posix_time::ptime {
      if(endDay == boost::posix_time::pos_infin) {
        return boost::posix_time::pos_infin;
      } else {
        return MarketDateToUtc(security.GetMarket(), endDay, marketDatabase,
          timeZoneDatabase) + boost::gregorian::days(1);
      }
    }();
    auto volumeQuery = ChartingService::SecurityChartingQuery();
    volumeQuery.SetIndex(std::move(security));
    volumeQuery.SetMarketDataType(
      MarketDataService::MarketDataType::TIME_AND_SALE);
    volumeQuery.SetRange(marketStartOfDay, marketEndOfDay);
    volumeQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::TAIL, 1);
    volumeQuery.SetUpdatePolicy(
      Beam::Queries::ExpressionQuery::UpdatePolicy::CHANGE);
    auto sum = Beam::Queries::ParameterExpression(0, Queries::QuantityType()) +
      Beam::Queries::ParameterExpression(1, Queries::QuantityType());
    auto volume = Beam::Queries::ReduceExpression(
      sum, Queries::TimeAndSaleAccessor(Beam::Queries::ParameterExpression(
        0, Queries::TimeAndSaleType())).m_size, Queries::QuantityValue(0));
    volumeQuery.SetExpression(volume);
    return volumeQuery;
  }

  /**
   * Submits a query for a Security's daily volume.
   * @param client The ChartingClient to submit the query to.
   * @param security The Security to query.
   * @param startDay The day to begin the volume query.
   * @param endDay The day to end the volume query.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @param queue The Queue to store the volume in.
   */
  template<typename ChartingClient>
  void QueryDailyVolume(ChartingClient& client, Security security,
      boost::posix_time::ptime startDay, boost::posix_time::ptime endDay,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      Beam::ScopedQueueWriter<Quantity> queue) {
    client.QuerySecurity(MakeDailyVolumeQuery(std::move(security), startDay,
      endDay, marketDatabase, timeZoneDatabase),
      Beam::MakeConverterQueueWriter<Queries::QueryVariant>(std::move(queue),
        [] (const Queries::QueryVariant& value) {
          return boost::get<Quantity>(value);
        }));
  }
}

#endif
