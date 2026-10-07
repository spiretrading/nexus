#include "WebPortal/ReportSchedule.hpp"
#include <algorithm>
#include <chrono>
#include <stdexcept>

using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
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
    static const auto EPOCH = ptime(date(1970, 1, 1));
    return std::chrono::microseconds((value - EPOCH).total_microseconds());
  }

  ptime from_epoch(std::chrono::microseconds value) {
    static const auto EPOCH = ptime(date(1970, 1, 1));
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
        constexpr auto DAYS_PER_WEEK = 7;
        amount *= DAYS_PER_WEEK;
      }
      auto available = (date(boost::date_time::max_date_time) - day).days();
      if(amount > static_cast<std::uint64_t>(available)) {
        throw ReportScheduleExhaustedException();
      }
      return ptime(day + days(static_cast<long>(amount)), start.time_of_day());
    }
    constexpr auto MONTHS_PER_YEAR = 12;
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
  if(interval.m_count == 0 ||
      interval.m_unit < ReportSchedule::Interval::Unit::DAY ||
      interval.m_unit > ReportSchedule::Interval::Unit::YEAR) {
    throw std::invalid_argument("Invalid repeat interval.");
  }
  if(*start > now) {
    return schedule.m_start_time;
  }
  auto local = from_utc(now, zone);
  auto elapsed = std::int64_t(0);
  if(interval.m_unit == ReportSchedule::Interval::Unit::DAY ||
      interval.m_unit == ReportSchedule::Interval::Unit::WEEK) {
    elapsed = (local.date() - schedule.m_start_time.date()).days();
    if(interval.m_unit == ReportSchedule::Interval::Unit::WEEK) {
      constexpr auto DAYS_PER_WEEK = 7;
      elapsed /= DAYS_PER_WEEK;
    }
  } else {
    elapsed =
      int(local.date().year()) - int(schedule.m_start_time.date().year());
    if(interval.m_unit == ReportSchedule::Interval::Unit::MONTH) {
      constexpr auto MONTHS_PER_YEAR = 12;
      elapsed = elapsed * MONTHS_PER_YEAR + int(local.date().month()) -
        int(schedule.m_start_time.date().month());
    }
  }
  auto steps =
    std::uint64_t(std::max<std::int64_t>(elapsed, 0)) / interval.m_count;
  while(true) {
    auto candidate = advance(schedule.m_start_time, interval, steps);
    auto utc = to_utc(candidate, zone);
    if(utc && *utc > now) {
      return candidate;
    }
    ++steps;
  }
}

ReportScheduleExhaustedException::ReportScheduleExhaustedException()
  : std::invalid_argument("Schedule exceeds the supported dates.") {}
