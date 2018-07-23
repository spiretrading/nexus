#include "Spire/Charting/ChartValue.hpp"
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace std;

ChartValue::ChartValue(Money value)
    : m_value{static_cast<Quantity>(value)} {}

ChartValue::ChartValue(Quantity value)
    : m_value{value} {}

ChartValue::ChartValue(const ptime& value) {
  if(value == not_a_date_time) {
    m_value = numeric_limits<Quantity>::quiet_NaN();
  } else if(value == pos_infin) {
    m_value = numeric_limits<Quantity>::infinity();
  } else if(value == neg_infin) {
    m_value = -numeric_limits<Quantity>::infinity();
  } else {
    static const ptime BASE(date(1970, Jan, 1), seconds(0));
    time_duration delta = value - BASE;
    m_value = delta.total_milliseconds();
  }
}

ChartValue::ChartValue(const time_duration& value)
    : m_value{value.total_milliseconds()} {}

bool ChartValue::operator <(ChartValue rhs) const {
  return m_value < rhs.m_value;
}

bool ChartValue::operator <=(ChartValue rhs) const {
  return m_value <= rhs.m_value;
}

bool ChartValue::operator ==(ChartValue rhs) const {
  return m_value == rhs.m_value;
}

bool ChartValue::operator !=(ChartValue rhs) const {
  return m_value != rhs.m_value;
}

bool ChartValue::operator >=(ChartValue rhs) const {
  return m_value >= rhs.m_value;
}

bool ChartValue::operator >(ChartValue rhs) const {
  return m_value > rhs.m_value;
}

ChartValue ChartValue::operator +(ChartValue rhs) const {
  return ChartValue{m_value + rhs.m_value};
}

ChartValue& ChartValue::operator +=(ChartValue rhs) {
  m_value += rhs.m_value;
  return *this;
}

ChartValue ChartValue::operator -(ChartValue rhs) const {
  return ChartValue{m_value - rhs.m_value};
}

ChartValue ChartValue::operator %(ChartValue rhs) const {
  return ChartValue{m_value % rhs.m_value};
}

double ChartValue::operator /(ChartValue rhs) const {
  return static_cast<double>(m_value / rhs.m_value);
}

ChartValue& ChartValue::operator -=(ChartValue rhs) {
  m_value -= rhs.m_value;
  return *this;
}

ChartValue ChartValue::operator -() const {
  return ChartValue{-m_value};
}

ptime ChartValue::ToDateTime() const {
  static const ptime BASE(date(1970, Jan, 1), seconds(0));
  return BASE + milliseconds(static_cast<std::int64_t>(m_value));
}

Money ChartValue::ToMoney() const {
  return Money{m_value};
}

Quantity ChartValue::ToQuantity() const {
  return m_value;
}

time_duration ChartValue::ToTimeDuration() const {
  return milliseconds(static_cast<std::int64_t>(m_value));
}
