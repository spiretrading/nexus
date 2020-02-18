#include "Spire/Ui/CalendarModel.hpp"

using namespace boost::gregorian;
using namespace Spire;

boost::gregorian::date CalendarModel::get_date(int row, int column) {
  return m_dates[row * 7 + column];
}

std::tuple<int, int> CalendarModel::get_pos(date date) {
  if(date.month() < m_reference_date.month()) {
    return {date.day_of_week(), 0};
  }
  // TODO: definitely possible to calculate this in constant time.
  auto day_index = std::distance(m_dates.begin(),
    std::find(m_dates.begin(), m_dates.end(), date));
  return {day_index % 7, static_cast<int>(std::floor(day_index / 7))};
}

void CalendarModel::set_month(int month, int year) {
  m_reference_date = boost::gregorian::date(year, month, 1);
  auto day_count = m_reference_date.end_of_month().day();
  auto first_day_of_week = m_reference_date.day_of_week().as_number();
  for(auto i = 0; i < 42; ++i) {
    if(i < first_day_of_week) {
      m_dates[i] = m_reference_date +
        boost::gregorian::days(i - first_day_of_week);
    } else if(i > day_count + first_day_of_week - 1) {
      m_dates[i] = boost::gregorian::date(m_reference_date.year(),
        m_reference_date.month(),
        i - day_count - first_day_of_week + 1) +
        boost::gregorian::months(1);
    } else {
      m_dates[i] = boost::gregorian::date(m_reference_date.year(),
        m_reference_date.month(),
        i + 1 - first_day_of_week);
    }
  }
}
