#ifndef NEXUS_OPPOSING_ORDER_SUBMISSION_COMPLIANCE_RULE_HPP
#define NEXUS_OPPOSING_ORDER_SUBMISSION_COMPLIANCE_RULE_HPP
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/TaggedQueueReader.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"

namespace Nexus {

  /**
   * Prevents opposing orders from being submitted after a cancellation.
   * @param <C> The type of TimeClient used to determine how much time has
   *        elapsed since the last cancellation.
   */
  template<typename C>
  class OpposingOrderSubmissionComplianceRule : public ComplianceRule {
    public:

      /**
       * The type of TimeClient used to determine how much time has elapsed
       * since the last cancellation.
       */
      using TimeClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs an OpposingOrderSubmissionComplianceRule.
       * @param timeout The amount of time to restrict submissions after a
       *        cancel.
       * @param offset The offset from the submission price to restrict
       *        submissions.
       * @param time_client Initializes the TimeClient.
       */
      template<Beam::Initializes<C> CF>
      OpposingOrderSubmissionComplianceRule(
        boost::posix_time::time_duration timeout, Money offset,
        CF&& time_client);

      void submit(const std::shared_ptr<const Order>& order) override;
      void add(const std::shared_ptr<const Order>& order) override;

    private:
      boost::posix_time::time_duration m_timeout;
      Money m_offset;
      Beam::GetOptionalLocalPtr<C> m_time_client;
      Beam::TaggedQueueReader<OrderFields, ExecutionReport> m_reports;
      boost::posix_time::ptime m_last_ask_cancel_time;
      Money m_ask_price;
      boost::posix_time::ptime m_last_bid_cancel_time;
      Money m_bid_price;

      Money get_submission_price(const OrderFields& fields) const;
      bool test_price_in_range(const OrderFields& fields) const;
  };

  template<typename TimeClient>
  OpposingOrderSubmissionComplianceRule(
    boost::posix_time::time_duration, Money, TimeClient&&) ->
      OpposingOrderSubmissionComplianceRule<
        std::remove_reference_t<TimeClient>>;

  template<typename C>
  template<Beam::Initializes<C> CF>
  OpposingOrderSubmissionComplianceRule<C>::
    OpposingOrderSubmissionComplianceRule(
      boost::posix_time::time_duration timeout, Money offset, CF&& time_client)
    : m_timeout(timeout),
      m_offset(offset),
      m_time_client(std::forward<CF>(time_client)),
      m_last_ask_cancel_time(boost::posix_time::min_date_time),
      m_ask_price(std::numeric_limits<Money>::max()),
      m_last_bid_cancel_time(boost::posix_time::min_date_time),
      m_bid_price(Money::ZERO) {}

  template<typename C>
  void OpposingOrderSubmissionComplianceRule<C>::submit(
      const std::shared_ptr<const Order>& order) {
    if(order->get_info().m_fields.m_type != OrderType::LIMIT &&
        order->get_info().m_fields.m_type != OrderType::MARKET) {
      return;
    }
    auto time = m_time_client->GetTime();
    while(auto report = m_reports.TryPop()) {
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
      BOOST_THROW_EXCEPTION(
        ComplianceCheckException("Opposing order can not be submitted yet."));
    }
    order->get_publisher().Monitor(
      m_reports.GetSlot(order->get_info().m_fields));
  }

  template<typename C>
  void OpposingOrderSubmissionComplianceRule<C>::add(
      const std::shared_ptr<const Order>& order) {
    if(order->get_info().m_fields.m_type != OrderType::LIMIT &&
        order->get_info().m_fields.m_type != OrderType::MARKET) {
      return;
    }
    order->get_publisher().Monitor(
      m_reports.GetSlot(order->get_info().m_fields));
  }

  template<typename C>
  Money OpposingOrderSubmissionComplianceRule<C>::get_submission_price(
      const OrderFields& fields) const {
    if(fields.m_type == OrderType::LIMIT) {
      return fields.m_price;
    } else if(fields.m_side == Side::ASK) {
      return Money::ZERO;
    } else {
      return std::numeric_limits<Money>::max();
    }
  }

  template<typename C>
  bool OpposingOrderSubmissionComplianceRule<C>::test_price_in_range(
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
