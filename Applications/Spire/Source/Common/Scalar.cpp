#include "Spire/Spire/Scalar.hpp"
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto BASE = ptime(date(1970, Jan, 1), seconds(0));
}

Scalar::Scalar(Money value) noexcept
  : m_value(static_cast<Quantity>(value)) {}

Scalar::Scalar(Quantity value) noexcept
  : m_value(value) {}

Scalar::Scalar(ptime value) noexcept {
  if(value == not_a_date_time) {
    m_value = std::numeric_limits<Quantity>::quiet_NaN();
  } else if(value == pos_infin) {
    m_value = std::numeric_limits<Quantity>::infinity();
  } else if(value == neg_infin) {
    m_value = -std::numeric_limits<Quantity>::infinity();
  } else {
    auto delta = value - BASE;
    m_value = delta.total_milliseconds();
  }
}

Scalar::Scalar(time_duration value) noexcept
  : m_value(value.total_milliseconds()) {}

Scalar::operator ptime() const {
  return BASE + milliseconds(static_cast<std::int64_t>(m_value));
}

Scalar::operator Money() const {
  return Money(m_value);
}

Scalar::operator Quantity() const {
  return m_value;
}

Scalar::operator time_duration() const {
  return milliseconds(static_cast<std::int64_t>(m_value));
}

Scalar Scalar::operator +(Scalar rhs) const {
  return Scalar(m_value + rhs.m_value);
}

Scalar& Scalar::operator +=(Scalar rhs) {
  m_value += rhs.m_value;
  return *this;
}

Scalar Scalar::operator -(Scalar rhs) const {
  return Scalar(m_value - rhs.m_value);
}

Scalar Scalar::operator %(Scalar rhs) const {
  return Scalar(m_value % rhs.m_value);
}

double Scalar::operator /(Scalar rhs) const {
  return static_cast<double>(m_value / rhs.m_value);
}

Scalar& Scalar::operator -=(Scalar rhs) {
  m_value -= rhs.m_value;
  return *this;
}

Scalar Scalar::operator -() const {
  return Scalar(-m_value);
}
