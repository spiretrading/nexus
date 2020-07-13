#ifndef NEXUS_STANDARDSECURITYQUERIES_HPP
#define NEXUS_STANDARDSECURITYQUERIES_HPP
#include <string>
#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include <Beam/Queries/StandardValues.hpp>
#include <Beam/Queues/Queue.hpp>
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

namespace Nexus {
namespace TechnicalAnalysis {

  //! Builds a query to retrieve a Security's opening trade.
  /*!
    \param security The Security to query.
    \param date The date to retrieve the opening trade for.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \param marketCenter The market center used to determine the opening trade.
    \return A SecurityMarketDataQuery that can be used to retrieve the
            <i>security</i>'s opening trade.
  */
  inline MarketDataService::SecurityMarketDataQuery BuildOpenQuery(
      const Security& security, const boost::posix_time::ptime& date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      std::string marketCenter) {
    if(marketCenter.empty()) {
      if(security.GetMarket() == DefaultMarkets::TSX()) {
        marketCenter = "TSE";
      } else if(security.GetMarket() == DefaultMarkets::TSXV()) {
        marketCenter = "CDX";
      } else if(security.GetMarket() == DefaultMarkets::CSE()) {
        marketCenter = "CNQ";
      } else if(security.GetMarket() == DefaultMarkets::HKEX()) {
        marketCenter = "HKEX";
      } else if(security.GetMarket() == DefaultMarkets::ASX()) {
        marketCenter = "ASX";
      } else if(!security.GetMarket().IsEmpty()) {
        marketCenter = security.GetMarket().GetData();
      }
    }
    auto marketStartOfDay = MarketDateToUtc(security.GetMarket(), date,
      marketDatabase, timeZoneDatabase);
    Beam::Queries::StringValue queryMarketCode(marketCenter);
    Beam::Queries::ConstantExpression marketCodeExpression(queryMarketCode);
    Beam::Queries::ParameterExpression parameterExpression(
      0, Nexus::Queries::TimeAndSaleType());
    Beam::Queries::MemberAccessExpression accessExpression("market_center",
      Beam::Queries::StringType(), parameterExpression);
    auto equalExpression = Beam::Queries::MakeEqualsExpression(
      marketCodeExpression, accessExpression);
    MarketDataService::SecurityMarketDataQuery openQuery;
    openQuery.SetIndex(security);
    openQuery.SetRange(marketStartOfDay,
      Beam::Queries::Decrement(Beam::Queries::Sequence::Last()));
    openQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::HEAD, 1);
    openQuery.SetFilter(equalExpression);
    return openQuery;
  }

  //! Queries for a Security's opening trade.
  /*!
    \param client The MarketDataClient to query.
    \param security The Security to query.
    \param date The date to retrieve the opening trade for.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \param marketCenter The market center used to determine the closing trade.
    \return The opening trade for the specified <i>security</i>.
  */
  template<typename MarketDataClient>
  boost::optional<TimeAndSale> LoadOpen(MarketDataClient& client,
      const Security& security, const boost::posix_time::ptime& date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      const std::string& marketCenter) {
    auto query = BuildOpenQuery(security, date, marketDatabase,
      timeZoneDatabase, marketCenter);
    auto queue = std::make_shared<Beam::Queue<TimeAndSale>>();
    client.QueryTimeAndSales(query, queue);
    boost::optional<TimeAndSale> open;
    try {
      open = queue->Top();
      queue->Pop();
    } catch(const std::exception&) {}
    return open;
  }

