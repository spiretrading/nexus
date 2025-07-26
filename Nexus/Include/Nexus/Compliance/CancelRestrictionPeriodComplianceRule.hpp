#ifndef NEXUS_CANCEL_RESTRICTION_PERIOD_COMPLIANCE_RULE_HPP
#define NEXUS_CANCEL_RESTRICTION_PERIOD_COMPLIANCE_RULE_HPP
#include <vector>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/date_time/local_time/tz_database.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus::Compliance {

  /**
   * Rejects cancel requests made during a specified time period.
   * @param <C> The type of TimeClient used to check the time of an order cancel
   *            request.
   */
  template<typename C>
  class CancelRestrictionPeriodComplianceRule : public ComplianceRule {
    public:

      /**
       * The type of TimeClient used to check the time of an order cancel
       * request.
       */
      using TimeClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a CancelRestrictionPeriodComplianceRule.
       * @param parameters The list of parameters used by this rule.
       * @param time_zones The available time zones.
       * @param venues The venues available.
       * @param time_client Initializes the TimeClient used to check order
       *        cancel requests.
       */
      template<typename CF>
      CancelRestrictionPeriodComplianceRule(
        const std::vector<ComplianceParameter>& parameters,
        boost::local_time::tz_database time_zones, VenueDatabase venues,
        CF&& time_client);

      /**
       * Constructs a CancelRestrictionPeriodComplianceRule.
       * @param start The beginning of the period to restrict cancels.
       * @param end The end of the period to restrict cancels.
       * @param time_zones The available time zones.
       * @param venues The venues available.
       * @param time_client Initializes the TimeClient used to check order
       *        cancel requests.
       */
      template<typename CF>
      CancelRestrictionPeriodComplianceRule(
        boost::posix_time::time_duration start,
        boost::posix_time::time_duration end,
        boost::local_time::tz_database time_zones, VenueDatabase venues,
        CF&& time_client);

      void cancel(const std::shared_ptr<
        const OrderExecutionService::Order>& order) override;

    private:
      boost::posix_time::time_duration m_start;
      boost::posix_time::time_duration m_end;
      boost::local_time::tz_database m_time_zones;
      VenueDatabase m_venues;
      Beam::GetOptionalLocalPtr<C> m_time_client;
      Beam::SynchronizedUnorderedMap<Venue, boost::local_time::time_zone_ptr>
        m_venue_time_zones;
  };

  /**
   * Makes a ComplianceRuleSchema representing a
   * CancelRestrictionPeriodComplianceRule.
   */
  inline ComplianceRuleSchema
      make_cancel_restriction_period_compliance_rule_schema() {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("start", boost::posix_time::time_duration());
    parameters.emplace_back("end", boost::posix_time::time_duration());
    return ComplianceRuleSchema("cancel_restriction_period", parameters);
  }

  template<typename C>
  template<typename CF>
  CancelRestrictionPeriodComplianceRule<C>::
      CancelRestrictionPeriodComplianceRule(
        const std::vector<ComplianceParameter>& parameters,
        boost::local_time::tz_database time_zones, VenueDatabase venues,
        CF&& time_client)
      : m_time_zones(std::move(time_zones)),
        m_venues(std::move(venues)),
        m_time_client(std::forward<CF>(time_client)) {
    for(auto& parameter : parameters) {
      if(parameter.m_name == "start") {
        m_start =
          boost::get<boost::posix_time::time_duration>(parameter.m_value);
      } else if(parameter.m_name == "end") {
        m_end =
          boost::get<boost::posix_time::time_duration>(parameter.m_value);
      }
    }
  }

  template<typename C>
  template<typename CF>
  CancelRestrictionPeriodComplianceRule<C>::
      CancelRestrictionPeriodComplianceRule(
        boost::posix_time::time_duration start,
        boost::posix_time::time_duration end,
        boost::local_time::tz_database time_zones, VenueDatabase venues,
        CF&& time_client)
    : m_start(start),
      m_end(end),
      m_time_zones(std::move(time_zones)),
      m_venues(std::move(venues)),
      m_time_client(std::forward<CF>(time_client)) {}

  template<typename C>
  void CancelRestrictionPeriodComplianceRule<C>::cancel(
      const std::shared_ptr<const OrderExecutionService::Order>& order) {
    auto& security = order->get_info().m_fields.m_security;
    auto time_zone = m_venue_time_zones.GetOrInsert(security.get_venue(), [&] {
      auto& venue_entry = m_venues.from(security.get_venue());
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
        throw ComplianceCheckException("Cancels not permitted at this time.");
      }
    } else {
      if(local_time_of_day >= m_start && local_time_of_day <= m_end) {
        throw ComplianceCheckException("Cancels not permitted at this time.");
      }
    }
  }
}

#endif
