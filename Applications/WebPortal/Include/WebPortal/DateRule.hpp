#ifndef NEXUS_DATE_RULE_HPP
#define NEXUS_DATE_RULE_HPP
#include <cstdint>
#include <limits>
#include <ostream>
#include <string_view>
#include <variant>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Utilities/ToString.hpp>

namespace Nexus {

  /** Selects a fixed calendar date. */
  struct SpecificDateRule {

    /** The fixed calendar date. */
    boost::gregorian::date m_date;
  };

  /** Offsets the reference date by calendar days. */
  struct DayOffsetDateRule {

    /** The signed number of days from the reference date. */
    std::int32_t m_offset = 0;
  };

  /** Selects a weekday in a week relative to the reference week. */
  struct WeekdayDateRule {

    /** The signed number of weeks from the Monday-based reference week. */
    std::int32_t m_offset = 0;

    /** The selected weekday. */
    boost::gregorian::greg_weekday m_day = boost::gregorian::Monday;
  };

  /** Selects a numbered day in a month relative to the reference month. */
  struct DayOfMonthDateRule {

    /** The signed number of months from the reference month. */
    std::int32_t m_offset = 0;

    /** The day from 1 to 31, clamped to the month's last day. */
    std::int32_t m_day = 1;
  };

  /** Selects a date relative to a calendar month's boundary. */
  struct MonthBoundaryDateRule {

    /** A calendar month's boundary. */
    enum class Boundary {

      /** The first day of the month. */
      FIRST,

      /** The last day of the month. */
      LAST
    };

    /** The signed number of months from the reference month. */
    std::int32_t m_offset = 0;

    /** The selected month boundary. */
    Boundary m_boundary = Boundary::FIRST;

    /** The signed number of days from the selected boundary. */
    std::int32_t m_day_offset = 0;
  };

  /** The supported calendar date transformations. */
  enum class DateRuleType {

    /** A fixed calendar date. */
    SPECIFIC_DATE,

    /** A calendar day offset. */
    DAY_OFFSET,

    /** A weekday in a relative calendar week. */
    WEEKDAY,

    /** A numbered day in a relative calendar month. */
    DAY_OF_MONTH,

    /** A day offset from a relative calendar month's boundary. */
    MONTH_BOUNDARY
  };

  /** A calendar date transformation. */
  class DateRule : public std::variant<SpecificDateRule, DayOffsetDateRule,
      WeekdayDateRule, DayOfMonthDateRule, MonthBoundaryDateRule> {
    public:

      /** The supported rule alternatives. */
      using Variant = std::variant<SpecificDateRule, DayOffsetDateRule,
        WeekdayDateRule, DayOfMonthDateRule, MonthBoundaryDateRule>;

      using Variant::Variant;
      using Variant::operator =;
  };

  /** Returns the type of the active rule. */
  DateRuleType get_type(const DateRule& rule);

  /** Parses a rule type's serialized name. */
  DateRuleType parse_date_rule_type(std::string_view value);

  /** Parses a weekday's full English name. */
  boost::gregorian::greg_weekday parse_weekday(std::string_view value);

  /** Parses a month boundary's serialized name. */
  MonthBoundaryDateRule::Boundary parse_month_boundary(std::string_view value);

  std::ostream& operator <<(std::ostream& out, DateRuleType type);
  std::ostream& operator <<(
    std::ostream& out, MonthBoundaryDateRule::Boundary boundary);

  /** Replaces the reference date, preserving its time of day. */
  boost::posix_time::ptime apply(
    const SpecificDateRule& rule, boost::posix_time::ptime reference);

  /** Offsets the reference date, preserving its time of day. */
  boost::posix_time::ptime apply(
    const DayOffsetDateRule& rule, boost::posix_time::ptime reference);

  /** Selects a weekday, preserving the reference's time of day. */
  boost::posix_time::ptime apply(
    const WeekdayDateRule& rule, boost::posix_time::ptime reference);

  /** Selects a month's numbered day, preserving the time of day. */
  boost::posix_time::ptime apply(
    const DayOfMonthDateRule& rule, boost::posix_time::ptime reference);

  /**
   * Selects a date relative to a month boundary, preserving the time of day.
   */
  boost::posix_time::ptime apply(
    const MonthBoundaryDateRule& rule, boost::posix_time::ptime reference);

