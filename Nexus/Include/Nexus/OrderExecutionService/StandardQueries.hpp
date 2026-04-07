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

namespace Nexus {

  /**
   * Returns a Query Expression to filter Orders by venue.
   * @param venue The venue to query.
   * @return A Query Expression that filters any Order not submitted to the
   *         specified <i>venue</i>.
   */
  inline auto make_venue_filter(Venue venue) {
    return std::string(venue.get_code().get_data()) ==
      SecurityAccessor(OrderFieldsAccessor(OrderInfoAccessor::from_parameter(0).
        get_fields()).get_security()).get_venue();
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
      const Beam::DirectoryEntry& account, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones) {
    auto venue_start = utc_start_of_day(venue, start, venues, time_zones);
    auto venue_end = [&] {
      if(end == boost::posix_time::pos_infin) {
        return boost::posix_time::ptime(boost::posix_time::pos_infin);
      } else {
        return utc_start_of_day(venue, end, venues, time_zones) +
          boost::gregorian::days(1);
      }
    }();
    auto venue_filter = make_venue_filter(venue);
    auto query = AccountQuery();
    query.set_index(account);
    query.set_range(venue_start, venue_end);
    query.set_filter(venue_filter);
    query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
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
  Beam::Routine::Id query_daily_order_submissions(
      const Beam::DirectoryEntry& account, boost::posix_time::ptime start,
      boost::posix_time::ptime end, const VenueDatabase& venues,
      const boost::local_time::tz_database& time_zones,
      IsOrderExecutionClient auto& client,
      Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue) {
    auto venue_time_zones =
      std::unordered_map<std::string, std::vector<Venue>>();
    for(auto& venue : venues.get_entries()) {
      venue_time_zones[venue.m_time_zone].push_back(venue.m_venue);
    }
    auto snapshots =
      std::vector<std::shared_ptr<Beam::Queue<SequencedOrder>>>();
    for(auto& venue_time_zone : venue_time_zones) {
      auto venue_start = utc_start_of_day(
        venue_time_zone.second.front(), start, venues, time_zones);
      auto venue_end = utc_start_of_day(venue_time_zone.second.front(),
        end, venues, time_zones) + boost::gregorian::days(1);
      auto venue_expressions = std::vector<Beam::Expression>();
      for(auto& venue : venue_time_zone.second) {
        venue_expressions.push_back(make_venue_filter(venue));
      }
      auto venue_filter =
        Beam::disjunction(venue_expressions.begin(), venue_expressions.end());
      auto query = AccountQuery();
      query.set_index(account);
      query.set_range(venue_start, venue_end);
      query.set_filter(venue_filter);
      query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
      auto snapshot_queue = std::make_shared<Beam::Queue<SequencedOrder>>();
      client.query(query, snapshot_queue);
      snapshots.push_back(std::move(snapshot_queue));
    }
    return Beam::spawn(
      [snapshots = std::move(snapshots), queue = std::move(queue)] () mutable {
        for(auto& snapshot : snapshots) {
          try {
            while(true) {
              queue.push(snapshot->pop().get_value());
            }
          } catch(const std::exception&) {}
        }
      });
  }

  /** Returns a Query Expression to filter an account's live Orders. */
  inline auto make_live_orders_filter() {
    return OrderInfoAccessor::from_parameter(0).is_live();
  }

  /**
   * Returns a query to retrieve all of an account's live Orders.
   * @param account The account to query.
   */
  inline AccountQuery make_live_orders_query(
      const Beam::DirectoryEntry& account) {
    auto query = AccountQuery();
    query.set_index(account);
    query.set_range(Beam::Range::HISTORICAL);
    query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
    query.set_filter(make_live_orders_filter());
    return query;
  }

  /**
   * Queries an account's live Orders.
   * @param account The account to query.
   * @param client The OrderExecutionClient to query.
   * @param queue The Queue to write to.
   */
  void query_live_orders(const Beam::DirectoryEntry& account,
      IsOrderExecutionClient auto& client,
      Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue) {
    client.query(make_live_orders_query(account), std::move(queue));
  }

  /**
   * Loads an account's live Orders.
   * @param account The account to query.
   * @param client The OrderExecutionClient to query.
   */
  std::vector<std::shared_ptr<Order>> load_live_orders(
      const Beam::DirectoryEntry& account,
      IsOrderExecutionClient auto& client) {
    auto queue = std::make_shared<Beam::Queue<std::shared_ptr<Order>>>();
    query_live_orders(account, client, queue);
    auto orders = std::vector<std::shared_ptr<Order>>();
    Beam::flush(queue, std::back_inserter(orders));
    return orders;
  }

  /**
   * Returns a Query Expression to filter Orders by id.
   * @param ids The order ids to filter.
   */
  inline auto make_order_id_filter(const std::vector<OrderId>& ids) {
    auto clauses = std::vector<Beam::Expression>();
    std::transform(ids.begin(), ids.end(), std::back_inserter(clauses),
      [&] (auto id) {
        return OrderInfoAccessor::from_parameter(0).get_order_id() == id;
      });
    return Beam::disjunction(clauses.begin(), clauses.end());
  }

  /**
   * Returns a query to retrieve Orders by their id.
   * @param account The account to query.
   * @param ids The order ids to query.
   */
  inline AccountQuery make_order_id_query(const Beam::DirectoryEntry& account,
      const std::vector<OrderId>& ids) {
    auto query = AccountQuery();
    query.set_index(account);
    query.set_range(Beam::Range::HISTORICAL);
    query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
    query.set_filter(make_order_id_filter(ids));
    return query;
  }

  /**
   * Queries for Orders by their id.
   * @param account The account to query.
   * @param ids The order ids to query.
   * @param client The OrderExecutionClient to query.
   * @param queue The Queue to write to.
   */
  void query_order_ids(const Beam::DirectoryEntry& account,
      const std::vector<OrderId>& ids, IsOrderExecutionClient auto& client,
      Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue) {
    client.query(make_order_id_query(account, ids), std::move(queue));
  }

  /**
   * Loads a list of Orders by id.
   * @param account The account to query.
   * @param ids The order ids to query.
   * @param client The OrderExecutionClient to query.
   */
  std::vector<std::shared_ptr<Order>> load_orders(
      const Beam::DirectoryEntry& account,
      const std::vector<OrderId>& ids, IsOrderExecutionClient auto& client) {
    auto queue = std::make_shared<Beam::Queue<std::shared_ptr<Order>>>();
    query_order_ids(account, ids, client, queue);
    auto orders = std::vector<std::shared_ptr<Order>>();
    Beam::flush(queue, std::back_inserter(orders));
    return orders;
  }
}

#endif
