#include "WebPortal/ReportSchedule.hpp"
#include <algorithm>
#include <chrono>
#include <stdexcept>

using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  const auto EPOCH = ptime(date(1970, 1, 1));
  constexpr auto DAYS_PER_WEEK = weeks(1).days();
  constexpr auto MONTHS_PER_YEAR =
    std::chrono::months(std::chrono::years(1)).count();

  const std::chrono::time_zone& find_zone(const std::string& name) {
    if(name.empty()) {
      throw std::invalid_argument("Missing schedule timezone.");
    }
    auto& database = std::chrono::get_tzdb();
    try {
      return *database.locate_zone(name);
    } catch(const std::runtime_error&) {
      throw std::invalid_argument("Unknown schedule timezone.");
    }
  }

  std::chrono::microseconds since_epoch(ptime value) {
    if(value.is_special()) {
      throw std::invalid_argument("Invalid schedule date and time.");
    }
    return std::chrono::microseconds((value - EPOCH).total_microseconds());
  }

  ptime from_epoch(std::chrono::microseconds value) {
    return EPOCH + microseconds(value.count());
  }

  std::optional<ptime> to_utc(ptime value, const std::chrono::time_zone& zone) {
    auto local =
      std::chrono::local_time<std::chrono::microseconds>(since_epoch(value));
    if(zone.get_info(local).result == std::chrono::local_info::nonexistent) {
      return std::nullopt;
    }
    return from_epoch(
      zone.to_sys(local, std::chrono::choose::earliest).time_since_epoch());
  }

  ptime from_utc(ptime value, const std::chrono::time_zone& zone) {
    return from_epoch(
      zone.to_local(std::chrono::sys_time<std::chrono::microseconds>(
        since_epoch(value))).time_since_epoch());
  }

  ptime advance(
      ptime start, ReportSchedule::Interval interval, std::uint64_t steps) {
    auto amount = steps * interval.m_count;
    auto day = start.date();
    if(interval.m_unit == ReportSchedule::Interval::Unit::DAY ||
        interval.m_unit == ReportSchedule::Interval::Unit::WEEK) {
      if(interval.m_unit == ReportSchedule::Interval::Unit::WEEK) {
        amount *= DAYS_PER_WEEK;
      }
      auto available = (date(boost::date_time::max_date_time) - day).days();
      if(amount > static_cast<std::uint64_t>(available)) {
        throw ReportScheduleExhaustedException();
      }
      return ptime(day + days(static_cast<long>(amount)), start.time_of_day());
    }
    if(interval.m_unit == ReportSchedule::Interval::Unit::YEAR) {
      amount *= MONTHS_PER_YEAR;
    }
    auto month = std::uint64_t(day.year()) * MONTHS_PER_YEAR + day.month() - 1;
    auto maximum = std::uint64_t(date(boost::date_time::max_date_time).year()) *
      MONTHS_PER_YEAR + MONTHS_PER_YEAR - 1;
    if(amount > maximum - month) {
      throw ReportScheduleExhaustedException();
    }
    month += amount;
    auto year = static_cast<unsigned short>(month / MONTHS_PER_YEAR);
    auto index = static_cast<unsigned short>(month % MONTHS_PER_YEAR + 1);
    auto number = std::min<unsigned short>(
      day.day(), gregorian_calendar::end_of_month_day(year, index));
    return ptime(date(year, index, number), start.time_of_day());
  }
}

ptime Nexus::convert_report_time(
    ptime value, const std::string& source, const std::string& destination) {
  auto& source_zone = find_zone(source);
  auto& destination_zone = find_zone(destination);
  auto utc = to_utc(value, source_zone);
  if(!utc) {
    throw std::invalid_argument(
      "Schedule time does not exist in its timezone.");
  }
  return from_utc(*utc, destination_zone);
}

