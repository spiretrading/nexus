#ifndef NEXUS_OPPOSING_ORDER_CANCELLATION_COMPLIANCE_RULE_HPP
#define NEXUS_OPPOSING_ORDER_CANCELLATION_COMPLIANCE_RULE_HPP
#include <type_traits>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/TaggedQueueReader.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"

namespace Nexus {

  /**
   * Prevents Orders on the opposite side of a fill from being canceled for a
   * certain period of time.
   * @param C The type of TimeClient used to determine how much time has elapsed
   *        since the last fill.
   */
  template<typename C>
  class OpposingOrderCancellationComplianceRule : public ComplianceRule {
    public:

      /**
       * The type of TimeClient used to determine how much time has elapsed
       * since the last fill.
       */
      using TimeClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs an OpposingOrderCancellationComplianceRule.
       * @param timeout The amount of time to restrict cancels after a fill.
       * @param time_client Initializes the TimeClient.
       */
      template<Beam::Initializes<C> CF>
      OpposingOrderCancellationComplianceRule(
        boost::posix_time::time_duration timeout, CF&& time_client);

      void cancel(const std::shared_ptr<Order>& order) override;
      void add(const std::shared_ptr<Order>& order) override;

    private:
      boost::posix_time::time_duration m_timeout;
      Beam::GetOptionalLocalPtr<C> m_time_client;
      Beam::TaggedQueueReader<Side, ExecutionReport> m_reports;
      boost::posix_time::ptime m_last_ask_fill_time;
      boost::posix_time::ptime m_last_bid_fill_time;
  };

  template<typename TimeClient>
  OpposingOrderCancellationComplianceRule(
    boost::posix_time::time_duration, TimeClient&&) ->
      OpposingOrderCancellationComplianceRule<
        std::remove_reference_t<TimeClient>>;

  /**
   * Returns a ComplianceRuleSchema representing an
   * OpposingOrderCancellationComplianceRule.
   */
  inline ComplianceRuleSchema
      make_opposing_order_cancellation_compliance_rule_schema() {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("timeout", Quantity(0));
    auto schema =
      ComplianceRuleSchema("opposing_order_cancellation", parameters);
    return schema;
  }

  template<typename C>
  template<Beam::Initializes<C> CF>
  OpposingOrderCancellationComplianceRule<C>::
      OpposingOrderCancellationComplianceRule(
        boost::posix_time::time_duration timeout, CF&& time_client)
    : m_timeout(timeout),
      m_time_client(std::forward<CF>(time_client)),
      m_last_ask_fill_time(boost::posix_time::not_a_date_time),
      m_last_bid_fill_time(boost::posix_time::not_a_date_time) {}

  template<typename C>
  void OpposingOrderCancellationComplianceRule<C>::cancel(
      const std::shared_ptr<Order>& order) {
    while(auto report = m_reports.TryPop()) {
      if(report->m_value.m_last_quantity != 0) {
        auto& last_fill_time =
          pick(report->m_key, m_last_ask_fill_time, m_last_bid_fill_time);
        last_fill_time = report->m_value.m_timestamp;
      }
    }
    auto time = m_time_client->GetTime();
    auto& last_fill_time = pick(order->get_info().m_fields.m_side,
      m_last_bid_fill_time, m_last_ask_fill_time);
    if(last_fill_time != boost::posix_time::not_a_date_time &&
        last_fill_time >= (time - m_timeout)) {
      BOOST_THROW_EXCEPTION(
        ComplianceCheckException("Opposing order can not be canceled yet."));
    }
  }

  template<typename C>
  void OpposingOrderCancellationComplianceRule<C>::add(
      const std::shared_ptr<Order>& order) {
    order->get_publisher().Monitor(
      m_reports.GetSlot(order->get_info().m_fields.m_side));
  }
}

#endif
