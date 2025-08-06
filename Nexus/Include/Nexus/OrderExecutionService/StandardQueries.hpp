#ifndef NEXUS_ORDER_EXECUTION_SERVICE_STANDARD_QUERIES_HPP
#define NEXUS_ORDER_EXECUTION_SERVICE_STANDARD_QUERIES_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include <Beam/Queries/StandardValues.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/Routine.hpp>
#include <boost/date_time/local_time/tz_database.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Returns a Query Expression to filter Orders by venue.
   * @param venue The venue to query.
   * @return A Query Expression that filters any Order not submitted to the
   *         specified <i>venue</i>.
   */
  inline auto make_venue_filter(Venue venue) {
    auto query_venue = Beam::Queries::StringValue(venue.get_code().GetData());
    auto venue_expression = Beam::Queries::ConstantExpression(query_venue);
    auto info_parameter =
      Beam::Queries::ParameterExpression(0, Nexus::Queries::OrderInfoType());
    auto fields_accessor = Beam::Queries::MemberAccessExpression(
      "fields", Nexus::Queries::OrderFieldsType(), info_parameter);
    auto security_accessor = Beam::Queries::MemberAccessExpression(
      "security", Nexus::Queries::SecurityType(), fields_accessor);
    auto venue_accessor = Beam::Queries::MemberAccessExpression(
      "venue", Beam::Queries::StringType(), security_accessor);
    return Beam::Queries::MakeEqualsExpression(
      venue_expression, venue_accessor);
  }

  /**
   * Returns a query to retrieve Orders submitted to a specified venue on a
   * daily basis.
   * @param venue The venue to query.
   * @param account The account to query.
   * @param start The first day to retrieve order submissions for.
   * @param end The last day to retrieve order submissions for.
   * @param venues The database containing available venues.
   * @param time_zones The database of timezones.
   * @return An AccountQuery that can be used to retrieve the daily Order
   *         submissions for the specified <i>account</i> on the specified
   *         <i>venue</i>.
   */
  inline AccountQuery make_daily_order_submission_query(Venue venue,
      const Beam::ServiceLocator::DirectoryEntry& account,
      boost::posix_time::ptime start, boost::posix_time::ptime end,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto venue_start = venue_date_to_utc(venue, start, venues, time_zones);
    auto venue_end = [&] {
      if(end == boost::posix_time::pos_infin) {
        return boost::posix_time::ptime(boost::posix_time::pos_infin);
      } else {
        return venue_date_to_utc(venue, end, venues, time_zones) +
          boost::gregorian::days(1);
      }
    }();
    auto venue_filter = make_venue_filter(venue);
    auto query = AccountQuery();
    query.SetIndex(account);
    query.SetRange(venue_start, venue_end);
    query.SetFilter(venue_filter);
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    return query;
  }

  /**
   * Queries for Orders submitted to a specified venue on a daily basis.
   * @param account The account to query.
   * @param start The first day to retrieve order submissions for.
   * @param end The last day to retrieve order submissions for.
   * @param venues The database containing available venues.
   * @param time_zones The database of timezones.
   * @param client The OrderExecutionClient to query.
   * @param queue The Queue to write to.
   */
  template<IsOrderExecutionClient C>
  Beam::Routines::Routine::Id query_daily_order_submissions(
      const Beam::ServiceLocator::DirectoryEntry& account,
      boost::posix_time::ptime start, boost::posix_time::ptime end,
      const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      C&& client, Beam::ScopedQueueWriter<std::shared_ptr<const Order>> queue) {
    return Beam::Routines::Spawn([=, queue = std::move(queue),
        client = Beam::CapturePtr<C>(client)] () mutable {
      auto venue_time_zones =
        std::unordered_map<std::string, std::vector<Venue>>();
      for(auto& venue : venues.get_entries()) {
        venue_time_zones[venue.m_time_zone].push_back(venue.m_venue);
      }
      for(auto& venue_time_zone : venue_time_zones) {
        auto venue_start = venue_date_to_utc(
          venue_time_zone.second.front(), start, venues, time_zones);
        auto venue_end = venue_date_to_utc(venue_time_zone.second.front(),
          end, venues, time_zones) + boost::gregorian::days(1);
        auto venue_expressions = std::vector<Beam::Queries::Expression>();
        for(auto& venue : venue_time_zone.second) {
          venue_expressions.push_back(make_venue_filter(venue));
        }
        auto venue_filter = Beam::Queries::MakeOrExpression(
          venue_expressions.begin(), venue_expressions.end());
        auto query = AccountQuery();
        query.SetIndex(account);
        query.SetRange(venue_start, venue_end);
        query.SetFilter(venue_filter);
        query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
        auto snapshot_queue = std::make_shared<Beam::Queue<SequencedOrder>>();
        client->query(query, snapshot_queue);
        try {
          while(true) {
            auto value = snapshot_queue->Pop();
            if(value.GetValue()->get_info().m_timestamp < venue_end) {
              queue->Push(std::move(value.GetValue()));
            }
          }
        } catch(const std::exception&) {}
      }
    });
  }

  /** Returns a Query Expression to filter an account's live Orders. */
  inline auto make_live_orders_filter() {
    auto info =
      Beam::Queries::ParameterExpression(0, Nexus::Queries::OrderInfoType());
    return Beam::Queries::MemberAccessExpression(
      "is_live", Beam::Queries::NativeDataType<bool>(), info);
  }

  /**
   * Returns a query to retrieve all of an account's live Orders.
   * @param account The account to query.
   */
  inline AccountQuery make_live_orders_query(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto query = AccountQuery();
    query.SetIndex(account);
    query.SetRange(Beam::Queries::Range::Historical());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    query.SetFilter(make_live_orders_filter());
    return query;
  }

  /**
   * Queries an account's live Orders.
   * @param account The account to query.
   * @param client The OrderExecutionClient to query.
   * @param queue The Queue to write to.
   */
  void query_live_orders(const Beam::ServiceLocator::DirectoryEntry& account,
      IsOrderExecutionClient auto& client,
      Beam::ScopedQueueWriter<std::shared_ptr<const Order>> queue) {
    client.query(make_live_orders_query(account), std::move(queue));
  }

  /**
   * Loads an account's live Orders.
   * @param account The account to query.
   * @param client The OrderExecutionClient to query.
   */
  std::vector<std::shared_ptr<const Order>> load_live_orders(
      const Beam::ServiceLocator::DirectoryEntry& account,
      IsOrderExecutionClient auto& client) {
    auto queue = std::make_shared<Beam::Queue<std::shared_ptr<const Order>>>();
    query_live_orders(account, client, queue);
    auto orders = std::vector<std::shared_ptr<const Order>>();
    Beam::Flush(queue, std::back_inserter(orders));
    return orders;
  }

  /**
   * Returns a Query Expression to filter Orders by id.
   * @param ids The order ids to filter.
   */
  inline auto make_order_id_filter(const std::vector<OrderId>& ids) {
    auto info =
      Beam::Queries::ParameterExpression(0, Nexus::Queries::OrderInfoType());
    auto field = Beam::Queries::MemberAccessExpression(
      "order_id", Beam::Queries::NativeDataType<OrderId>(), info);
    auto clauses = std::vector<Beam::Queries::Expression>();
    std::transform(ids.begin(), ids.end(), std::back_inserter(clauses),
      [&] (auto id) {
        return Beam::Queries::MakeEqualsExpression(field,
          Beam::Queries::ConstantExpression(Beam::Queries::NativeValue(id)));
      });
    return Beam::Queries::MakeOrExpression(clauses.begin(), clauses.end());
  }

  /**
   * Returns a query to retrieve Orders by their id.
   * @param account The account to query.
   * @param ids The order ids to query.
   */
  inline AccountQuery make_order_id_query(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<OrderId>& ids) {
    auto query = AccountQuery();
    query.SetIndex(account);
    query.SetRange(Beam::Queries::Range::Historical());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    query.SetFilter(make_order_id_filter(ids));
    return query;
  }

  /**
   * Queries for Orders by their id.
   * @param account The account to query.
   * @param ids The order ids to query.
   * @param client The OrderExecutionClient to query.
   * @param queue The Queue to write to.
   */
  void query_order_ids(const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<OrderId>& ids, IsOrderExecutionClient auto& client,
      Beam::ScopedQueueWriter<std::shared_ptr<const Order>> queue) {
    client.query(make_order_id_query(account, ids), std::move(queue));
  }

  /**
   * Loads a list of Orders by id.
   * @param account The account to query.
   * @param ids The order ids to query.
   * @param client The OrderExecutionClient to query.
   */
  std::vector<std::shared_ptr<const Order>> load_orders(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<OrderId>& ids, IsOrderExecutionClient auto& client) {
    auto queue = std::make_shared<Beam::Queue<std::shared_ptr<const Order>>>();
    query_order_ids(account, ids, client, queue);
    auto orders = std::vector<std::shared_ptr<const Order>>();
    Beam::Flush(queue, std::back_inserter(orders));
    return orders;
  }
}

#endif
