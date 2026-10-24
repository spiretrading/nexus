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
    } else if constexpr(std::is_same_v<Rule, OrdinalDayOfMonthDateRule>) {
      return DateRuleType::ORDINAL_DAY_OF_MONTH;
    } else {
      return DateRuleType::MONTH_BOUNDARY;
    }
  }, rule);
}

DateRuleType Nexus::parse_date_rule_type(std::string_view value) {
  if(value == "SPECIFIC_DATE") {
    return DateRuleType::SPECIFIC_DATE;
  } else if(value == "DAY_OFFSET") {
    return DateRuleType::DAY_OFFSET;
  } else if(value == "WEEKDAY") {
    return DateRuleType::WEEKDAY;
  } else if(value == "DAY_OF_MONTH") {
    return DateRuleType::DAY_OF_MONTH;
  } else if(value == "ORDINAL_DAY_OF_MONTH") {
    return DateRuleType::ORDINAL_DAY_OF_MONTH;
  } else if(value == "MONTH_BOUNDARY") {
    return DateRuleType::MONTH_BOUNDARY;
  }
  throw std::invalid_argument("Unknown date rule type.");
}

greg_weekday Nexus::parse_weekday(std::string_view value) {
  for(auto i = 0; i < DAYS_PER_WEEK; ++i) {
    auto day = greg_weekday(static_cast<unsigned short>(i));
    if(value == boost::to_upper_copy(std::string(day.as_long_string()))) {
      return day;
    }
  }
  throw std::invalid_argument("Unknown weekday.");
}

MonthBoundaryDateRule::Boundary Nexus::parse_month_boundary(
    std::string_view value) {
  if(value == "FIRST") {
    return MonthBoundaryDateRule::Boundary::FIRST;
  } else if(value == "LAST") {
    return MonthBoundaryDateRule::Boundary::LAST;
  }
  throw std::invalid_argument("Unknown month boundary.");
}

std::ostream& Nexus::operator <<(std::ostream& out, DateRuleType type) {
  if(type == DateRuleType::SPECIFIC_DATE) {
    return out << "SPECIFIC_DATE";
  } else if(type == DateRuleType::DAY_OFFSET) {
    return out << "DAY_OFFSET";
  } else if(type == DateRuleType::WEEKDAY) {
    return out << "WEEKDAY";
  } else if(type == DateRuleType::DAY_OF_MONTH) {
    return out << "DAY_OF_MONTH";
  } else if(type == DateRuleType::ORDINAL_DAY_OF_MONTH) {
    return out << "ORDINAL_DAY_OF_MONTH";
  } else if(type == DateRuleType::MONTH_BOUNDARY) {
    return out << "MONTH_BOUNDARY";
  }
  throw std::invalid_argument("Invalid date rule type.");
}

std::ostream& Nexus::operator <<(
    std::ostream& out, MonthBoundaryDateRule::Boundary boundary) {
  if(boundary == MonthBoundaryDateRule::Boundary::FIRST) {
    return out << "FIRST";
  } else if(boundary == MonthBoundaryDateRule::Boundary::LAST) {
    return out << "LAST";
  }
  throw std::invalid_argument("Invalid month boundary.");
}

std::ostream& Nexus::operator <<(
    std::ostream& out, const OrdinalDayOfMonthDateRule::Position& value) {
  if(auto first = std::get_if<OrdinalDayOfMonthDateRule::First>(&value)) {
    return out << "FIRST(" << static_cast<int>(first->m_ordinal) << ')';
  }
  return out << "LAST(" << static_cast<int>(
    std::get<OrdinalDayOfMonthDateRule::Last>(value).m_ordinal) << ')';
}

OrdinalDayOfMonthDateRule::Day Nexus::parse_ordinal_day(
    std::string_view value) {
  if(value == "DAY") {
    return OrdinalDayOfMonthDateRule::DayType::DAY;
  } else if(value == "WEEKDAY") {
    return OrdinalDayOfMonthDateRule::DayType::WEEKDAY;
  }
  return parse_weekday(value);
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

ptime Nexus::apply(const OrdinalDayOfMonthDateRule& rule, ptime reference) {
  validate(reference);
  using DayType = OrdinalDayOfMonthDateRule::DayType;
  auto ordinal = std::visit([] (const auto& position) {
    return position.m_ordinal;
  }, rule.m_position);
  auto category = std::get_if<DayType>(&rule.m_day);
  if(ordinal == 0 || (category && *category != DayType::DAY &&
      *category != DayType::WEEKDAY)) {
    throw std::invalid_argument("Invalid ordinal day of month.");
  }
  auto month = offset_months(reference.date(), rule.m_offset);
  auto last = static_cast<int>(month.end_of_month().day());
  auto remaining = ordinal;
  auto step = 1;
  auto number = 1;
  if(std::holds_alternative<OrdinalDayOfMonthDateRule::Last>(
      rule.m_position)) {
    step = -1;
    number = last;
  }
  for(; number >= 1 && number <= last; number += step) {
    auto day = date(month.year(), month.month(),
      static_cast<unsigned short>(number));
    auto weekday = std::get_if<greg_weekday>(&rule.m_day);
    if((weekday && day.day_of_week() == *weekday) ||
        (category && (*category == DayType::DAY ||
          (day.day_of_week() != Saturday && day.day_of_week() != Sunday)))) {
      --remaining;
      if(remaining == 0) {
        return ptime(day, reference.time_of_day());
      }
    }
  }
  throw std::invalid_argument("Ordinal day does not occur in the month.");
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

std::format_context::iterator
    std::formatter<OrdinalDayOfMonthDateRule::Day>::format(
      const OrdinalDayOfMonthDateRule::Day& value,
      std::format_context& context) const {
  auto name = [&] {
    if(auto day = std::get_if<greg_weekday>(&value)) {
      return boost::to_upper_copy(std::string(day->as_long_string()));
    } else if(std::get<OrdinalDayOfMonthDateRule::DayType>(value) ==
        OrdinalDayOfMonthDateRule::DayType::DAY) {
      return std::string("DAY");
    } else if(std::get<OrdinalDayOfMonthDateRule::DayType>(value) ==
        OrdinalDayOfMonthDateRule::DayType::WEEKDAY) {
      return std::string("WEEKDAY");
    }
    throw std::invalid_argument("Invalid ordinal day.");
  }();
  return std::formatter<std::string_view>::format(name, context);
}
