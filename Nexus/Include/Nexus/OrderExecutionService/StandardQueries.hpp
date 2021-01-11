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
   * Returns a Query Expression to filter Orders by MarketCode.
   * @param market The market to query.
   * @return A Query Expression that filters any Order not submitted to the
   *         specified <i>market</i>.
   */
  inline auto MakeMarketFilter(MarketCode market) {
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
   * Returns a query to retrieve Orders submitted to a specified market on
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
  inline AccountQuery MakeDailyOrderSubmissionQuery(MarketCode market,
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
    auto marketFilter = MakeMarketFilter(market);
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
      OrderExecutionClient&& orderExecutionClient,
      Beam::ScopedQueueWriter<const Order*> queue) {
    Beam::Routines::Spawn([=, queue = std::move(queue), orderExecutionClient =
        Beam::CapturePtr<OrderExecutionClient>(
          orderExecutionClient)] () mutable {
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
          marketExpressions.push_back(MakeMarketFilter(market));
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
        orderExecutionClient->QueryOrderSubmissions(dailyOrderSubmissionQuery,
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

  /** Returns a Query Expression to filter an account's live Orders. */
  inline auto MakeLiveOrdersFilter() {
    auto info = Beam::Queries::ParameterExpression(0,
      Nexus::Queries::OrderInfoType());
    return Beam::Queries::MemberAccessExpression("is_live",
      Beam::Queries::NativeDataType<bool>(), info);
  }

  /**
   * Returns a query to retrieve all of an account's live Orders.
   * @param account The account to query.
   */
  inline AccountQuery MakeLiveOrdersQuery(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto query = AccountQuery();
    query.SetIndex(account);
    query.SetRange(Beam::Queries::Range::Historical());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    query.SetFilter(MakeLiveOrdersFilter());
    return query;
  }

  /**
   * Queries an account's live Orders.
   * @param account The account to query.
   * @param orderExecutionClient The OrderExecutionClient to query.
   * @param queue The Queue to write to.
   */
  template<typename OrderExecutionClient>
  void QueryLiveOrders(const Beam::ServiceLocator::DirectoryEntry& account,
      OrderExecutionClient& orderExecutionClient,
      Beam::ScopedQueueWriter<const Order*> queue) {
    orderExecutionClient.QueryOrderSubmissions(MakeLiveOrdersQuery(account),
      std::move(queue));
  }

  /**
   * Loads an account's live Orders.
   * @param account The account to query.
   * @param orderExecutionClient The OrderExecutionClient to query.
   */
  template<typename OrderExecutionClient>
  std::vector<const Order*> LoadLiveOrders(
      const Beam::ServiceLocator::DirectoryEntry& account,
      OrderExecutionClient& orderExecutionClient) {
    auto queue = std::make_shared<Beam::Queue<const Order*>>();
    QueryLiveOrders(account, orderExecutionClient, queue);
    auto orders = std::vector<const Order*>();
    Beam::Flush(queue, std::back_inserter(orders));
    return orders;
  }

  /**
   * Returns a Query Expression to filter Orders by id.
   * @param ids The order ids to filter.
   */
  inline auto MakeOrderIdFilter(const std::vector<OrderId>& ids) {
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
    return Beam::Queries::MakeOrExpression(clauses.begin(), clauses.end());
  }

  /**
   * Returns a query to retrieve Orders by their id.
   * @param account The account to query.
   * @param ids The order ids to query.
   */
  inline AccountQuery MakeOrderIdQuery(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<OrderId>& ids) {
    auto query = AccountQuery();
    query.SetIndex(account);
    query.SetRange(Beam::Queries::Range::Historical());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    query.SetFilter(MakeOrderIdFilter(ids));
    return query;
  }

  /**
   * Queries for Orders by their id.
   * @param account The account to query.
   * @param ids The order ids to query.
   * @param orderExecutionClient The OrderExecutionClient to query.
   * @param queue The Queue to write to.
   */
  template<typename OrderExecutionClient>
  void QueryOrderIds(const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<OrderId>& ids,
      OrderExecutionClient& orderExecutionClient,
      Beam::ScopedQueueWriter<const Order*> queue) {
    orderExecutionClient.QueryOrderSubmissions(MakeOrderIdQuery(account, ids),
      std::move(queue));
  }

  /**
   * Loads a list of Orders by id.
   * @param account The account to query.
   * @param ids The order ids to query.
   * @param orderExecutionClient The OrderExecutionClient to query.
   */
  template<typename OrderExecutionClient>
  std::vector<const Order*> LoadOrderIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<OrderId>& ids,
      OrderExecutionClient& orderExecutionClient) {
    auto queue = std::make_shared<Beam::Queue<const Order*>>();
    QueryOrderIds(account, ids, orderExecutionClient, queue);
    auto orders = std::vector<const Order*>();
    Beam::Flush(queue, std::back_inserter(orders));
    return orders;
  }
}

#endif
