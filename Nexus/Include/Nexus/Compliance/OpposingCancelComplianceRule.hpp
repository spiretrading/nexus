#ifndef NEXUS_OPPOSING_CANCEL_COMPLIANCE_RULE_HPP
#define NEXUS_OPPOSING_CANCEL_COMPLIANCE_RULE_HPP
#include <type_traits>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/TaggedQueueReader.hpp>
#include <Beam/TimeService/TimeClient.hpp>
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
  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  class OpposingCancelComplianceRule : public ComplianceRule {
    public:

      /**
       * The type of TimeClient used to determine how much time has elapsed
       * since the last fill.
       */
      using TimeClient = Beam::dereference_t<C>;

      /**
       * Constructs an OpposingCancelComplianceRule.
       * @param timeout The amount of time to restrict cancels after a fill.
       * @param time_client Initializes the TimeClient.
       */
      template<Beam::Initializes<C> CF>
      OpposingCancelComplianceRule(
        boost::posix_time::time_duration timeout, CF&& time_client);

      void cancel(const std::shared_ptr<Order>& order) override;
      void add(const std::shared_ptr<Order>& order) override;

    private:
      boost::posix_time::time_duration m_timeout;
      Beam::local_ptr_t<C> m_time_client;
      Beam::TaggedQueueReader<Side, ExecutionReport> m_reports;
      boost::posix_time::ptime m_last_ask_fill_time;
      boost::posix_time::ptime m_last_bid_fill_time;
  };

  template<typename C>
  OpposingCancelComplianceRule(
    boost::posix_time::time_duration, C&&) ->
      OpposingCancelComplianceRule<std::remove_cvref_t<C>>;

  /**
   * The standard name used to identify the OpposingCancelComplianceRule.
   */
  inline auto OPPOSING_CANCEL_RULE_NAME = std::string("opposing_cancel");

  /**
   * Returns a ComplianceRuleSchema representing an
   * OpposingCancelComplianceRule.
   */
  inline ComplianceRuleSchema make_opposing_cancel_compliance_rule_schema() {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("timeout", Quantity(0));
    return ComplianceRuleSchema(
      OPPOSING_CANCEL_RULE_NAME, std::move(parameters));
  }

  /**
   * Makes a new OpposingCancelComplianceRule from a list of
   * ComplianceParameters.
   * @param parameters The parameters to construct the rule from.
   * @param time_client Initializes the TimeClient.
   */
  inline auto make_opposing_cancel_compliance_rule(
      const std::vector<ComplianceParameter>& parameters, auto& time_client) {
    auto timeout =
      boost::posix_time::time_duration(boost::posix_time::seconds(0));
    for(auto& parameter : parameters) {
      if(parameter.m_name == "timeout") {
        timeout = boost::posix_time::seconds(
          static_cast<int>(boost::get<Quantity>(parameter.m_value)));
      }
    }
    using Rule = OpposingCancelComplianceRule<
      std::remove_reference_t<decltype(time_client)>*>;
    return std::make_unique<Rule>(timeout, &time_client);
  }

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  OpposingCancelComplianceRule<C>::OpposingCancelComplianceRule(
        boost::posix_time::time_duration timeout, CF&& time_client)
    : m_timeout(timeout),
      m_time_client(std::forward<CF>(time_client)),
      m_last_ask_fill_time(boost::posix_time::not_a_date_time),
      m_last_bid_fill_time(boost::posix_time::not_a_date_time) {}

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  void OpposingCancelComplianceRule<C>::cancel(
      const std::shared_ptr<Order>& order) {
    while(auto report = m_reports.try_pop()) {
      if(report->m_value.m_last_quantity != 0) {
        auto& last_fill_time =
          pick(report->m_key, m_last_ask_fill_time, m_last_bid_fill_time);
        last_fill_time = report->m_value.m_timestamp;
      }
    }
    auto time = m_time_client->get_time();
    auto& last_fill_time = pick(order->get_info().m_fields.m_side,
      m_last_bid_fill_time, m_last_ask_fill_time);
    if(last_fill_time != boost::posix_time::not_a_date_time &&
        last_fill_time >= (time - m_timeout)) {
      boost::throw_with_location(
        ComplianceCheckException("Opposing order can not be canceled yet."));
    }
  }

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  void OpposingCancelComplianceRule<C>::add(
      const std::shared_ptr<Order>& order) {
    order->get_publisher().monitor(
      m_reports.get_slot(order->get_info().m_fields.m_side));
  }
}

#endif