  /**
   * Applies a rule to a calendar timestamp in the caller's chosen timezone.
   * @throws std::invalid_argument If the rule or reference is invalid.
   * @throws std::out_of_range If the result is outside the supported dates.
   */
  boost::posix_time::ptime apply(
    const DateRule& rule, boost::posix_time::ptime reference);

namespace Details {
  std::int32_t parse_date_rule_integer(
    double value, std::int32_t minimum, std::int32_t maximum);

  template<Beam::IsShuttle S>
  void shuttle_date_rule_integer(S& shuttle, const char* name,
      std::int32_t& value, std::int32_t minimum, std::int32_t maximum) {
    auto number = static_cast<double>(value);
    shuttle.shuttle(name, number);
    auto checked = parse_date_rule_integer(number, minimum, maximum);
    if constexpr(Beam::IsReceiver<S>) {
      value = checked;
    }
  }

  template<Beam::IsShuttle S>
  void shuttle_date_rule_offset(
      S& shuttle, const char* name, std::int32_t& value) {
    shuttle_date_rule_integer(shuttle, name, value,
      std::numeric_limits<std::int32_t>::min(),
      std::numeric_limits<std::int32_t>::max());
  }
}
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::SpecificDateRule> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::SpecificDateRule& value,
        unsigned int version) const {
      shuttle.shuttle("date", value.m_date);
      if(value.m_date.is_special()) {
        throw std::invalid_argument("Invalid fixed date.");
      }
    }
  };

  template<>
  struct Shuttle<Nexus::DayOffsetDateRule> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::DayOffsetDateRule& value,
        unsigned int version) const {
      Nexus::Details::shuttle_date_rule_offset(
        shuttle, "offset", value.m_offset);
    }
  };

  template<>
  struct Shuttle<Nexus::WeekdayDateRule> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::WeekdayDateRule& value,
        unsigned int version) const {
      Nexus::Details::shuttle_date_rule_offset(
        shuttle, "offset", value.m_offset);
      auto day = std::string(value.m_day.as_long_string());
      shuttle.shuttle("day", day);
      if constexpr(IsReceiver<S>) {
        value.m_day = Nexus::parse_weekday(day);
      }
    }
  };

  template<>
  struct Shuttle<Nexus::DayOfMonthDateRule> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::DayOfMonthDateRule& value,
        unsigned int version) const {
      Nexus::Details::shuttle_date_rule_offset(
        shuttle, "offset", value.m_offset);
      constexpr auto MAXIMUM_DAY = 31;
      Nexus::Details::shuttle_date_rule_integer(
        shuttle, "day", value.m_day, 1, MAXIMUM_DAY);
    }
  };

  template<>
  struct Shuttle<Nexus::MonthBoundaryDateRule> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::MonthBoundaryDateRule& value,
        unsigned int version) const {
      Nexus::Details::shuttle_date_rule_offset(
        shuttle, "offset", value.m_offset);
      auto boundary = to_string(value.m_boundary);
      shuttle.shuttle("boundary", boundary);
      if constexpr(IsReceiver<S>) {
        value.m_boundary = Nexus::parse_month_boundary(boundary);
      }
      Nexus::Details::shuttle_date_rule_offset(
        shuttle, "day_offset", value.m_day_offset);
    }
  };

  template<>
  struct Send<Nexus::DateRule> {
    template<IsSender S>
    void operator ()(
        S& sender, const Nexus::DateRule& value, unsigned int version) const {
      sender.send("type", to_string(Nexus::get_type(value)));
      std::visit([&] (const auto& rule) {
        sender.send("value", rule);
      }, value);
    }
  };

  template<>
  struct Receive<Nexus::DateRule> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, Nexus::DateRule& value, unsigned int version) const {
      auto type = Nexus::parse_date_rule_type(
        receive<std::string>(receiver, "type"));
      if(type == Nexus::DateRuleType::SPECIFIC_DATE) {
        value = receive<Nexus::SpecificDateRule>(receiver, "value");
      } else if(type == Nexus::DateRuleType::DAY_OFFSET) {
        value = receive<Nexus::DayOffsetDateRule>(receiver, "value");
      } else if(type == Nexus::DateRuleType::WEEKDAY) {
        value = receive<Nexus::WeekdayDateRule>(receiver, "value");
      } else if(type == Nexus::DateRuleType::DAY_OF_MONTH) {
        value = receive<Nexus::DayOfMonthDateRule>(receiver, "value");
      } else if(type == Nexus::DateRuleType::MONTH_BOUNDARY) {
        value = receive<Nexus::MonthBoundaryDateRule>(receiver, "value");
      }
    }
  };
}

#endif
