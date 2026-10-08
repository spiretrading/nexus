#include "WebPortal/DateRule.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <type_traits>

using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  constexpr auto DAYS_PER_WEEK = weeks(1).days();
  constexpr auto MONTHS_PER_YEAR = 12;
  const auto MINIMUM_DATE = date(boost::date_time::min_date_time);
  const auto MAXIMUM_DATE = date(boost::date_time::max_date_time);

  void validate(ptime reference) {
    if(reference.is_special()) {
      throw std::invalid_argument("Invalid reference timestamp.");
    }
  }

  date offset_days(date reference, std::int64_t offset) {
    if(offset < (MINIMUM_DATE - reference).days() ||
        offset > (MAXIMUM_DATE - reference).days()) {
      throw std::out_of_range("Date rule result is out of range.");
    }
    return reference + days(static_cast<long>(offset));
  }

  date offset_months(date reference, std::int32_t offset) {
    auto index = std::int64_t(reference.year()) * MONTHS_PER_YEAR +
      reference.month() - 1 + offset;
    auto minimum = std::int64_t(MINIMUM_DATE.year()) * MONTHS_PER_YEAR;
    auto maximum = std::int64_t(MAXIMUM_DATE.year()) * MONTHS_PER_YEAR +
      MONTHS_PER_YEAR - 1;
    if(index < minimum || index > maximum) {
      throw std::out_of_range("Date rule month is out of range.");
    }
    return date(static_cast<unsigned short>(index / MONTHS_PER_YEAR),
      static_cast<unsigned short>(index % MONTHS_PER_YEAR + 1), 1);
  }
}

DateRuleType Nexus::get_type(const DateRule& rule) {
  return std::visit([] (const auto& rule) {
    using Rule = std::remove_cvref_t<decltype(rule)>;
    if constexpr(std::is_same_v<Rule, SpecificDateRule>) {
      return DateRuleType::SPECIFIC_DATE;
    } else if constexpr(std::is_same_v<Rule, DayOffsetDateRule>) {
      return DateRuleType::DAY_OFFSET;
    } else if constexpr(std::is_same_v<Rule, WeekdayDateRule>) {
      return DateRuleType::WEEKDAY;
    } else if constexpr(std::is_same_v<Rule, DayOfMonthDateRule>) {
      return DateRuleType::DAY_OF_MONTH;
    } else {
      return DateRuleType::MONTH_BOUNDARY;
    }
  }, rule);
}

DateRuleType Nexus::parse_date_rule_type(std::string_view value) {
  if(value == "SpecificDate") {
    return DateRuleType::SPECIFIC_DATE;
  } else if(value == "DayOffset") {
    return DateRuleType::DAY_OFFSET;
  } else if(value == "Weekday") {
    return DateRuleType::WEEKDAY;
  } else if(value == "DayOfMonth") {
    return DateRuleType::DAY_OF_MONTH;
  } else if(value == "MonthBoundary") {
    return DateRuleType::MONTH_BOUNDARY;
  }
  throw std::invalid_argument("Unknown date rule type.");
}

greg_weekday Nexus::parse_weekday(std::string_view value) {
  for(auto i = 0; i < DAYS_PER_WEEK; ++i) {
    auto day = greg_weekday(static_cast<unsigned short>(i));
    if(value == day.as_long_string()) {
      return day;
    }
  }
  throw std::invalid_argument("Unknown weekday.");
}

MonthBoundaryDateRule::Boundary Nexus::parse_month_boundary(
    std::string_view value) {
  if(value == "First") {
    return MonthBoundaryDateRule::Boundary::FIRST;
  } else if(value == "Last") {
    return MonthBoundaryDateRule::Boundary::LAST;
  }
  throw std::invalid_argument("Unknown month boundary.");
}

std::ostream& Nexus::operator <<(std::ostream& out, DateRuleType type) {
  if(type == DateRuleType::SPECIFIC_DATE) {
    return out << "SpecificDate";
  } else if(type == DateRuleType::DAY_OFFSET) {
    return out << "DayOffset";
  } else if(type == DateRuleType::WEEKDAY) {
    return out << "Weekday";
  } else if(type == DateRuleType::DAY_OF_MONTH) {
    return out << "DayOfMonth";
  } else if(type == DateRuleType::MONTH_BOUNDARY) {
    return out << "MonthBoundary";
  }
  throw std::invalid_argument("Invalid date rule type.");
}

std::ostream& Nexus::operator <<(
    std::ostream& out, MonthBoundaryDateRule::Boundary boundary) {
  if(boundary == MonthBoundaryDateRule::Boundary::FIRST) {
    return out << "First";
  } else if(boundary == MonthBoundaryDateRule::Boundary::LAST) {
    return out << "Last";
  }
  throw std::invalid_argument("Invalid month boundary.");
}

ptime Nexus::apply(const SpecificDateRule& rule, ptime reference) {
  validate(reference);
  if(rule.m_date.is_special()) {
    throw std::invalid_argument("Invalid fixed date.");
  }
  return ptime(rule.m_date, reference.time_of_day());
}

ptime Nexus::apply(const DayOffsetDateRule& rule, ptime reference) {
  validate(reference);
  return ptime(offset_days(reference.date(), rule.m_offset),
    reference.time_of_day());
}

ptime Nexus::apply(const WeekdayDateRule& rule, ptime reference) {
  validate(reference);
  auto weekday = (reference.date().day_of_week().as_number() +
    DAYS_PER_WEEK - Monday) % DAYS_PER_WEEK;
  auto target =
    (rule.m_day.as_number() + DAYS_PER_WEEK - Monday) % DAYS_PER_WEEK;
  auto offset = std::int64_t(rule.m_offset) * DAYS_PER_WEEK + target - weekday;
  return ptime(offset_days(reference.date(), offset), reference.time_of_day());
}

ptime Nexus::apply(const DayOfMonthDateRule& rule, ptime reference) {
  validate(reference);
  constexpr auto MAXIMUM_DAY = 31;
  if(rule.m_day < 1 || rule.m_day > MAXIMUM_DAY) {
    throw std::invalid_argument("Invalid day of month.");
  }
  auto month = offset_months(reference.date(), rule.m_offset);
  auto day = std::min(rule.m_day,
    static_cast<std::int32_t>(month.end_of_month().day()));
  return ptime(date(month.year(), month.month(),
    static_cast<unsigned short>(day)), reference.time_of_day());
}

ptime Nexus::apply(const MonthBoundaryDateRule& rule, ptime reference) {
  validate(reference);
  auto boundary = offset_months(reference.date(), rule.m_offset);
  if(rule.m_boundary == MonthBoundaryDateRule::Boundary::LAST) {
    boundary = boundary.end_of_month();
  } else if(rule.m_boundary != MonthBoundaryDateRule::Boundary::FIRST) {
    throw std::invalid_argument("Invalid month boundary.");
  }
  return ptime(offset_days(boundary, rule.m_day_offset),
    reference.time_of_day());
}

ptime Nexus::apply(const DateRule& rule, ptime reference) {
  return std::visit([&] (const auto& rule) {
    return apply(rule, reference);
  }, rule);
}

std::int32_t Nexus::Details::parse_date_rule_integer(
    double value, std::int32_t minimum, std::int32_t maximum) {
  if(!std::isfinite(value) || std::trunc(value) != value ||
      value < minimum || value > maximum) {
    throw std::invalid_argument("Invalid date rule integer.");
  }
  return static_cast<std::int32_t>(value);
}
