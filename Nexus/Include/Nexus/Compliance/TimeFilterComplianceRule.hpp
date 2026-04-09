#ifndef NEXUS_TIME_FILTER_COMPLIANCE_RULE_HPP
#define NEXUS_TIME_FILTER_COMPLIANCE_RULE_HPP
#include <type_traits>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /**
   * Applies a ComplianceRule only during a specified time period relative the
   * venue an order is being submitted to or cancelled from.
   * @param <C> The type of TimeClient used to determine whether the
   *        ComplianceRule applies.
   */
  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  class TimeFilterComplianceRule : public ComplianceRule {
    public:

      /**
       * The type of TimeClient used to determine whether the ComplianceRule
       * applies.
       */
      using TimeClient = Beam::dereference_t<C>;

      /**
       * Constructs a TimeFilterComplianceRule.
       * @param start The beginning of the period to apply the rule.
       * @param end The end of the period to apply the rule.
       * @param time_zones The available time zones.
       * @param venues The venues available.
       * @param time_client Initializes the TimeClient used to test the period.
       * @param rule The rule to apply within the time period.
       */
      template<Beam::Initializes<C> CF>
      TimeFilterComplianceRule(boost::posix_time::time_duration start,
        boost::posix_time::time_duration end,
        boost::local_time::tz_database time_zones, VenueDatabase venues,
        CF&& time_client, std::unique_ptr<ComplianceRule> rule);

      void submit(const std::shared_ptr<Order>& order) override;
      void cancel(const std::shared_ptr<Order>& order) override;
      void add(const std::shared_ptr<Order>& order) override;

    private:
      boost::posix_time::time_duration m_start;
      boost::posix_time::time_duration m_end;
      boost::local_time::tz_database m_time_zones;
      VenueDatabase m_venues;
      Beam::local_ptr_t<C> m_time_client;
      Beam::SynchronizedUnorderedMap<Venue, boost::local_time::time_zone_ptr>
        m_venue_time_zones;
      std::unique_ptr<ComplianceRule> m_rule;

      bool is_within_period(Venue venue);
  };

  template<typename C>
  TimeFilterComplianceRule(boost::posix_time::time_duration,
    boost::posix_time::time_duration, boost::local_time::tz_database,
    VenueDatabase, C&&, std::unique_ptr<ComplianceRule>) ->
      TimeFilterComplianceRule<std::remove_cvref_t<C>>;

  /** The standard name used to identify the TimeFilterComplianceRule. */
  inline const auto TIME_FILTER_RULE_NAME = std::string("time_filter");

  /**
   * Returns a ComplianceRuleSchema representing a TimeFilterComplianceRule.
   * @param schema The ComplianceRuleSchema to apply within the time period.
   */
  inline ComplianceRuleSchema make_time_filter_compliance_rule_schema(
      const ComplianceRuleSchema& schema) {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back(
      "start", boost::posix_time::time_duration(boost::posix_time::seconds(0)));
    parameters.emplace_back(
      "end", boost::posix_time::time_duration(boost::posix_time::seconds(0)));
    return wrap(TIME_FILTER_RULE_NAME, std::move(parameters), schema);
  }

  /**
   * Makes a new TimeFilterComplianceRule.
   * @param parameters The parameters used to construct the rule.
   * @param time_zones The available time zones.
   * @param venues The venues available.
   * @param time_client The TimeClient used to test the period.
   * @param rule The rule to apply within the time period.
   */
  inline auto make_time_filter_compliance_rule(
      const std::vector<ComplianceParameter>& parameters,
      boost::local_time::tz_database time_zones, VenueDatabase venues,
      auto& time_client, std::unique_ptr<ComplianceRule> rule) {
    auto start =
      boost::posix_time::time_duration(boost::posix_time::seconds(0));
    auto end = boost::posix_time::time_duration(boost::posix_time::seconds(0));
    for(auto& parameter : parameters) {
      if(parameter.m_name == "start") {
        start = boost::get<boost::posix_time::time_duration>(parameter.m_value);
      } else if(parameter.m_name == "end") {
        end = boost::get<boost::posix_time::time_duration>(parameter.m_value);
      }
    }
    using Rule = TimeFilterComplianceRule<
      std::remove_cvref_t<decltype(time_client)>*>;
    return std::make_unique<Rule>(start, end, std::move(time_zones),
      std::move(venues), &time_client, std::move(rule));
  }

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
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

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  void TimeFilterComplianceRule<C>::submit(
      const std::shared_ptr<Order>& order) {
    if(is_within_period(order->get_info().m_fields.m_ticker.get_venue())) {
      m_rule->submit(order);
    } else {
      add(order);
    }
  }

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  void TimeFilterComplianceRule<C>::cancel(
      const std::shared_ptr<Order>& order) {
    if(is_within_period(order->get_info().m_fields.m_ticker.get_venue())) {
      m_rule->cancel(order);
    }
  }

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  void TimeFilterComplianceRule<C>::add(const std::shared_ptr<Order>& order) {
    m_rule->add(order);
  }

  template<typename C> requires Beam::IsTimeClient<Beam::dereference_t<C>>
  bool TimeFilterComplianceRule<C>::is_within_period(Venue venue) {
    auto time_zone = m_venue_time_zones.get_or_insert(venue, [&] {
      auto& venue_entry = m_venues.from(venue);
      auto time_zone =
        m_time_zones.time_zone_from_region(venue_entry.m_time_zone);
      if(!time_zone) {
        boost::throw_with_location(
          ComplianceCheckException("Time zone not found."));
      }
      return time_zone;
    });
    auto local_timestamp =
      boost::local_time::local_date_time(m_time_client->get_time(), time_zone);
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
