#ifndef NEXUS_OPPOSINGORDERCANCELLATIONCOMPLIANCERULE_HPP
#define NEXUS_OPPOSINGORDERCANCELLATIONCOMPLIANCERULE_HPP
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/TaggedQueue.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Compliance/SecurityFilterComplianceRule.hpp"
#include "Nexus/Compliance/TimeFilterComplianceRule.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class OpposingOrderCancellationComplianceRule
      \brief Prevents Orders on the opposite side of a fill from being canceled
             for a certain period of time.
      \tparam TimeClientType The type of TimeClient used to determine how much
              time has elapsed since the last fill.
   */
  template<typename TimeClientType>
  class OpposingOrderCancellationComplianceRule : public ComplianceRule {
    public:

      //! The type of TimeClient used to determine how much time has elapsed
      //! since the last fill.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! Constructs an OpposingOrderCancellationComplianceRule.
      /*!
        \param timeout The amount of time to restrict cancels after a fill.
        \param timeClient Initializes the TimeClient.
      */
      template<typename TimeClientForward>
      OpposingOrderCancellationComplianceRule(
        boost::posix_time::time_duration timeout,
        TimeClientForward&& timeClient);

      virtual void Add(const OrderExecutionService::Order& order);

      virtual void Cancel(const OrderExecutionService::Order& order);

    private:
      boost::posix_time::time_duration m_timeout;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
      Beam::TaggedQueue<const OrderExecutionService::Order*,
        OrderExecutionService::ExecutionReport> m_executionReportQueue;
      boost::posix_time::ptime m_lastAskFillTime;
      boost::posix_time::ptime m_lastBidFillTime;
  };

  template<typename TimeClient>
  OpposingOrderCancellationComplianceRule(boost::posix_time::time_duration,
    TimeClient&& timeClient) -> OpposingOrderCancellationComplianceRule<
    std::decay_t<TimeClient>>;

  //! Builds a ComplianceRuleSchema representing an
  //! OpposingOrderCancellationComplianceRule.
  inline ComplianceRuleSchema
      BuildOpposingOrderCancellationComplianceRuleSchema() {
    std::vector<ComplianceParameter> parameters;
    std::vector<ComplianceValue> symbols;
    symbols.push_back(Security{});
    parameters.emplace_back("symbols", symbols);
    parameters.emplace_back("start_period", boost::posix_time::time_duration{});
    parameters.emplace_back("end_period", boost::posix_time::time_duration{});
    parameters.emplace_back("timeout", Quantity{0});
    ComplianceRuleSchema schema{"opposing_order_cancellation", parameters};
    return schema;
  }

  //! Builds an OpposingOrderCancellationComplianceRule from a list of
  //! ComplianceParameters.
  /*!
    \param parameters The list of ComplianceParameters used to build the rule.
    \param timeClient Initializes the TimeClient.
  */
  template<typename TimeClient>
  std::unique_ptr<ComplianceRule> MakeOpposingOrderCancellationComplianceRule(
      const std::vector<ComplianceParameter>& parameters,
      const TimeClient& timeClient) {
    SecuritySet symbols;
    boost::posix_time::time_duration startPeriod;
    boost::posix_time::time_duration endPeriod;
    boost::posix_time::time_duration timeout;
    for(auto& parameter : parameters) {
      if(parameter.m_name == "symbols") {
        for(auto& security : boost::get<std::vector<ComplianceValue>>(
            parameter.m_value)) {
          symbols.Add(std::move(boost::get<Security>(security)));
        }
      } else if(parameter.m_name == "start_period") {
        startPeriod = boost::get<boost::posix_time::time_duration>(
          parameter.m_value);
      } else if(parameter.m_name == "end_period") {
        endPeriod = boost::get<boost::posix_time::time_duration>(
          parameter.m_value);
      } else if(parameter.m_name == "timeout") {
        timeout = boost::posix_time::seconds(
          static_cast<int>(boost::get<Quantity>(parameter.m_value)));
      }
    }
    auto mapRule = MakeMapSecurityComplianceRule({},
      [=] (const ComplianceRuleSchema&) {
        return std::make_unique<OpposingOrderCancellationComplianceRule<
          std::decay_t<TimeClient>>>(timeout, timeClient);
      });
    auto timeFilter = std::make_unique<TimeFilterComplianceRule<
      std::decay_t<TimeClient>>>(startPeriod, endPeriod, timeClient,
      std::move(mapRule));
    auto symbolFilter = std::make_unique<SecurityFilterComplianceRule>(
      std::move(symbols), std::move(timeFilter));
    return std::move(symbolFilter);
  }

  template<typename TimeClientType>
  template<typename TimeClientForward>
  OpposingOrderCancellationComplianceRule<TimeClientType>::
      OpposingOrderCancellationComplianceRule(
      boost::posix_time::time_duration timeout, TimeClientForward&& timeClient)
      : m_timeout{timeout},
        m_timeClient{std::forward<TimeClientForward>(timeClient)},
        m_lastAskFillTime{boost::posix_time::not_a_date_time},
        m_lastBidFillTime{boost::posix_time::not_a_date_time} {}

  template<typename TimeClientType>
  void OpposingOrderCancellationComplianceRule<TimeClientType>::Add(
      const OrderExecutionService::Order& order) {
    order.GetPublisher().Monitor(m_executionReportQueue.GetSlot(&order));
  }

  template<typename TimeClientType>
  void OpposingOrderCancellationComplianceRule<TimeClientType>::Cancel(
      const OrderExecutionService::Order& order) {
    while(!m_executionReportQueue.IsEmpty()) {
      auto executionReport = m_executionReportQueue.Top();
      m_executionReportQueue.Pop();
      if(executionReport.m_value.m_lastQuantity != 0) {
        auto& lastFillTime = Pick(
          executionReport.m_key->GetInfo().m_fields.m_side, m_lastAskFillTime,
          m_lastBidFillTime);
        lastFillTime = executionReport.m_value.m_timestamp;
      }
    }
    auto time = m_timeClient->GetTime();
    auto& lastFillTime = Pick(order.GetInfo().m_fields.m_side,
      m_lastBidFillTime, m_lastAskFillTime);
    if(lastFillTime != boost::posix_time::not_a_date_time &&
        lastFillTime >= (time - m_timeout)) {
      BOOST_THROW_EXCEPTION(ComplianceCheckException{
        "Opposing order can not be canceled yet."});
    }
  }
}
}

#endif
