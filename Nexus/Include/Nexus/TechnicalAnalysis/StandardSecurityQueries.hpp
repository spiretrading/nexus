#ifndef NEXUS_STANDARD_SECURITY_QUERIES_HPP
#define NEXUS_STANDARD_SECURITY_QUERIES_HPP
#include <memory>
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
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/StandardValues.hpp"
#include "Nexus/Queries/TraversalExpressionVisitor.hpp"

namespace Nexus::TechnicalAnalysis {

  /**
   * Returns a query to retrieve a Security's opening trade.
   * @param security The Security to query.
   * @param date The date to retrieve the opening trade for.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @return A SecurityMarketDataQuery that can be used to retrieve the
   *         <i>security</i>'s opening trade.
   */
  inline MarketDataService::SecurityMarketDataQuery make_open_query(
      const Security& security, boost::posix_time::ptime date,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto start_of_day =
      venue_date_to_utc(security.get_venue(), date, venues, time_zones);
    auto query = MarketDataService::SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(
      start_of_day, Beam::Queries::Decrement(Beam::Queries::Sequence::Last()));
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromHead(1));
    auto parameter =
      Beam::Queries::ParameterExpression(0, Nexus::Queries::TimeAndSaleType());
    auto market_center = venues.from(security.get_venue()).m_market_center;
    query.SetFilter(
      Beam::Queries::ConstantExpression(std::move(market_center)) ==
        Queries::TimeAndSaleAccessor(parameter).m_marketCenter);
    return query;
  }

  /**
   * Queries for a Security's opening trade.
   * @param client The MarketDataClient to query.
   * @param security The Security to query.
   * @param date The date to retrieve the opening trade for.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @return The opening trade for the specified <i>security</i>.
   */
  template<typename MarketDataClient>
  boost::optional<TimeAndSale> load_open(MarketDataClient& client,
      const Security& security, boost::posix_time::ptime date,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto query = make_open_query(security, date, venues, time_zones);
    auto queue = std::make_shared<Beam::Queue<TimeAndSale>>();
    client.query(query, queue);
    auto open = boost::optional<TimeAndSale>();
    try {
      open = queue->Pop();
    } catch(const std::exception&) {}
    return open;
  }

  /**
   * Queries for a Security's opening trade.
   * @param client The MarketDataClient to query.
   * @param security The Security to query.
   * @param date The date to retrieve the opening trade for.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @param queue The Queue to store the opening trade in.
   */
  template<typename MarketDataClient>
  void query_open(MarketDataClient&& client, const Security& security,
      boost::posix_time::ptime date, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    Beam::Routines::Spawn(
      [=, client = Beam::CapturePtr<MarketDataClient>(client),
          queue = std::move(queue)] () mutable {
        if(auto open = load_open(*client, security, date, venues, time_zones)) {
          queue.Push(*open);
          return;
        }
        auto query = make_open_query(security, date, venues, time_zones);
        query.SetRange(
          query.GetRange().GetStart(), Beam::Queries::Sequence::Last());
        query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromHead(1));
        auto local_queue = std::make_shared<Beam::Queue<TimeAndSale>>();
        client->query(query, local_queue);
        try {
          auto time_and_sale = local_queue->Pop();
          queue.Push(std::move(time_and_sale));
        } catch(const std::exception&) {}
        local_queue->Break();
      });
  }

  /**
   * Returns a query to retrieve a Security's previous session's closing trade.
   * @param security The Security to query.
   * @param date The date for which the previous trading session's closing trade
   *        will be retrieved.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @return A SecurityMarketDataQuery that can be used to retrieve the
   *         <i>security</i>'s previous session's closing trade.
   */
  inline MarketDataService::SecurityMarketDataQuery make_previous_close_query(
      const Security& security, boost::posix_time::ptime date,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto start_of_day =
      venue_date_to_utc(security.get_venue(), date, venues, time_zones);
    auto query = MarketDataService::SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Beam::Queries::Sequence::First(), start_of_day);
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromTail(1));
    auto parameter =
      Beam::Queries::ParameterExpression(0, Nexus::Queries::TimeAndSaleType());
    auto market_center = venues.from(security.get_venue()).m_market_center;
    query.SetFilter(
      Beam::Queries::ConstantExpression(std::move(market_center)) ==
        Queries::TimeAndSaleAccessor(parameter).m_marketCenter);
    return query;
  }

  /**
   * Queries for a Security's previous session's closing trade.
   * @param client The MarketDataClient to query.
   * @param security The Security to query.
   * @param date The date for which the previous trading session's closing trade
   *        will be retrieved.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @return The previous session's closing trade for the specified
   *         <i>security</i>.
   */
  template<typename MarketDataClient>
  boost::optional<TimeAndSale> load_previous_close(MarketDataClient& client,
      const Security& security, boost::posix_time::ptime date,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto query = make_previous_close_query(security, date, venues, time_zones);
    auto queue = std::make_shared<Beam::Queue<TimeAndSale>>();
    client.query(query, queue);
    auto previous_close = boost::optional<TimeAndSale>();
    try {
      previous_close = queue->Pop();
    } catch(const std::exception&) {}
    return previous_close;
  }

  /**
   * Makes a range suitable for a query covering a range of days.
   * @param security The Security to query.
   * @param start The day to begin the query.
   * @param end The day to end the query.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @return A Range object for the daily query.
   */
  inline Beam::Queries::Range make_daily_query_range(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto start_of_day =
      venue_date_to_utc(security.get_venue(), start, venues, time_zones);
    auto end_of_day = [&] () -> boost::posix_time::ptime {
      if(end == boost::posix_time::pos_infin) {
        return boost::posix_time::pos_infin;
      }
      return venue_date_to_utc(security.get_venue(), end, venues,
        time_zones) + boost::gregorian::days(1);
    }();
    return Beam::Queries::Range(start_of_day, end_of_day);
  }

  /**
   * Makes a charting query for a security.
   * @param security The Security to query.
   * @param start The day to begin the query.
   * @param end The day to end the query.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @param expression The expression to apply to the data.
   * @return A SecurityChartingQuery configured with the specified parameters.
   */
  inline ChartingService::SecurityChartingQuery make_query(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      const Beam::Queries::Expression& expression) {
    auto query = ChartingService::SecurityChartingQuery();
    query.SetIndex(security);
    struct MarketDataTypeFinder : Queries::TraversalExpressionVisitor {
      MarketDataService::MarketDataType m_type;

      MarketDataTypeFinder()
        : m_type(MarketDataService::MarketDataType::NONE) {}

      void Visit(
          const Beam::Queries::ParameterExpression& expression) override {
        if(m_type != MarketDataService::MarketDataType::NONE) {
          return;
        }
        if(expression.GetType() == Queries::TimeAndSaleType()) {
          m_type = MarketDataService::MarketDataType::TIME_AND_SALE;
        } else if(expression.GetType() == Queries::BookQuoteType()) {
          m_type = MarketDataService::MarketDataType::BOOK_QUOTE;
        } else if(expression.GetType() == Queries::BboQuoteType()) {
          m_type = MarketDataService::MarketDataType::BBO_QUOTE;
        }
      }

      MarketDataService::MarketDataType GetType() const {
        return m_type;
      }
    };
    auto type = [&] {
      auto visitor = MarketDataTypeFinder();
      expression->Apply(visitor);
      return visitor.GetType();
    }();
    query.set_market_data_type(type);
    query.SetRange(
      make_daily_query_range(query.GetIndex(), start, end, venues, time_zones));
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromTail(1));
    query.SetUpdatePolicy(Beam::Queries::ExpressionQuery::UpdatePolicy::CHANGE);
    query.SetExpression(expression);
    return query;
  }

  /**
   * Returns a query for a Security's high price.
   * @param security The Security to query.
   * @param start The day to begin the high query.
   * @param end The day to end the high query.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @return A SecurityChartingQuery that can be used to retrieve the
   *         <i>security</i>'s high price.
   */
  inline ChartingService::SecurityChartingQuery make_daily_high_query(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto max = Beam::Queries::MakeMaxExpression(
      Beam::Queries::ParameterExpression(0, Nexus::Queries::MoneyType()),
      Beam::Queries::ParameterExpression(1, Nexus::Queries::MoneyType()));
    auto high = Beam::Queries::ReduceExpression(
      max, Queries::TimeAndSaleAccessor(Beam::Queries::ParameterExpression(
        0, Queries::TimeAndSaleType())).m_price,
      Queries::MoneyValue(Money::ZERO));
    return make_query(security, start, end, venues, time_zones, high);
  }

  /**
   * Submits a query for a Security's high price.
   * @param client The ChartingClient to submit the query to.
   * @param security The Security to query.
   * @param start The day to begin the high query.
   * @param end The day to end the high query.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @param queue The Queue to store the high price in.
   */
  template<typename ChartingClient>
  void query_daily_high(ChartingClient& client, const Security& security,
      boost::posix_time::ptime start, boost::posix_time::ptime end,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      Beam::ScopedQueueWriter<Money> queue) {
    client.query(
      make_daily_high_query(security, start, end, venues, time_zones),
      Beam::MakeConverterQueueWriter<Queries::QueryVariant>(std::move(queue),
        [] (const Queries::QueryVariant& value) {
          return boost::get<Money>(value);
        }));
  }

  /**
   * Returns a query for a Security's low price.
   * @param security The Security to query.
   * @param start The day to begin the low query.
   * @param end The day to end the low query.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @return A SecurityChartingQuery that can be used to retrieve the
   *         <i>security</i>'s low price.
   */
  inline ChartingService::SecurityChartingQuery make_daily_low_query(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto min = Beam::Queries::MakeMinExpression(
      Beam::Queries::ParameterExpression(0, Nexus::Queries::MoneyType()),
      Beam::Queries::ParameterExpression(1, Nexus::Queries::MoneyType()));
    auto low = Beam::Queries::ReduceExpression(
      min, Queries::TimeAndSaleAccessor(Beam::Queries::ParameterExpression(
        0, Queries::TimeAndSaleType())).m_price,
      Queries::MoneyValue(99999999 * Money::ONE));
    return make_query(security, start, end, venues, time_zones, low);
  }

  /**
   * Submits a query for a Security's low price.
   * @param client The ChartingClient to submit the query to.
   * @param security The Security to query.
   * @param start The day to begin the low query.
   * @param end The day to end the low query.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @param queue The Queue to store the low price in.
   */
  template<typename ChartingClient>
  void query_daily_low(ChartingClient& client, const Security& security,
      boost::posix_time::ptime start, boost::posix_time::ptime end,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      Beam::ScopedQueueWriter<Money> queue) {
    client.query(make_daily_low_query(security, start, end, venues, time_zones),
      Beam::MakeConverterQueueWriter<Queries::QueryVariant>(std::move(queue),
        [] (const Queries::QueryVariant& value) {
          return boost::get<Money>(value);
        }));
  }

  /**
   * Returns a query over a Security's volume.
   * @param security The Security to query.
   * @param start The day to begin the volume query.
   * @param end The day to end the volume query.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @return A SecurityChartingQuery that can be used to retrieve the
   *         <i>security</i>'s volume.
   */
  inline ChartingService::SecurityChartingQuery make_daily_volume_query(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto sum = Beam::Queries::ParameterExpression(0, Queries::QuantityType()) +
      Beam::Queries::ParameterExpression(1, Queries::QuantityType());
    auto volume = Beam::Queries::ReduceExpression(
      sum, Queries::TimeAndSaleAccessor(Beam::Queries::ParameterExpression(
        0, Queries::TimeAndSaleType())).m_size, Queries::QuantityValue(0));
    return make_query(security, start, end, venues, time_zones, volume);
  }

  /**
   * Submits a query for a Security's daily volume.
   * @param client The ChartingClient to submit the query to.
   * @param security The Security to query.
   * @param start The day to begin the volume query.
   * @param end The day to end the volume query.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @param queue The Queue to store the volume in.
   */
  template<typename ChartingClient>
  void query_daily_volume(ChartingClient& client, Security security,
      boost::posix_time::ptime start, boost::posix_time::ptime end,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      Beam::ScopedQueueWriter<Quantity> queue) {
    client.query(make_daily_volume_query(
        security, start, end, venues, time_zones),
      Beam::MakeConverterQueueWriter<Queries::QueryVariant>(std::move(queue),
        [] (const Queries::QueryVariant& value) {
          return boost::get<Quantity>(value);
        }));
  }
}

#endif
