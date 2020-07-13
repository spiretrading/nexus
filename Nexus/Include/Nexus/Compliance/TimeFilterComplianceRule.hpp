#ifndef NEXUS_TIME_FILTER_COMPLIANCE_RULE_HPP
#define NEXUS_TIME_FILTER_COMPLIANCE_RULE_HPP
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"

namespace Nexus::Compliance {

  /**
   * Applies a ComplianceRule only during a specified time period.
   * @param <C> The type of TimeClient used to determine whether the
   *        ComplianceRule applies.
   */
  template<typename C>
  class TimeFilterComplianceRule : public ComplianceRule {
    public:

      /**
       * The type of TimeClient used to determine whether the ComplianceRule
       * applies.
       */
      using TimeClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a TimeFilterComplianceRule.
       * @param startPeriod The start of the period to apply the ComplianceRule.
       * @param endPeriod The end of the period to apply the ComplianceRule.
       * @param timeClient Initializes the TimeClient.
       * @param rule The ComplianceRule to apply.
       */
      template<typename CF>
      TimeFilterComplianceRule(boost::posix_time::time_duration startPeriod,
        boost::posix_time::time_duration endPeriod, CF&& timeClient,
        std::unique_ptr<ComplianceRule> rule);

      void Submit(const OrderExecutionService::Order& order) override;

      void Cancel(const OrderExecutionService::Order& order) override;

      void Add(const OrderExecutionService::Order& order) override;

    private:
      boost::posix_time::time_duration m_startPeriod;
      boost::posix_time::time_duration m_endPeriod;
      Beam::GetOptionalLocalPtr<C> m_timeClient;
      std::unique_ptr<ComplianceRule> m_rule;

      bool IsWithinTimePeriod();
  };

  template<typename TimeClient>
  TimeFilterComplianceRule(boost::posix_time::time_duration,
    boost::posix_time::time_duration, TimeClient&&,
    std::unique_ptr<ComplianceRule>) -> TimeFilterComplianceRule<
    std::decay_t<TimeClient>>;

  template<typename C>
  template<typename CF>
  TimeFilterComplianceRule<C>::TimeFilterComplianceRule(
    boost::posix_time::time_duration startPeriod,
    boost::posix_time::time_duration endPeriod, CF&& timeClient,
    std::unique_ptr<ComplianceRule> rule)
    : m_startPeriod(startPeriod),
      m_endPeriod(endPeriod),
      m_timeClient(std::forward<CF>(timeClient)),
      m_rule(std::move(rule)) {}

  template<typename C>
  void TimeFilterComplianceRule<C>::Submit(
      const OrderExecutionService::Order& order) {
    if(IsWithinTimePeriod()) {
      m_rule->Submit(order);
    } else {
      Add(order);
    }
  }

  template<typename C>
  void TimeFilterComplianceRule<C>::Cancel(
      const OrderExecutionService::Order& order) {
    if(IsWithinTimePeriod()) {
      m_rule->Cancel(order);
    }
  }

  template<typename C>
  void TimeFilterComplianceRule<C>::Add(
      const OrderExecutionService::Order& order) {
    m_rule->Add(order);
  }

  template<typename C>
  bool TimeFilterComplianceRule<C>::IsWithinTimePeriod() {
    auto time = m_timeClient->GetTime();
    if(m_startPeriod > m_endPeriod) {
      if(time.time_of_day() >= m_startPeriod ||
          time.time_of_day() <= m_endPeriod) {
        return true;
      }
    } else {
      if(time.time_of_day() >= m_startPeriod &&
          time.time_of_day() <= m_endPeriod) {
        return true;
      }
    }
    return false;
  }
}

#endif