void Nexus::validate(const ReportSchedule::Interval& interval) {
  using Unit = ReportSchedule::Interval::Unit;
  if(interval.m_count == 0 || interval.m_unit < Unit::DAY ||
      interval.m_unit > Unit::YEAR) {
    throw std::invalid_argument("Invalid repeat interval.");
  }
  if(!interval.m_rule) {
    return;
  }
  auto is_valid = std::visit([&] (const auto& rule) {
    using Rule = std::remove_cvref_t<decltype(rule)>;
    if constexpr(std::is_same_v<Rule, SpecificDateRule>) {
      return false;
    } else if constexpr(std::is_same_v<Rule, DayOffsetDateRule>) {
      return rule.m_offset == 0;
    } else if constexpr(std::is_same_v<Rule, WeekdayDateRule>) {
      return interval.m_unit == Unit::WEEK && rule.m_offset == 0;
    } else {
      if((interval.m_unit != Unit::MONTH && interval.m_unit != Unit::YEAR) ||
          rule.m_offset != 0) {
        return false;
      }
      if constexpr(std::is_same_v<Rule, DayOfMonthDateRule>) {
        return rule.m_day >= 1 && rule.m_day <= 31;
      } else {
        return rule.m_boundary == MonthBoundaryDateRule::Boundary::FIRST ||
          rule.m_boundary == MonthBoundaryDateRule::Boundary::LAST;
      }
    }
  }, *interval.m_rule);
  if(!is_valid) {
    throw std::invalid_argument("Invalid repeat date rule.");
  }
}

ptime Nexus::next_report_run(const ReportSchedule& schedule, ptime now) {
  auto& zone = find_zone(schedule.m_time_zone);
  auto start = to_utc(schedule.m_start_time, zone);
  if(!start) {
    throw std::invalid_argument(
      "Schedule time does not exist in its timezone.");
  }
  if(!schedule.m_repeat_interval) {
    return schedule.m_start_time;
  }
  auto interval = *schedule.m_repeat_interval;
  validate(interval);
  if(!interval.m_rule && *start > now) {
    return schedule.m_start_time;
  }
  auto local = std::max(from_utc(now, zone), schedule.m_start_time);
  auto day_offset = std::int64_t(0);
  if(interval.m_rule) {
    if(auto rule = std::get_if<MonthBoundaryDateRule>(&*interval.m_rule)) {
      day_offset = rule->m_day_offset;
    }
    auto minimum = date(boost::date_time::min_date_time);
    auto maximum = date(boost::date_time::max_date_time);
    if(-day_offset > (maximum - local.date()).days()) {
      throw ReportScheduleExhaustedException();
    } else if(-day_offset < (minimum - local.date()).days()) {
      local = ptime(minimum, local.time_of_day());
    } else {
      local -= days(static_cast<long>(day_offset));
    }
  }
  auto elapsed = std::int64_t(0);
  if(interval.m_unit == ReportSchedule::Interval::Unit::DAY ||
      interval.m_unit == ReportSchedule::Interval::Unit::WEEK) {
    elapsed = (local.date() - schedule.m_start_time.date()).days();
    if(interval.m_unit == ReportSchedule::Interval::Unit::WEEK) {
      elapsed /= DAYS_PER_WEEK;
    }
  } else {
    elapsed =
      int(local.date().year()) - int(schedule.m_start_time.date().year());
    if(interval.m_unit == ReportSchedule::Interval::Unit::MONTH) {
      elapsed = elapsed * MONTHS_PER_YEAR + int(local.date().month()) -
        int(schedule.m_start_time.date().month());
    }
  }
  auto steps =
    std::uint64_t(std::max<std::int64_t>(elapsed, 0)) / interval.m_count;
  if(interval.m_rule && steps != 0) {
    --steps;
  }
  while(true) {
    auto candidate = advance(schedule.m_start_time, interval, steps);
    if(interval.m_rule) {
      try {
        candidate = apply(*interval.m_rule, candidate);
      } catch(const std::out_of_range&) {
        if(day_offset < 0 || candidate.date().year() ==
            date(boost::date_time::min_date_time).year()) {
          ++steps;
          continue;
        }
        throw ReportScheduleExhaustedException();
      }
    }
    auto utc = to_utc(candidate, zone);
    if(candidate >= schedule.m_start_time && utc && *utc > now) {
      return candidate;
    }
    ++steps;
  }
}

ReportScheduleExhaustedException::ReportScheduleExhaustedException()
  : std::invalid_argument("Schedule exceeds the supported dates.") {}
