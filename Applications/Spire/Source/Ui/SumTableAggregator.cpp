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

template<std::floating_point T>
T Details::RealSum::get() const {
  if(m_nan != 0 || (m_positive_infinity != 0 && m_negative_infinity != 0)) {
    return std::numeric_limits<T>::quiet_NaN();
  } else if(m_positive_infinity != 0) {
    return std::numeric_limits<T>::infinity();
  } else if(m_negative_infinity != 0) {
    return -std::numeric_limits<T>::infinity();
  }
  return static_cast<T>(ldexp(cpp_bin_float_50(m_significand), m_exponent));
}

template float Details::RealSum::get<float>() const;
template double Details::RealSum::get<double>() const;
template long double Details::RealSum::get<long double>() const;
