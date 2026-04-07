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
#include "Nexus/ChartingService/ChartingClient.hpp"
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/StandardValues.hpp"
#include "Nexus/Queries/TraversalExpressionVisitor.hpp"

namespace Nexus {

  /**
   * Returns a query to retrieve a Security's opening trade.
   * @param security The Security to query.
   * @param date The date to retrieve the opening trade for.
   * @param venues The database containing venue time zones.
   * @param time_zones The database of timezones.
   * @return A SecurityMarketDataQuery that can be used to retrieve the
   *         <i>security</i>'s opening trade.
   */
  inline SecurityMarketDataQuery make_open_query(
      const Security& security, boost::posix_time::ptime date,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto start_of_day =
      utc_start_of_day(security.get_venue(), date, venues, time_zones);
    auto query = SecurityMarketDataQuery();
    query.set_index(security);
    query.set_range(start_of_day, Beam::decrement(Beam::Sequence::LAST));
    query.set_snapshot_limit(Beam::SnapshotLimit::from_head(1));
    query.set_filter(venues.from(security.get_venue()).m_market_center ==
      TimeAndSaleAccessor::from_parameter(0).get_market_center());
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
  boost::optional<TimeAndSale> load_open(IsMarketDataClient auto& client,
      const Security& security, boost::posix_time::ptime date,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto query = make_open_query(security, date, venues, time_zones);
    auto queue = std::make_shared<Beam::Queue<TimeAndSale>>();
    client.query(query, queue);
    auto open = boost::optional<TimeAndSale>();
    try {
      open = queue->pop();
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
  Beam::Routine::Id query_open(IsMarketDataClient auto& client,
      const Security& security, boost::posix_time::ptime date,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    return Beam::spawn([=, &client, queue = std::move(queue)] () mutable {
      if(auto open = load_open(client, security, date, venues, time_zones)) {
        queue.push(*open);
        return;
      }
      auto query = make_open_query(security, date, venues, time_zones);
      query.set_range(query.get_range().get_start(), Beam::Sequence::LAST);
      query.set_snapshot_limit(Beam::SnapshotLimit::from_head(1));
      auto local_queue = std::make_shared<Beam::Queue<TimeAndSale>>();
      client.query(query, local_queue);
      try {
        auto time_and_sale = local_queue->pop();
        queue.push(std::move(time_and_sale));
      } catch(const std::exception&) {}
      local_queue->close();
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
  inline SecurityMarketDataQuery make_previous_close_query(
      const Security& security, boost::posix_time::ptime date,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto start_of_day =
      utc_start_of_day(security.get_venue(), date, venues, time_zones);
    auto query = SecurityMarketDataQuery();
    query.set_index(security);
    query.set_range(Beam::Sequence::FIRST, start_of_day);
    query.set_snapshot_limit(Beam::SnapshotLimit::from_tail(1));
    auto market_center = venues.from(security.get_venue()).m_market_center;
    query.set_filter(venues.from(security.get_venue()).m_market_center ==
      TimeAndSaleAccessor::from_parameter(0).get_market_center());
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
  boost::optional<TimeAndSale> load_previous_close(
      IsMarketDataClient auto& client, const Security& security,
      boost::posix_time::ptime date, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto query = make_previous_close_query(security, date, venues, time_zones);
    auto queue = std::make_shared<Beam::Queue<TimeAndSale>>();
    client.query(query, queue);
    auto previous_close = boost::optional<TimeAndSale>();
    try {
      previous_close = queue->pop();
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
  inline Beam::Range make_daily_query_range(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto start_of_day =
      utc_start_of_day(security.get_venue(), start, venues, time_zones);
    auto end_of_day = [&] () -> boost::posix_time::ptime {
      if(end == boost::posix_time::pos_infin) {
        return boost::posix_time::pos_infin;
      }
      return utc_end_of_day(security.get_venue(), end, venues, time_zones);
    }();
    return Beam::Range(start_of_day, end_of_day);
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
  inline SecurityChartingQuery make_query(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      const Beam::Expression& expression) {
    auto query = SecurityChartingQuery();
    query.set_index(security);
    struct MarketDataTypeFinder : TraversalExpressionVisitor {
      MarketDataType m_type;

      MarketDataTypeFinder()
        : m_type(MarketDataType::NONE) {}

      void visit(const Beam::ParameterExpression& expression) override {
        if(m_type != MarketDataType::NONE) {
          return;
        }
        if(expression.get_type() == typeid(TimeAndSale)) {
          m_type = MarketDataType::TIME_AND_SALE;
        } else if(expression.get_type() == typeid(BookQuote)) {
          m_type = MarketDataType::BOOK_QUOTE;
        } else if(expression.get_type() == typeid(BboQuote)) {
          m_type = MarketDataType::BBO_QUOTE;
        }
      }

      MarketDataType get_type() const {
        return m_type;
      }
    };
    auto type = [&] {
      auto visitor = MarketDataTypeFinder();
      expression.apply(visitor);
      return visitor.get_type();
    }();
    query.set_market_data_type(type);
    query.set_range(make_daily_query_range(
      query.get_index(), start, end, venues, time_zones));
    query.set_snapshot_limit(Beam::SnapshotLimit::from_tail(1));
    query.set_update_policy(Beam::ExpressionQuery::UpdatePolicy::CHANGE);
    query.set_expression(expression);
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
  inline SecurityChartingQuery make_daily_high_query(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto max = Beam::max(Beam::ParameterExpression(0, typeid(Money)),
      Beam::ParameterExpression(1, typeid(Money)));
    auto high = Beam::ReduceExpression(
      max, TimeAndSaleAccessor::from_parameter(0).get_price(), Money::ZERO);
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
  void query_daily_high(IsChartingClient auto& client,
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      Beam::ScopedQueueWriter<Money> queue) {
    client.query(
      make_daily_high_query(security, start, end, venues, time_zones),
      Beam::convert<QueryVariant>(std::move(queue),
        [] (const QueryVariant& value) {
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
  inline SecurityChartingQuery make_daily_low_query(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto min = Beam::min(Beam::ParameterExpression(0, typeid(Money)),
      Beam::ParameterExpression(1, typeid(Money)));
    auto low = Beam::ReduceExpression(
      min, TimeAndSaleAccessor::from_parameter(0).get_price(),
      99999999 * Money::ONE);
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
  void query_daily_low(IsChartingClient auto& client,
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      Beam::ScopedQueueWriter<Money> queue) {
    client.query(make_daily_low_query(security, start, end, venues, time_zones),
      Beam::convert<QueryVariant>(std::move(queue),
        [] (const QueryVariant& value) {
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
  inline SecurityChartingQuery make_daily_volume_query(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto sum = Beam::ParameterExpression(0, typeid(Quantity)) +
      Beam::ParameterExpression(1, typeid(Quantity));
    auto volume = Beam::ReduceExpression(
      sum, TimeAndSaleAccessor::from_parameter(0).get_size(), Quantity(0));
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
  void query_daily_volume(IsChartingClient auto& client, Security security,
      boost::posix_time::ptime start, boost::posix_time::ptime end,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      Beam::ScopedQueueWriter<Quantity> queue) {
    client.query(
      make_daily_volume_query(security, start, end, venues, time_zones),
      Beam::convert<QueryVariant>(std::move(queue),
        [] (const QueryVariant& value) {
          return boost::get<Quantity>(value);
        }));
  }
}

#endif