  //! Queries for a Security's opening trade.
  /*!
    \param client The MarketDataClient to query.
    \param security The Security to query.
    \param date The date to retrieve the opening trade for.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \param marketCenter The market center used to determine the closing trade.
    \param queue The Queue to store the opening trade in.
  */
  template<typename MarketDataClient>
  void QueryOpen(MarketDataClient& client, const Security& security,
      const boost::posix_time::ptime& date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      const std::string& marketCenter,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {
    Beam::Routines::Spawn(
      [=, &client] {
        auto open = LoadOpen(client, security, date, marketDatabase,
          timeZoneDatabase, marketCenter);
        if(open.is_initialized()) {
          queue->Push(*open);
          queue->Break();
          return;
        }
        auto query = BuildOpenQuery(security, date, marketDatabase,
          timeZoneDatabase, marketCenter);
        query.SetRange(query.GetRange().GetStart(),
          Beam::Queries::Sequence::Last());
        query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::HEAD, 1);
        auto localQueue = std::make_shared<Beam::Queue<TimeAndSale>>();
        client.QueryTimeAndSales(query, localQueue);
        try {
          auto timeAndSale = localQueue->Top();
          localQueue->Pop();
          queue->Push(std::move(timeAndSale));
        } catch(const std::exception&) {}
        localQueue->Break();
        queue->Break();
      });
  }

