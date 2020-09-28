#ifndef NEXUS_ORDER_EXECUTION_SERVICE_STANDARD_QUERIES_HPP
#define NEXUS_ORDER_EXECUTION_SERVICE_STANDARD_QUERIES_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include <Beam/Queries/StandardValues.hpp>
#include <Beam/Routines/Routine.hpp>
#include <boost/date_time/local_time/tz_database.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Builds a Query Expression to filter Orders by MarketCode.
   * @param market The market to query.
   * @return A Query Expression that filters any Order not submitted to the
   *         specified <i>market</i>.
   */
  inline auto BuildMarketFilter(MarketCode market) {
    auto queryMarketCode = Beam::Queries::StringValue(market.GetData());
    auto marketCodeExpression = Beam::Queries::ConstantExpression(
      queryMarketCode);
    auto infoParameterExpression = Beam::Queries::ParameterExpression(
      0, Nexus::Queries::OrderInfoType());
    auto fieldsAccessExpression = Beam::Queries::MemberAccessExpression(
      "fields", Nexus::Queries::OrderFieldsType(), infoParameterExpression);
    auto securityAccessExpression = Beam::Queries::MemberAccessExpression(
      "security", Nexus::Queries::SecurityType(), fieldsAccessExpression);
    auto marketAccessExpression = Beam::Queries::MemberAccessExpression(
      "market", Beam::Queries::StringType(), securityAccessExpression);
    auto equalExpression = Beam::Queries::MakeEqualsExpression(
      marketCodeExpression, marketAccessExpression);
    return equalExpression;
  }

  /**
   * Builds a query to retrieve Orders submitted to a specified market on
   * on a daily basis.
   * @param market The market to query.
   * @param account The account to query.
   * @param startTime The first day to retrieve order submissions for.
   * @param endTime The last day to retrieve order submissions for.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @return An AccountQuery that can be used to retrieve the daily Order
   *         submissions for the specified <i>account</i> on the specified
   *         <i>market</i>.
   */
  inline AccountQuery BuildDailyOrderSubmissionQuery(MarketCode market,
      const Beam::ServiceLocator::DirectoryEntry& account,
      boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto marketStartOfDay = MarketDateToUtc(market, startTime, marketDatabase,
      timeZoneDatabase);
    auto marketEndOfDay = [&] () {
      if(endTime == boost::posix_time::pos_infin) {
        return boost::posix_time::ptime(boost::posix_time::pos_infin);
      } else {
        return MarketDateToUtc(market, endTime, marketDatabase,
          timeZoneDatabase) + boost::gregorian::days(1);
      }
    }();
    auto marketFilter = BuildMarketFilter(market);
    auto dailyOrderSubmissionQuery = AccountQuery();
    dailyOrderSubmissionQuery.SetIndex(account);
    dailyOrderSubmissionQuery.SetRange(marketStartOfDay, marketEndOfDay);
    dailyOrderSubmissionQuery.SetFilter(marketFilter);
    dailyOrderSubmissionQuery.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Unlimited());
    return dailyOrderSubmissionQuery;
  }

  /**
   * Queries for Orders submitted to a specified market on a daily basis.
   * @param account The account to query.
   * @param startTime The first day to retrieve order submissions for.
   * @param endTime The last day to retrieve order submissions for.
   * @param marketDatabase The database containing Market info.
   * @param timeZoneDatabase The database of timezones.
   * @param orderExecutionClient The OrderExecutionClient to query.
   * @param queue The Queue to write to.
   */
  template<typename OrderExecutionClient>
  void QueryDailyOrderSubmissions(
      const Beam::ServiceLocator::DirectoryEntry& account,
      boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      OrderExecutionClient& orderExecutionClient,
      Beam::ScopedQueueWriter<const Order*> queue) {
    Beam::Routines::Spawn([=, queue = std::move(queue),
        &orderExecutionClient] () mutable {
      auto marketTimeZones =
        std::unordered_map<std::string, std::vector<MarketCode>>();
      for(auto& market : marketDatabase.GetEntries()) {
        marketTimeZones[market.m_timeZone].push_back(market.m_code);
      }
      for(auto& marketTimeZone : marketTimeZones) {
        auto marketStartDate = MarketDateToUtc(marketTimeZone.second.front(),
          startTime, marketDatabase, timeZoneDatabase);
        auto marketEndDate = MarketDateToUtc(marketTimeZone.second.front(),
          endTime, marketDatabase, timeZoneDatabase) +
          boost::gregorian::days(1);
        auto marketExpressions = std::vector<Beam::Queries::Expression>();
        for(auto& market : marketTimeZone.second) {
          marketExpressions.push_back(BuildMarketFilter(market));
        }
        auto marketFilter = Beam::Queries::MakeOrExpression(
          marketExpressions.begin(), marketExpressions.end());
        auto dailyOrderSubmissionQuery = AccountQuery();
        dailyOrderSubmissionQuery.SetIndex(account);
        dailyOrderSubmissionQuery.SetRange(marketStartDate, marketEndDate);
        dailyOrderSubmissionQuery.SetFilter(marketFilter);
        dailyOrderSubmissionQuery.SetSnapshotLimit(
          Beam::Queries::SnapshotLimit::Unlimited());
        auto snapshotQueue = std::make_shared<Beam::Queue<SequencedOrder>>();
        orderExecutionClient.QueryOrderSubmissions(dailyOrderSubmissionQuery,
          snapshotQueue);
        try {
          while(true) {
            auto value = snapshotQueue->Pop();
            if(value.GetValue()->GetInfo().m_timestamp < marketEndDate) {
              queue.Push(std::move(value.GetValue()));
            }
          }
        } catch(const std::exception&) {}
      }
    });
  }

  /**
   * Builds a query to retrieve Orders by their id.
   * @param account The account to query.
   * @param ids The order ids to query.
   */
  inline AccountQuery BuildOrderSubmissionQuery(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<OrderId>& ids) {
    auto info = Beam::Queries::ParameterExpression(0,
      Nexus::Queries::OrderInfoType());
    auto field = Beam::Queries::MemberAccessExpression("order_id",
      Beam::Queries::NativeDataType<OrderId>(), info);
    auto clauses = std::vector<Beam::Queries::Expression>();
    std::transform(ids.begin(), ids.end(), std::back_inserter(clauses),
      [&] (auto& id) {
        return Beam::Queries::MakeEqualsExpression(field,
          Beam::Queries::ConstantExpression(Beam::Queries::NativeValue(id)));
      });
    auto query = AccountQuery();
    query.SetIndex(account);
    query.SetRange(Beam::Queries::Range::Historical());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    query.SetFilter(Beam::Queries::MakeOrExpression(
      clauses.begin(), clauses.end()));
    return query;
  }

  /**
   * Builds a query to retrieve Orders by their id.
   * @param account The account to query.
   * @param ids The order ids to query.
   * @param orderExecutionClient The OrderExecutionClient to query.
   * @param queue The Queue to write to.
   */
  template<typename OrderExecutionClient>
  void QueryOrderSubmissions(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<OrderId>& ids,
      OrderExecutionClient& orderExecutionClient,
      Beam::ScopedQueueWriter<const Order*> queue) {
    auto query = BuildOrderSubmissionQuery(account, ids);
    orderExecutionClient.QueryOrderSubmissions(query, std::move(queue));
  }

  /**
   * Loads a list of Orders by id.
   * @param account The account to query.
   * @param ids The order ids to query.
   * @param orderExecutionClient The OrderExecutionClient to query.
   */
  template<typename OrderExecutionClient>
  std::vector<const Order*> LoadOrderSubmissions(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<OrderId>& ids,
      OrderExecutionClient& orderExecutionClient) {
    auto queue = std::make_shared<Beam::Queue<const Order*>>();
    QueryOrderSubmissions(account, ids, orderExecutionClient, queue);
    auto orders = std::vector<const Order*>();
    Beam::Flush(queue, std::back_inserter(orders));
    return orders;
  }
}

#endif
