#ifndef NEXUS_OPPOSING_SUBMISSION_COMPLIANCE_RULE_HPP
#define NEXUS_OPPOSING_SUBMISSION_COMPLIANCE_RULE_HPP
#include <limits>
#include <vector>
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
   * Prevents opposing orders from being submitted after a cancellation.
   * @param <C> The type of TimeClient used to determine how much time has
   *        elapsed since the last cancellation.
   */
  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  class OpposingSubmissionComplianceRule : public ComplianceRule {
    public:

      /**
       * The type of TimeClient used to determine how much time has elapsed
       * since the last cancellation.
       */
      using TimeClient = Beam::dereference_t<C>;

      /**
       * Constructs an OpposingSubmissionComplianceRule.
       * @param timeout The amount of time to restrict submissions after a
       *        cancel.
       * @param offset The offset from the submission price to restrict
       *        submissions.
       * @param time_client Initializes the TimeClient.
       */
      template<Beam::Initializes<C> CF>
      OpposingSubmissionComplianceRule(boost::posix_time::time_duration timeout,
        Money offset, CF&& time_client);

      void submit(const std::shared_ptr<Order>& order) override;
      void add(const std::shared_ptr<Order>& order) override;

    private:
      boost::posix_time::time_duration m_timeout;
      Money m_offset;
      Beam::local_ptr_t<C> m_time_client;
      Beam::TaggedQueueReader<OrderFields, ExecutionReport> m_reports;
      boost::posix_time::ptime m_last_ask_cancel_time;
      Money m_ask_price;
      boost::posix_time::ptime m_last_bid_cancel_time;
      Money m_bid_price;

      Money get_submission_price(const OrderFields& fields) const;
      bool test_price_in_range(const OrderFields& fields) const;
  };

  template<typename TimeClient>
  OpposingSubmissionComplianceRule(
    boost::posix_time::time_duration, Money, TimeClient&&) ->
      OpposingSubmissionComplianceRule<std::remove_cvref_t<TimeClient>>;

  /**
   * The standard name used to identify the
   * OpposingSubmissionComplianceRule.
   */
  inline auto OPPOSING_SUBMISSION_RULE_NAME =
    std::string("opposing_submission");

  /**
   * Returns a ComplianceRuleSchema representing an
   * OpposingSubmissionComplianceRule.
   */
  inline ComplianceRuleSchema
      make_opposing_submission_compliance_rule_schema() {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("timeout", Quantity(0));
    parameters.emplace_back("offset", Money::ZERO);
    return ComplianceRuleSchema(
      OPPOSING_SUBMISSION_RULE_NAME, std::move(parameters));
  }

  /**
   * Makes a new OpposingSubmissionComplianceRule from a list of
   * ComplianceParameters.
   * @param parameters The parameters to construct the rule from.
   * @param time_client Initializes the TimeClient.
   */
  inline auto make_opposing_submission_compliance_rule(
      const std::vector<ComplianceParameter>& parameters, auto& time_client) {
    auto timeout =
      boost::posix_time::time_duration(boost::posix_time::seconds(0));
    auto offset = Money::ZERO;
    for(auto& parameter : parameters) {
      if(parameter.m_name == "timeout") {
        timeout = boost::posix_time::seconds(
          static_cast<int>(boost::get<Quantity>(parameter.m_value)));
      } else if(parameter.m_name == "offset") {
        offset = boost::get<Money>(parameter.m_value);
      }
    }
    using Rule = OpposingSubmissionComplianceRule<
      std::remove_cvref_t<decltype(time_client)>*>;
    return std::make_unique<Rule>(timeout, offset, &time_client);
  }

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  OpposingSubmissionComplianceRule<C>::OpposingSubmissionComplianceRule(
    boost::posix_time::time_duration timeout, Money offset, CF&& time_client)
    : m_timeout(timeout),
      m_offset(offset),
      m_time_client(std::forward<CF>(time_client)),
      m_last_ask_cancel_time(boost::posix_time::min_date_time),
      m_ask_price(std::numeric_limits<Money>::max()),
      m_last_bid_cancel_time(boost::posix_time::min_date_time),
      m_bid_price(Money::ZERO) {}

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  void OpposingSubmissionComplianceRule<C>::submit(
      const std::shared_ptr<Order>& order) {
    if(order->get_info().m_fields.m_type != OrderType::LIMIT &&
        order->get_info().m_fields.m_type != OrderType::MARKET) {
      return;
    }
    auto time = m_time_client->get_time();
    while(auto report = m_reports.try_pop()) {
      if(report->m_value.m_status == OrderStatus::CANCELED) {
        auto side = report->m_key.m_side;
        auto price = get_submission_price(report->m_key);
        if(side == Side::ASK) {
          if((time - m_last_ask_cancel_time) > m_timeout) {
            m_ask_price = std::numeric_limits<Money>::max();
          }
          if(report->m_value.m_timestamp >= m_last_ask_cancel_time) {
            if(price <= m_ask_price) {
              m_last_ask_cancel_time = report->m_value.m_timestamp;
              m_ask_price = price;
            }
          }
        } else {
          if((time - m_last_bid_cancel_time) > m_timeout) {
            m_bid_price = Money::ZERO;
          }
          if(report->m_value.m_timestamp >= m_last_bid_cancel_time) {
            if(price >= m_bid_price) {
              m_last_bid_cancel_time = report->m_value.m_timestamp;
              m_bid_price = price;
            }
          }
        }
      }
    }
    auto& cancel_time = pick(order->get_info().m_fields.m_side,
      m_last_bid_cancel_time, m_last_ask_cancel_time);
    if(test_price_in_range(order->get_info().m_fields) &&
        cancel_time >= (time - m_timeout)) {
      boost::throw_with_location(
        ComplianceCheckException("Opposing order can not be submitted yet."));
    }
    order->get_publisher().monitor(
      m_reports.get_slot(order->get_info().m_fields));
  }

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  void OpposingSubmissionComplianceRule<C>::add(
      const std::shared_ptr<Order>& order) {
    if(order->get_info().m_fields.m_type != OrderType::LIMIT &&
        order->get_info().m_fields.m_type != OrderType::MARKET) {
      return;
    }
    order->get_publisher().monitor(
      m_reports.get_slot(order->get_info().m_fields));
  }

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  Money OpposingSubmissionComplianceRule<C>::get_submission_price(
      const OrderFields& fields) const {
    if(fields.m_type == OrderType::LIMIT) {
      return fields.m_price;
    } else if(fields.m_side == Side::ASK) {
      return Money::ZERO;
    } else {
      return std::numeric_limits<Money>::max();
    }
  }

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  bool OpposingSubmissionComplianceRule<C>::test_price_in_range(
      const OrderFields& fields) const {
    auto price = get_submission_price(fields);
    if(fields.m_side == Side::ASK) {
      return price <= m_bid_price + m_offset;
    } else {
      return price >= m_ask_price - m_offset;
    }
  }
}

#endif
