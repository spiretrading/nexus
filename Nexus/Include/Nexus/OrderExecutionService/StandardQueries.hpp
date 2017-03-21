#ifndef NEXUS_ORDEREXECUTIONSERVICESTANDARDQUERIES_HPP
#define NEXUS_ORDEREXECUTIONSERVICESTANDARDQUERIES_HPP
#include <unordered_map>
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

namespace Nexus {
namespace OrderExecutionService {

  //! Builds Query Expression to filter Orders by MarketCode.
  /*!
    \param market The market to query.
    \return A Query Expression that filters any Order not submitted to the
            specified <i>market</i>.
  */
  inline auto BuildMarketFilter(MarketCode market) {
    Beam::Queries::StringValue queryMarketCode{market.GetData()};
    Beam::Queries::ConstantExpression marketCodeExpression{queryMarketCode};
    Beam::Queries::ParameterExpression infoParameterExpression{
      0, Nexus::Queries::OrderInfoType()};
    Beam::Queries::MemberAccessExpression fieldsAccessExpression{"fields",
      Nexus::Queries::OrderFieldsType(), infoParameterExpression};
    Beam::Queries::MemberAccessExpression securityAccessExpression{"security",
      Nexus::Queries::SecurityType(), fieldsAccessExpression};
    Beam::Queries::MemberAccessExpression marketAccessExpression{"market",
      Beam::Queries::StringType(), securityAccessExpression};
    auto equalExpression = Beam::Queries::MakeEqualsExpression(
      marketCodeExpression, marketAccessExpression);
    return equalExpression;
  }

  //! Builds a query to retrieve Orders submitted to a specified market on
  //! on a daily basis.
  /*!
    \param market The market to query.
    \param account The account to query.
    \param startTime The first day to retrieve order submissions for.
    \param endTime The last day to retrieve order submissions for.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \return An AccountQuery that can be used to retrieve the daily Order
            submissions for the specified <i>account</i> on the specified
            <i>market</i>.
  */
  inline AccountQuery BuildDailyOrderSubmissionQuery(MarketCode market,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const boost::posix_time::ptime& startTime,
      const boost::posix_time::ptime& endTime,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto marketStartOfDay = MarketDateToUtc(market, startTime, marketDatabase,
      timeZoneDatabase);
    boost::posix_time::ptime marketEndOfDay;
    if(endTime == boost::posix_time::pos_infin) {
      marketEndOfDay = boost::posix_time::pos_infin;
    } else {
      marketEndOfDay = MarketDateToUtc(market, endTime, marketDatabase,
        timeZoneDatabase) + boost::gregorian::days(1);
    }
    auto marketFilter = BuildMarketFilter(market);
    AccountQuery dailyOrderSubmissionQuery;
    dailyOrderSubmissionQuery.SetIndex(account);
    dailyOrderSubmissionQuery.SetRange(marketStartOfDay, marketEndOfDay);
    dailyOrderSubmissionQuery.SetFilter(marketFilter);
    dailyOrderSubmissionQuery.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Unlimited());
    return dailyOrderSubmissionQuery;
  }

  //! Builds a query to retrieve Orders submitted to a specified market on
  //! on a daily basis.
  /*!
    \param account The account to query.
    \param startTime The first day to retrieve order submissions for.
    \param endTime The last day to retrieve order submissions for.
    \param marketDatabase The database containing Market info.
    \param timeZoneDatabase The database of timezones.
    \param orderExecutionClient The OrderExecutionClient to query.
    \param queue The Queue to write to.
  */
  template<typename OrderExecutionClient>
  void QueryDailyOrderSubmissions(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const boost::posix_time::ptime& startTime,
      const boost::posix_time::ptime& endTime,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      OrderExecutionClient& orderExecutionClient,
      const std::shared_ptr<Beam::QueueWriter<const Order*>>& queue) {
    Beam::Routines::Spawn(
      [=, &orderExecutionClient] {
        std::unordered_map<std::string, std::vector<MarketCode>>
          marketTimeZones;
        for(auto& market : marketDatabase.GetEntries()) {
          marketTimeZones[market.m_timeZone].push_back(market.m_code);
        }
        for(auto& marketTimeZone : marketTimeZones) {
          auto marketStartDate = MarketDateToUtc(marketTimeZone.second.front(),
            startTime, marketDatabase, timeZoneDatabase);
          auto marketEndDate = MarketDateToUtc(marketTimeZone.second.front(),
            endTime, marketDatabase, timeZoneDatabase) +
            boost::gregorian::days(1);
          std::vector<Beam::Queries::Expression> marketExpressions;
          for(auto& market : marketTimeZone.second) {
            auto marketFilter = BuildMarketFilter(market);
            marketExpressions.push_back(marketFilter);
          }
          auto marketFilter = Beam::Queries::MakeOrExpression(
            marketExpressions.begin(), marketExpressions.end());
          AccountQuery dailyOrderSubmissionQuery;
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
              auto value = snapshotQueue->Top();
              snapshotQueue->Pop();
              if(value.GetValue()->GetInfo().m_timestamp < marketEndDate) {
                queue->Push(std::move(value.GetValue()));
              }
            }
          } catch(const std::exception&) {}
        }
        queue->Break();
      });
  }
}
}

#endif
