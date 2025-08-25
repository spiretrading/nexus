#ifndef NEXUS_TIME_FILTER_COMPLIANCE_RULE_HPP
#define NEXUS_TIME_FILTER_COMPLIANCE_RULE_HPP
#include <type_traits>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /**
   * Applies a ComplianceRule only during a specified time period relative the
   * venue an order is being submitted to or cancelled from.
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
       * @param start The beginning of the period to apply the rule.
       * @param end The end of the period to apply the rule.
       * @param time_zones The available time zones.
       * @param venues The venues available.
       * @param time_client Initializes the TimeClient used to check order
       *        cancel requests.
       * @param rule The rule to apply within the time period.
       */
      template<Beam::Initializes<C> CF>
      TimeFilterComplianceRule(boost::posix_time::time_duration start,
        boost::posix_time::time_duration end,
        boost::local_time::tz_database time_zones, VenueDatabase venues,
        CF&& time_client, std::unique_ptr<ComplianceRule> rule);

      void submit(const std::shared_ptr<const Order>& order) override;
      void cancel(const std::shared_ptr<const Order>& order) override;
      void add(const std::shared_ptr<const Order>& order) override;

    private:
      boost::posix_time::time_duration m_start;
      boost::posix_time::time_duration m_end;
      boost::local_time::tz_database m_time_zones;
      VenueDatabase m_venues;
      Beam::GetOptionalLocalPtr<C> m_time_client;
      Beam::SynchronizedUnorderedMap<Venue, boost::local_time::time_zone_ptr>
        m_venue_time_zones;
      std::unique_ptr<ComplianceRule> m_rule;

      bool is_within_period(Venue venue);
  };

  template<typename TimeClient>
  TimeFilterComplianceRule(boost::posix_time::time_duration,
    boost::posix_time::time_duration, boost::local_time::tz_database,
    VenueDatabase, TimeClient&&, std::unique_ptr<ComplianceRule>) ->
      TimeFilterComplianceRule<std::remove_reference_t<TimeClient>>;

  template<typename C>
  template<Beam::Initializes<C> CF>
  TimeFilterComplianceRule<C>::TimeFilterComplianceRule(
    boost::posix_time::time_duration start,
    boost::posix_time::time_duration end,
    boost::local_time::tz_database time_zones, VenueDatabase venues,
    CF&& time_client, std::unique_ptr<ComplianceRule> rule)
    : m_start(start),
      m_end(end),
      m_time_zones(std::move(time_zones)),
      m_venues(std::move(venues)),
      m_time_client(std::forward<CF>(time_client)),
      m_rule(std::move(rule)) {}

  template<typename C>
  void TimeFilterComplianceRule<C>::submit(
      const std::shared_ptr<const Order>& order) {
    if(is_within_period(order->get_info().m_fields.m_security.get_venue())) {
      m_rule->submit(order);
    } else {
      add(order);
    }
  }

  template<typename C>
  void TimeFilterComplianceRule<C>::cancel(
      const std::shared_ptr<const Order>& order) {
    if(is_within_period(order->get_info().m_fields.m_security.get_venue())) {
      m_rule->cancel(order);
    }
  }

  template<typename C>
  void TimeFilterComplianceRule<C>::add(
      const std::shared_ptr<const Order>& order) {
    m_rule->add(order);
  }

  template<typename C>
  bool TimeFilterComplianceRule<C>::is_within_period(Venue venue) {
    auto time_zone = m_venue_time_zones.GetOrInsert(venue, [&] {
      auto& venue_entry = m_venues.from(venue);
      auto time_zone =
        m_time_zones.time_zone_from_region(venue_entry.m_time_zone);
      if(!time_zone) {
        throw ComplianceCheckException("Time zone not found.");
      }
      return time_zone;
    });
    auto local_timestamp =
      boost::local_time::local_date_time(m_time_client->GetTime(), time_zone);
    auto local_time_of_day = local_timestamp.local_time().time_of_day();
    if(m_start > m_end) {
      if(local_time_of_day >= m_start || local_time_of_day <= m_end) {
        return true;
      }
    } else {
      if(local_time_of_day >= m_start && local_time_of_day <= m_end) {
        return true;
      }
    }
    return false;
  }
}

#endif