  //! Builds a query to retrieve a Security's previous session's closing trade.
  /*!
    \param security The Security to query.
    \param date The date for which the previous trading session's closing trade
           will be retrieved.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \param marketCenter The market center used to determine the closing trade.
    \return A SecurityMarketDataQuery that can be used to retrieve the
            <i>security</i>'s previous session's closing trade.
  */
  inline MarketDataService::SecurityMarketDataQuery BuildPreviousCloseQuery(
      const Security& security, const boost::posix_time::ptime& date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      std::string marketCenter) {
    if(marketCenter.empty()) {
      if(security.GetMarket() == DefaultMarkets::TSX()) {
        marketCenter = "TSE";
      } else if(security.GetMarket() == DefaultMarkets::TSXV()) {
        marketCenter = "CDX";
      } else if(security.GetMarket() == DefaultMarkets::CSE()) {
        marketCenter = "CNQ";
      } else if(security.GetMarket() == DefaultMarkets::HKEX()) {
        marketCenter = "HKEX";
      } else if(security.GetMarket() == DefaultMarkets::ASX()) {
        marketCenter = "ASX";
      } else if(!security.GetMarket().IsEmpty()) {
        marketCenter = security.GetMarket().GetData();
      }
    }
    auto marketStartOfDay = MarketDateToUtc(security.GetMarket(), date,
      marketDatabase, timeZoneDatabase);
    Beam::Queries::StringValue queryMarketCode(marketCenter);
    Beam::Queries::ConstantExpression marketCodeExpression(queryMarketCode);
    Beam::Queries::ParameterExpression parameterExpression(
      0, Nexus::Queries::TimeAndSaleType());
    Beam::Queries::MemberAccessExpression accessExpression("market_center",
      Beam::Queries::StringType(), parameterExpression);
    auto equalExpression = Beam::Queries::MakeEqualsExpression(
      marketCodeExpression, accessExpression);
    MarketDataService::SecurityMarketDataQuery previousCloseQuery;
    previousCloseQuery.SetIndex(security);
    previousCloseQuery.SetRange(Beam::Queries::Sequence::First(),
      marketStartOfDay);
    previousCloseQuery.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Type::TAIL, 1);
    previousCloseQuery.SetFilter(equalExpression);
    return previousCloseQuery;
  }

  //! Queries for a Security's previous session's closing trade.
  /*!
    \param client The MarketDataClient to query.
    \param security The Security to query.
    \param date The date for which the previous trading session's closing trade
           will be retrieved.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \param marketCenter The market center used to determine the closing trade.
    \return The previous session's closing trade for the specified
            <i>security</i>.
  */
  template<typename MarketDataClient>
  boost::optional<TimeAndSale> LoadPreviousClose(MarketDataClient& client,
      const Security& security, const boost::posix_time::ptime& date,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      const std::string& marketCenter) {
    auto query = BuildPreviousCloseQuery(security, date, marketDatabase,
      timeZoneDatabase, marketCenter);
    auto queue = std::make_shared<Beam::Queue<TimeAndSale>>();
    client.QueryTimeAndSales(query, queue);
    boost::optional<TimeAndSale> previousClose;
    try {
      previousClose = queue->Top();
      queue->Pop();
    } catch(const std::exception&) {}
    return previousClose;
  }

  //! Builds a query for a Security's high price.
  /*!
    \param security The Security to query.
    \param startDay The day to begin the high query.
    \param endDay The day to end the high query.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \return A SecurityChartingQuery that can be used to retrieve the
            <i>security</i>'s high price.
  */
  inline ChartingService::SecurityChartingQuery BuildDailyHighQuery(
      const Security& security, const boost::posix_time::ptime& startDay,
      const boost::posix_time::ptime& endDay,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto marketStartOfDay = MarketDateToUtc(security.GetMarket(), startDay,
      marketDatabase, timeZoneDatabase);
    boost::posix_time::ptime marketEndOfDay;
    if(endDay == boost::posix_time::pos_infin) {
      marketEndOfDay = boost::posix_time::pos_infin;
    } else {
      marketEndOfDay = MarketDateToUtc(security.GetMarket(), endDay,
        marketDatabase, timeZoneDatabase) + boost::gregorian::days(1);
    }
    Queries::MoneyValue initialValue(Money::ZERO);
    auto maxExpression = Beam::Queries::MakeMaxExpression(
      Beam::Queries::ParameterExpression(0, Nexus::Queries::MoneyType()),
      Beam::Queries::ParameterExpression(1, Nexus::Queries::MoneyType()));
    Beam::Queries::ReduceExpression highExpression(maxExpression,
      Beam::Queries::MemberAccessExpression("price",
      Nexus::Queries::MoneyType(),
      Beam::Queries::ParameterExpression(0, Queries::TimeAndSaleType())),
      initialValue);
    ChartingService::SecurityChartingQuery highQuery;
    highQuery.SetIndex(security);
    highQuery.SetMarketDataType(
      MarketDataService::MarketDataType::TIME_AND_SALE);
    highQuery.SetRange(marketStartOfDay, marketEndOfDay);
    highQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::TAIL, 1);
    highQuery.SetUpdatePolicy(
      Beam::Queries::ExpressionQuery::UpdatePolicy::CHANGE);
    highQuery.SetExpression(highExpression);
    return highQuery;
  }

  //! Submits a query for a Security's high price.
  /*!
    \param client The ChartingClient to submit the query to.
    \param security The Security to query.
    \param startDay The day to begin the high query.
    \param endDay The day to end the high query.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \param queue The Queue to store the high price in.
  */
  template<typename ChartingClient>
  void QueryDailyHigh(ChartingClient& client, const Security& security,
      const boost::posix_time::ptime& startDay,
      const boost::posix_time::ptime& endDay,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      const std::shared_ptr<Beam::QueueWriter<Queries::QueryVariant>>& queue) {
    auto query = BuildDailyHighQuery(security, startDay, endDay, marketDatabase,
      timeZoneDatabase);
    client.QuerySecurity(query, queue);
  }

  //! Builds a query for a Security's low price.
  /*!
    \param security The Security to query.
    \param startDay The day to begin the low query.
    \param startDay The day to end the low query.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \return A SecurityChartingQuery that can be used to retrieve the
            <i>security</i>'s low price.
  */
  inline ChartingService::SecurityChartingQuery BuildDailyLowQuery(
      const Security& security, const boost::posix_time::ptime& startDay,
      const boost::posix_time::ptime& endDay,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto marketStartOfDay = MarketDateToUtc(security.GetMarket(), startDay,
      marketDatabase, timeZoneDatabase);
    boost::posix_time::ptime marketEndOfDay;
    if(endDay == boost::posix_time::pos_infin) {
      marketEndOfDay = boost::posix_time::pos_infin;
    } else {
      marketEndOfDay = MarketDateToUtc(security.GetMarket(), endDay,
        marketDatabase, timeZoneDatabase) + boost::gregorian::days(1);
    }
    Queries::MoneyValue initialValue(99999999 * Money::ONE);
    auto minExpression = Beam::Queries::MakeMinExpression(
      Beam::Queries::ParameterExpression(0, Nexus::Queries::MoneyType()),
      Beam::Queries::ParameterExpression(1, Nexus::Queries::MoneyType()));
    Beam::Queries::ReduceExpression highExpression(minExpression,
      Beam::Queries::MemberAccessExpression("price",
      Nexus::Queries::MoneyType(),
      Beam::Queries::ParameterExpression(0, Queries::TimeAndSaleType())),
      initialValue);
    ChartingService::SecurityChartingQuery lowQuery;
    lowQuery.SetIndex(security);
    lowQuery.SetMarketDataType(
      MarketDataService::MarketDataType::TIME_AND_SALE);
    lowQuery.SetRange(marketStartOfDay, marketEndOfDay);
    lowQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::TAIL, 1);
    lowQuery.SetUpdatePolicy(
      Beam::Queries::ExpressionQuery::UpdatePolicy::CHANGE);
    lowQuery.SetExpression(highExpression);
    return lowQuery;
  }

  //! Submits a query for a Security's low price.
  /*!
    \param client The ChartingClient to submit the query to.
    \param security The Security to query.
    \param startDay The day to begin the low query.
    \param endDay The day to end the low query.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \param queue The Queue to store the low price in.
  */
  template<typename ChartingClient>
  void QueryDailyLow(ChartingClient& client, const Security& security,
      const boost::posix_time::ptime& startDay,
      const boost::posix_time::ptime& endDay,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      const std::shared_ptr<Beam::QueueWriter<Queries::QueryVariant>>& queue) {
    auto query = BuildDailyLowQuery(security, startDay, endDay, marketDatabase,
      timeZoneDatabase);
    client.QuerySecurity(query, queue);
  }

  //! Builds a query over a Security's volume.
  /*!
    \param security The Security to query.
    \param startDay The day to begin the volume query.
    \param endDay The day to end the volume query.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \return A SecurityChartingQuery that can be used to retrieve the
            <i>security</i>'s volume.
  */
  inline ChartingService::SecurityChartingQuery BuildDailyVolumeQuery(
      const Security& security, const boost::posix_time::ptime& startDay,
      const boost::posix_time::ptime& endDay,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto marketStartOfDay = MarketDateToUtc(security.GetMarket(), startDay,
      marketDatabase, timeZoneDatabase);
    boost::posix_time::ptime marketEndOfDay;
    if(endDay == boost::posix_time::pos_infin) {
      marketEndOfDay = boost::posix_time::pos_infin;
    } else {
      marketEndOfDay = MarketDateToUtc(security.GetMarket(), endDay,
        marketDatabase, timeZoneDatabase) + boost::gregorian::days(1);
    }
    Queries::QuantityValue initialValue(0);
    auto sumExpression = Beam::Queries::MakeAdditionExpression(
      Beam::Queries::ParameterExpression(0, Queries::QuantityType()),
      Beam::Queries::ParameterExpression(1, Queries::QuantityType()));
    Beam::Queries::ReduceExpression volumeExpression(sumExpression,
      Beam::Queries::MemberAccessExpression("size", Queries::QuantityType(),
      Beam::Queries::ParameterExpression(0, Queries::TimeAndSaleType())),
      initialValue);
    ChartingService::SecurityChartingQuery volumeQuery;
    volumeQuery.SetIndex(security);
    volumeQuery.SetMarketDataType(
      MarketDataService::MarketDataType::TIME_AND_SALE);
    volumeQuery.SetRange(marketStartOfDay, marketEndOfDay);
    volumeQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::TAIL, 1);
    volumeQuery.SetUpdatePolicy(
      Beam::Queries::ExpressionQuery::UpdatePolicy::CHANGE);
    volumeQuery.SetExpression(volumeExpression);
    return volumeQuery;
  }

  //! Submits a query for a Security's daily volume.
  /*!
    \param client The ChartingClient to submit the query to.
    \param security The Security to query.
    \param startDay The day to begin the volume query.
    \param endDay The day to end the volume query.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \param queue The Queue to store the volume in.
  */
  template<typename ChartingClient>
  void QueryDailyVolume(ChartingClient& client, const Security& security,
      const boost::posix_time::ptime& startDay,
      const boost::posix_time::ptime& endDay,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      const std::shared_ptr<Beam::QueueWriter<Queries::QueryVariant>>& queue) {
    auto query = BuildDailyVolumeQuery(security, startDay, endDay,
      marketDatabase, timeZoneDatabase);
    client.QuerySecurity(query, queue);
  }
}
}

#endif
