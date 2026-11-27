#include "Spire/Ui/SumTableAggregator.hpp"
#include <cmath>
#include <limits>
#include <boost/multiprecision/cpp_bin_float.hpp>

using namespace boost::multiprecision;
using namespace Spire;

void Details::RealSum::apply(long double value, int count) {
  if(std::isnan(value)) {
    m_nan += count;
  } else if(std::isinf(value)) {
    if(value > 0) {
      m_positive_infinity += count;
    } else {
      m_negative_infinity += count;
    }
  } else if(value != 0) {
    constexpr auto DIGITS = std::numeric_limits<long double>::digits;
    auto exponent = 0;
    auto fraction = std::frexp(value * count, &exponent);
    auto significand = cpp_int(std::ldexp(fraction, DIGITS));
    exponent -= DIGITS;
    if(m_significand == 0) {
      m_exponent = exponent;
    } else if(exponent < m_exponent) {
      m_significand <<= m_exponent - exponent;
      m_exponent = exponent;
    } else {
      significand <<= exponent - m_exponent;
    }
    m_significand += significand;
  }
}

double Details::RealSum::get() const {
  if(m_nan != 0 || (m_positive_infinity != 0 && m_negative_infinity != 0)) {
    return std::numeric_limits<double>::quiet_NaN();
  } else if(m_positive_infinity != 0) {
    return std::numeric_limits<double>::infinity();
  } else if(m_negative_infinity != 0) {
    return -std::numeric_limits<double>::infinity();
  }
  return static_cast<double>(
    ldexp(cpp_bin_float_50(m_significand), m_exponent));
}

std::any Details::integer_sum(const int128_t& value) {
  if(value >= std::numeric_limits<std::int64_t>::min() &&
      value <= std::numeric_limits<std::int64_t>::max()) {
    return value.convert_to<std::int64_t>();
  } else if(value >= 0 && value <= std::numeric_limits<std::uint64_t>::max()) {
    return value.convert_to<std::uint64_t>();
  }
  return value;
}
