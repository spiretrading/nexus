#ifndef NEXUS_OPPOSINGORDERSUBMISSIONCOMPLIANCERULE_HPP
#define NEXUS_OPPOSINGORDERSUBMISSIONCOMPLIANCERULE_HPP
#include <vector>
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
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class OpposingOrderSubmissionComplianceRule
      \brief Prevents opposing orders from being submitted after a cancellation.
      \tparam TimeClientType The type of TimeClient used to determine how much
              time has elapsed since the last cancellation.
   */
  template<typename TimeClientType>
  class OpposingOrderSubmissionComplianceRule : public ComplianceRule {
    public:

      //! The type of TimeClient used to determine how much time has elapsed
      //! since the last cancellation.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! Constructs an OpposingOrderSubmissionComplianceRule.
      /*!
        \param timeout The amount of time to restrict submissions after a
               cancel.
        \param offset The offset from the submission price to restrict
               submissions.
        \param timeClient Initializes the TimeClient.
      */
      template<typename TimeClientForward>
      OpposingOrderSubmissionComplianceRule(
        boost::posix_time::time_duration timeout, Money offset,
        TimeClientForward&& timeClient);

      virtual void Add(const OrderExecutionService::Order& order);

      virtual void Submit(const OrderExecutionService::Order& order);

    private:
      boost::posix_time::time_duration m_timeout;
      Money m_offset;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
      Beam::TaggedQueue<const OrderExecutionService::Order*,
        OrderExecutionService::ExecutionReport> m_executionReportQueue;
      boost::posix_time::ptime m_lastAskCancelTime;
      Money m_askPrice;
      boost::posix_time::ptime m_lastBidCancelTime;
      Money m_bidPrice;

      Money GetSubmissionPrice(const OrderExecutionService::Order& order);
      bool TestSubmissionPriceInRange(
        const OrderExecutionService::Order& order);
  };

  //! Builds a ComplianceRuleSchema representing an
  //! OpposingOrderSubmissionComplianceRule.
  inline ComplianceRuleSchema
      BuildOpposingOrderSubmissionComplianceRuleSchema() {
    std::vector<ComplianceParameter> parameters;
    std::vector<ComplianceValue> symbols;
    symbols.push_back(Security{});
    parameters.emplace_back("symbols", symbols);
    parameters.emplace_back("start_period", boost::posix_time::time_duration{});
    parameters.emplace_back("end_period", boost::posix_time::time_duration{});
    parameters.emplace_back("timeout", Quantity{0});
    parameters.emplace_back("offset", Money::ZERO);
    ComplianceRuleSchema schema{"opposing_order_submission", parameters};
    return schema;
  }

  //! Builds an OpposingOrderSubmissionComplianceRule from a list of
  //! ComplianceParameters.
  /*!
    \param parameters The list of ComplianceParameters used to build the rule.
    \param timeClient Initializes the TimeClient.
  */
  template<typename TimeClient>
  std::unique_ptr<ComplianceRule> MakeOpposingOrderSubmissionComplianceRule(
      const std::vector<ComplianceParameter>& parameters,
      TimeClient&& timeClient) {
    SecuritySet symbols;
    boost::posix_time::time_duration startPeriod;
    boost::posix_time::time_duration endPeriod;
    boost::posix_time::time_duration timeout;
    Money offset;
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
      } else if(parameter.m_name == "offset") {
        offset = boost::get<Money>(parameter.m_value);
      }
    }
    auto rule = std::make_unique<OpposingOrderSubmissionComplianceRule<
      std::decay_t<TimeClient>>>(timeout, offset, timeClient);
    auto timeFilter = std::make_unique<TimeFilterComplianceRule<
      std::decay_t<TimeClient>>>(startPeriod, endPeriod, timeClient,
      std::move(rule));
    auto symbolFilter = std::make_unique<SecurityFilterComplianceRule>(
      std::move(symbols), std::move(timeFilter));
    return std::move(symbolFilter);
  }

  template<typename TimeClientType>
  template<typename TimeClientForward>
  OpposingOrderSubmissionComplianceRule<TimeClientType>::
      OpposingOrderSubmissionComplianceRule(
      boost::posix_time::time_duration timeout, Money offset,
      TimeClientForward&& timeClient)
      : m_timeout{timeout},
        m_offset{offset},
        m_timeClient{std::forward<TimeClientForward>(timeClient)} {}

  template<typename TimeClientType>
  void OpposingOrderSubmissionComplianceRule<TimeClientType>::Add(
      const OrderExecutionService::Order& order) {
    order.GetPublisher().Monitor(m_executionReportQueue.GetSlot(&order));
  }

  template<typename TimeClientType>
  void OpposingOrderSubmissionComplianceRule<TimeClientType>::Submit(
      const OrderExecutionService::Order& order) {
    if(order.GetInfo().m_fields.m_type != OrderType::LIMIT &&
        order.GetInfo().m_fields.m_type != OrderType::MARKET) {
      return;
    }
    while(!m_executionReportQueue.IsEmpty()) {
      auto executionReport = m_executionReportQueue.Top();
      m_executionReportQueue.Pop();
      if(executionReport.m_value.m_status == OrderStatus::CANCELED) {
        auto side = executionReport.m_key->GetInfo().m_fields.m_side;
        auto submissionPrice = GetSubmissionPrice(order);
        if(side == Side::ASK) {
          if(submissionPrice >= submissionPrice) {
            m_askPrice = submissionPrice;
            m_lastAskCancelTime = executionReport.m_value.m_timestamp;
          }
        } else {
          if(submissionPrice <= submissionPrice) {
            m_bidPrice = submissionPrice;
            m_lastBidCancelTime = executionReport.m_value.m_timestamp;
          }
        }
      }
    }
    auto time = m_timeClient->GetTime();
    auto& lastCancelTime = Pick(order.GetInfo().m_fields.m_side,
      m_lastBidCancelTime, m_lastAskCancelTime);
    if(TestSubmissionPriceInRange(order) &&
        lastCancelTime != boost::posix_time::not_a_date_time &&
        lastCancelTime >= (time - m_timeout)) {
      BOOST_THROW_EXCEPTION(ComplianceCheckException{
        "Opposing order can not be submitted yet."});
    }
  }

  template<typename TimeClientType>
  Money OpposingOrderSubmissionComplianceRule<TimeClientType>::
      GetSubmissionPrice(const OrderExecutionService::Order& order) {
    if(order.GetInfo().m_fields.m_type == OrderType::LIMIT) {
      return order.GetInfo().m_fields.m_price;
    } else if(order.GetInfo().m_fields.m_side == Side::ASK) {
      return Money::ZERO;
    } else {
      return Money::FromRepresentation(
        std::numeric_limits<std::int64_t>::max());
    }
  }

  template<typename TimeClientType>
  bool OpposingOrderSubmissionComplianceRule<TimeClientType>::
      TestSubmissionPriceInRange(const OrderExecutionService::Order& order) {
    auto price = GetSubmissionPrice(order);
    if(order.GetInfo().m_fields.m_side == Side::ASK) {
      return price + m_offset <= m_askPrice;
    } else {
      return price - m_offset >= m_bidPrice;
    }
  }
}
}

#endif
