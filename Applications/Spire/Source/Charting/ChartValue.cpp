#include "Spire/Charting/ChartValue.hpp"
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace std;

ChartValue ChartValue::FromRepresentation(int64_t value) {
  ChartValue v;
  v.m_value = value;
  return v;
}

ChartValue::ChartValue()
    : m_value(0){}

ChartValue::ChartValue(Money value)
    : m_value(value.GetRepresentation()){}

ChartValue::ChartValue(Quantity value)
    : m_value(value){}

ChartValue::ChartValue(const ptime& value) {
  if(value == not_a_date_time) {
    m_value = numeric_limits<std::int64_t>::max();
  } else if(value == pos_infin) {
    m_value = numeric_limits<std::int64_t>::max() - 1;
  } else if(value == neg_infin) {
     m_value = numeric_limits<std::int64_t>::min();
  } else {
    static const ptime BASE(date(1970, Jan, 1), seconds(0));
    time_duration delta = value - BASE;
    m_value = delta.total_milliseconds();
  }
}

ChartValue::ChartValue(const time_duration& value)
    : m_value(value.total_milliseconds()) {}

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
  return FromRepresentation(m_value + rhs.m_value);
}

ChartValue& ChartValue::operator +=(ChartValue rhs) {
  m_value += rhs.m_value;
  return *this;
}

ChartValue ChartValue::operator -(ChartValue rhs) const {
  return FromRepresentation(m_value - rhs.m_value);
}

ChartValue ChartValue::operator %(ChartValue rhs) const {
  return ChartValue::FromRepresentation(m_value % rhs.m_value);
}

double ChartValue::operator /(ChartValue rhs) const {
  return m_value / static_cast<double>(rhs.m_value);
}

ChartValue& ChartValue::operator -=(ChartValue rhs) {
  m_value -= rhs.m_value;
  return *this;
}

ChartValue ChartValue::operator -() const {
  return FromRepresentation(-m_value);
}

ptime ChartValue::ToDateTime() const {
  static const ptime BASE(date(1970, Jan, 1), seconds(0));
  return BASE + milliseconds(m_value);
}

Money ChartValue::ToMoney() const {
  return Money::FromRepresentation(m_value);
}

Quantity ChartValue::ToQuantity() const {
  return m_value;
}

time_duration ChartValue::ToTimeDuration() const {
  return milliseconds(m_value);
}

std::int64_t ChartValue::GetRepresentation() const {
  return m_value;
}
