#ifndef NEXUS_QUANTITY_HPP
#define NEXUS_QUANTITY_HPP
#include <cstdint>
#include <cstdlib>
#include <istream>
#include <limits>
#include <ostream>
#include <string>
#include <type_traits>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Utilities/Math.hpp>
#include <boost/cstdfloat.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  /** Used to represent a quantity up to 15 significant decimal places. */
  class Quantity {
    public:

      /** The number of decimal places that can be represented accurately. */
      static constexpr auto DECIMAL_PLACES = 6;

      /** The multiplier used. */
      static constexpr auto MULTIPLIER = std::int64_t(1000000);

      /**
       * Returns a Quantity from a string.
       * @param value The value to represent.
       * @return A Quantity representing the specified <i>value</i>.
       */
      static boost::optional<Quantity> FromValue(const std::string& value);

      /** Constructs a Quantity with a value of 0. */
      constexpr Quantity();

      /** Constructs a Quantity from an int32. */
      constexpr Quantity(std::int32_t value);

      /** Constructs a Quantity from a uint32. */
      constexpr Quantity(std::uint32_t value);

      /** Constructs a Quantity from an int64. */
      constexpr Quantity(std::int64_t value);

      /** Constructs a Quantity from a uint64. */
      constexpr Quantity(std::uint64_t value);

      /** Constructs a Quantity from a double. */
      constexpr Quantity(double value);

      /** Returns a Quantity from its raw representation. */
      static constexpr  Quantity FromRepresentation(boost::float64_t value);

      /** Converts this Quantity into a float. */
      explicit constexpr operator boost::float64_t() const;

      /** Converts this Quantity into an int. */
      explicit constexpr operator int() const;

      /** Converts this Quantity into an unsigned int. */
      explicit constexpr operator unsigned int() const;

      /** Converts this Quantity into a long. */
      explicit constexpr operator long() const;

      /** Converts this Quantity into an unsigned long. */
      explicit constexpr operator unsigned long() const;

      /** Converts this Quantity into a long long. */
      explicit constexpr operator long long() const;

      /** Converts this Quantity into an unsigned long long. */
      explicit constexpr operator unsigned long long() const;

      /**
       * Adds two Quantities together.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i> + <i>rhs</i>.
       */
      constexpr Quantity operator +(Quantity rhs) const;

      /**
       * Increases this Quantity.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i>.
       */
      constexpr Quantity& operator +=(Quantity rhs);

      /** Increments this Quantity. */
      constexpr Quantity& operator ++();

      /** Increments this Quantity. */
      constexpr Quantity operator ++(int);

      /**
       * Subtracts two Quantities together.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i> - <i>rhs</i>.
       */
      constexpr Quantity operator -(Quantity rhs) const;

      /**
       * Decreases this Quantity.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i>.
       */
      constexpr Quantity& operator -=(Quantity rhs);

      /** Decrements this Quantity. */
      constexpr Quantity& operator --();

      /** Decrements this Quantity. */
      constexpr Quantity operator --(int);

      /**
       * Multiplies two Quantities together.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i> * <i>rhs</i>.
       */
      constexpr Quantity operator *(Quantity rhs) const;

      /**
       * Multiplies this Quantity.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i>.
       */
      constexpr Quantity& operator *=(Quantity rhs);

      /**
       * Divides two Quantities together.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i> / <i>rhs</i>.
       */
      constexpr Quantity operator /(Quantity rhs) const;

      /**
       * Divides this Quantity.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i>.
       */
      constexpr Quantity& operator /=(Quantity rhs);

      /**
       * Returns the unary negation of this Quantity.
       * @return -<i>this</i>.
       */
      constexpr Quantity operator -() const;

      /** Returns the raw representation of this Quantity. */
      constexpr boost::float64_t GetRepresentation() const;

      constexpr auto operator<=>(const Quantity&) const = default;

    private:
      template<typename, typename> friend struct Beam::Serialization::Send;
      template<typename, typename> friend struct Beam::Serialization::Receive;
      friend std::ostream& operator <<(std::ostream& out, Quantity value);
      friend Quantity operator %(Quantity lhs, Quantity rhs);
      friend Quantity Abs(Quantity);
      friend class std::numeric_limits<Nexus::Quantity>;
      boost::float64_t m_value;
  };

  inline std::ostream& operator <<(std::ostream& out, Quantity value) {
    auto unscaledValue = value.m_value / Quantity::MULTIPLIER;
    auto integerPart = boost::float64_t();
    auto fraction = std::modf(unscaledValue, &integerPart);
    if(fraction == 0) {
      return out << static_cast<std::int64_t>(integerPart);
    }
    auto ifs = boost::io::ios_flags_saver(out);
    out << std::fixed;
    out << unscaledValue;
    return out;
  }

  inline std::istream& operator >>(std::istream& in, Quantity& value) {
    auto symbol = std::string();
    in >> symbol;
    auto parsedValue = Quantity::FromValue(symbol);
    if(!parsedValue.is_initialized()) {
      in.setstate(std::ios::failbit);
      return in;
    }
    value = *parsedValue;
    return in;
  }

  template<typename T, typename U>
  constexpr std::enable_if_t<std::is_convertible<T, Quantity>::value &&
      std::is_same<U, Quantity>::value, bool> operator <(T lhs, U rhs) {
    return Quantity(lhs) < rhs;
  }

  template<typename T, typename U>
  constexpr std::enable_if_t<std::is_convertible<T, Quantity>::value &&
      std::is_same<U, Quantity>::value, bool> operator <=(T lhs, U rhs) {
    return Quantity(lhs) <= rhs;
  }

  template<typename T, typename U>
  constexpr std::enable_if_t<std::is_convertible_v<T, Quantity>, bool>
      operator ==(T lhs, Quantity rhs) {
    return Quantity(lhs) == rhs;
  }

  template<typename T, typename U>
  constexpr std::enable_if_t<std::is_convertible<T, Quantity>::value &&
      std::is_same<U, Quantity>::value, bool> operator !=(T lhs, U rhs) {
    return Quantity(lhs) != rhs;
  }

  template<typename T, typename U>
  constexpr std::enable_if_t<std::is_convertible<T, Quantity>::value &&
      std::is_same<U, Quantity>::value, bool> operator >(T lhs, U rhs) {
    return Quantity(lhs) > rhs;
  }

  template<typename T, typename U>
  constexpr std::enable_if_t<std::is_convertible<T, Quantity>::value &&
      std::is_same<U, Quantity>::value, bool> operator >=(T lhs, U rhs) {
    return Quantity(lhs) >= rhs;
  }

  template<typename T, typename U>
  constexpr std::enable_if_t<std::is_convertible<T, Quantity>::value &&
      std::is_same<U, Quantity>::value, Quantity> operator +(T lhs, U rhs) {
    return Quantity(lhs) + rhs;
  }

  template<typename T, typename U>
  constexpr std::enable_if_t<std::is_convertible<T, Quantity>::value &&
      std::is_same<U, Quantity>::value, Quantity> operator -(T lhs, U rhs) {
    return Quantity(lhs) - rhs;
  }

  template<typename T, typename U>
  constexpr std::enable_if_t<std::is_convertible<T, Quantity>::value &&
      std::is_same<U, Quantity>::value, Quantity> operator *(T lhs, U rhs) {
    return Quantity(lhs) * rhs;
  }

  template<typename T>
  constexpr Quantity operator *(const boost::rational<T>& lhs, Quantity rhs) {
    return (lhs.numerator() * rhs) / lhs.denominator();
  }

  template<typename T, typename U>
  constexpr std::enable_if_t<std::is_convertible<T, Quantity>::value &&
      std::is_same<U, Quantity>::value, Quantity> operator /(T lhs, U rhs) {
    return Quantity(lhs) / rhs;
  }

  /**
   * Returns the modulus of two Quantities.
   * @param lhs The left hand side.
   * @param rhs The right hand side.
   * @return <i>lhs</i> % <i>rhs</i>
   */
  inline Quantity operator %(Quantity lhs, Quantity rhs) {
    return Quantity::FromRepresentation(std::fmod(lhs.m_value, rhs.m_value));
  }

  /**
   * Returns the absolute value.
   * @param value The value.
   */
  inline Quantity Abs(Quantity value) {
    return Quantity::FromRepresentation(std::abs(value.m_value));
  }

  /**
   * Returns the floor.
   * @param value The value to floor.
   * @param decimalPlaces The decimal place to floor to.
   */
  inline Quantity Floor(Quantity value, int decimalPlaces) {
    if(decimalPlaces > 0) {
      auto multiplier = Beam::PowerOfTen(decimalPlaces);
      auto remainder = value % (Quantity(1) / multiplier);
      if(value > 0 || remainder == 0) {
        return value - remainder;
      } else {
        return value - ((Quantity(1) / multiplier) + remainder);
      }
    } else {
      auto multiplier = Beam::PowerOfTen(-decimalPlaces);
      auto remainder = value % multiplier;
      if(value > 0 || remainder == 0) {
        return value - remainder;
      } else {
        return value - (multiplier + remainder);
      }
    }
  }

  /**
   * Returns the ceiling.
   * @param value The value to ceil.
   * @param decimalPlaces The decimal place to ceil to.
   */
  inline Quantity Ceil(Quantity value, int decimalPlaces) {
    return -Floor(-value, decimalPlaces);
  }

  /**
   * Returns the truncated value.
   * @param value The value to truncate.
   * @param decimalPlaces The decimal place to truncate.
   */
  inline Quantity Truncate(Quantity value, int decimalPlaces) {
    if(value < 0) {
      return Ceil(value, decimalPlaces);
    } else {
      return Floor(value, decimalPlaces);
    }
  }

  /**
   * Returns the rounded value.
   * @param value The value to round.
   * @param decimalPlaces The decimal place to round to.
   */
  inline Quantity Round(Quantity value, int decimalPlaces) {
    if(decimalPlaces >= 0) {
      auto multiplier = Beam::PowerOfTen(decimalPlaces + 1);
      return Floor(value + Quantity(5) / multiplier, decimalPlaces);
    } else {
      auto multiplier = Beam::PowerOfTen(-(decimalPlaces + 1));
      return Floor(value + Quantity(5) * multiplier, decimalPlaces);
    }
  }

  inline boost::optional<Quantity> Quantity::FromValue(
      const std::string& value) {
    if(value.empty()) {
      return boost::none;
    }
    auto i = value.begin();
    auto sign = std::int64_t();
    if(*i == '-') {
      sign = -1;
      ++i;
    } else {
      sign = 1;
      if(*i == '+') {
        ++i;
      }
    }
    auto leftHand = std::int64_t(0);
    auto hasDecimals = false;
    while(i != value.end()) {
      if(*i >= '0' && *i <= '9') {
        leftHand *= 10;
        leftHand += *i - '0';
        ++i;
      } else if(*i == '.') {
        hasDecimals = true;
        ++i;
        break;
      } else {
        return boost::none;
      }
    }
    auto exponent = std::int64_t(0);
    auto rightHand = std::int64_t(0);
    if(hasDecimals) {
      auto q = value.end() - 1;
      while(*q == '0') {
        --q;
      }
      while(i <= q) {
        if(*i >= '0' && *i <= '9') {
          rightHand *= 10;
          rightHand += *i - '0';
          --exponent;
          ++i;
        } else {
          return boost::none;
        }
      }
    }
    auto lhs = MULTIPLIER * static_cast<boost::float64_t>(leftHand);
    auto rhs = MULTIPLIER * static_cast<boost::float64_t>(rightHand);
    while(exponent != 0) {
      rhs /= 10;
      ++exponent;
    }
    return Quantity::FromRepresentation(sign * (lhs + rhs));
  }

  inline constexpr Quantity::Quantity()
    : m_value(0) {}

  inline constexpr Quantity::Quantity(std::int32_t value)
    : m_value(static_cast<boost::float64_t>(MULTIPLIER * value)) {}

  inline constexpr Quantity::Quantity(std::uint32_t value)
    : m_value(static_cast<boost::float64_t>(MULTIPLIER * value)) {}

  inline constexpr Quantity::Quantity(std::int64_t value)
    : m_value(static_cast<boost::float64_t>(MULTIPLIER * value)) {}

  inline constexpr Quantity::Quantity(std::uint64_t value)
    : m_value(static_cast<boost::float64_t>(MULTIPLIER * value)) {}

  inline constexpr Quantity::Quantity(double value)
    : m_value(static_cast<boost::float64_t>(MULTIPLIER * value)) {}

  inline constexpr Quantity Quantity::FromRepresentation(
      boost::float64_t value) {
    auto q = Quantity();
    q.m_value = value;
    return q;
  }

  inline constexpr Quantity::operator boost::float64_t() const {
    return m_value / MULTIPLIER;
  }

  inline constexpr Quantity::operator int() const {
    return static_cast<int>(m_value / MULTIPLIER);
  }

  inline constexpr Quantity::operator unsigned int() const {
    return static_cast<unsigned int>(m_value / MULTIPLIER);
  }

  inline constexpr Quantity::operator long() const {
    return static_cast<long>(m_value / MULTIPLIER);
  }

  inline constexpr Quantity::operator unsigned long() const {
    return static_cast<unsigned long>(m_value / MULTIPLIER);
  }

  inline constexpr Quantity::operator long long() const {
    return static_cast<long long>(m_value / MULTIPLIER);
  }

  inline constexpr Quantity::operator unsigned long long() const {
    return static_cast<unsigned long long>(m_value / MULTIPLIER);
  }

  inline constexpr Quantity Quantity::operator +(Quantity rhs) const {
    return FromRepresentation(m_value + rhs.m_value);
  }

  inline constexpr Quantity& Quantity::operator +=(Quantity rhs) {
    m_value += rhs.m_value;
    return *this;
  }

  inline constexpr Quantity& Quantity::operator ++() {
    m_value += MULTIPLIER;
    return *this;
  }

  inline constexpr Quantity Quantity::operator ++(int) {
    auto q = *this;
    ++(*this);
    return q;
  }

  inline constexpr Quantity Quantity::operator -(Quantity rhs) const {
    return FromRepresentation(m_value - rhs.m_value);
  }

  inline constexpr Quantity& Quantity::operator -=(Quantity rhs) {
    m_value -= rhs.m_value;
    return *this;
  }

  inline constexpr Quantity& Quantity::operator --() {
    m_value -= MULTIPLIER;
    return *this;
  }

  inline constexpr Quantity Quantity::operator --(int) {
    auto q = *this;
    --(*this);
    return q;
  }

  inline constexpr Quantity Quantity::operator *(Quantity rhs) const {
    return FromRepresentation(m_value * (rhs.m_value / MULTIPLIER));
  }

  inline constexpr Quantity& Quantity::operator *=(Quantity rhs) {
    m_value *= (rhs.m_value / MULTIPLIER);
    return *this;
  }

  inline constexpr Quantity Quantity::operator /(Quantity rhs) const {
    return FromRepresentation(MULTIPLIER * (m_value / rhs.m_value));
  }

  inline constexpr Quantity& Quantity::operator /=(Quantity rhs) {
    m_value = MULTIPLIER * (m_value / rhs.m_value);
    return *this;
  }

  inline constexpr Quantity Quantity::operator -() const {
    return FromRepresentation(-m_value);
  }

  inline constexpr boost::float64_t Quantity::GetRepresentation() const {
    return m_value;
  }
}

namespace Beam::Serialization {
  template<>
  struct IsStructure<Nexus::Quantity> : std::false_type {};

  template<>
  struct Send<Nexus::Quantity> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const Nexus::Quantity& value) const {
      shuttle.Send(name, value.m_value);
    }
  };

  template<>
  struct Receive<Nexus::Quantity> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::Quantity& value) const {
      auto representation = boost::float64_t();
      shuttle.Shuttle(name, representation);
      value = Nexus::Quantity::FromRepresentation(representation);
    }
  };
}

namespace std {
  template<>
  class numeric_limits<Nexus::Quantity> {
    public:
      static constexpr bool is_specialized = true;
      static constexpr bool is_signed =
        numeric_limits<boost::float64_t>::is_signed;
      static constexpr bool is_integer =
        numeric_limits<boost::float64_t>::is_integer;
      static constexpr bool is_exact =
        numeric_limits<boost::float64_t>::is_exact;
      static constexpr bool has_infinity =
        numeric_limits<boost::float64_t>::has_infinity;
      static constexpr bool has_quiet_NaN =
        numeric_limits<boost::float64_t>::has_quiet_NaN;
      static constexpr bool has_signaling_NaN =
        numeric_limits<boost::float64_t>::has_signaling_NaN;
      static constexpr bool has_denorm =
        numeric_limits<boost::float64_t>::has_denorm;
      static constexpr bool has_denorm_loss =
        numeric_limits<boost::float64_t>::has_denorm_loss;
      static constexpr std::float_round_style round_style =
        numeric_limits<boost::float64_t>::round_style;
      static constexpr bool is_iec559 =
        numeric_limits<boost::float64_t>::is_iec559;
      static constexpr bool is_bounded =
        numeric_limits<boost::float64_t>::is_bounded;
      static constexpr bool is_modulo =
        numeric_limits<boost::float64_t>::is_modulo;
      static constexpr int digits = numeric_limits<boost::float64_t>::digits;
      static constexpr int digits10 =
        numeric_limits<boost::float64_t>::digits10;
      static constexpr int max_digits10 =
        numeric_limits<boost::float64_t>::max_digits10;
      static constexpr int radix = numeric_limits<boost::float64_t>::radix;
      static constexpr int min_exponent =
        numeric_limits<boost::float64_t>::min_exponent;
      static constexpr int min_exponent10 =
        numeric_limits<boost::float64_t>::min_exponent10;
      static constexpr int max_exponent =
        numeric_limits<boost::float64_t>::max_exponent;
      static constexpr int max_exponent10 =
        numeric_limits<boost::float64_t>::max_exponent10;
      static constexpr bool traps =
        numeric_limits<boost::float64_t>::traps;
      static constexpr bool tinyness_before =
        numeric_limits<boost::float64_t>::tinyness_before;

      static constexpr Nexus::Quantity min() {
        return Nexus::Quantity::FromRepresentation(
          numeric_limits<boost::float64_t>::min());
      }

      static constexpr Nexus::Quantity lowest() {
        return Nexus::Quantity::FromRepresentation(
          numeric_limits<boost::float64_t>::lowest());
      }

      static constexpr Nexus::Quantity max() {
        return Nexus::Quantity::FromRepresentation(
          numeric_limits<boost::float64_t>::max());
      }

      static constexpr Nexus::Quantity epsilon() {
        return Nexus::Quantity::FromRepresentation(
          numeric_limits<boost::float64_t>::epsilon());
      }

      static constexpr Nexus::Quantity round_error() {
        return Nexus::Quantity::FromRepresentation(
          numeric_limits<boost::float64_t>::round_error());
      }

      static constexpr Nexus::Quantity infinity() {
        return Nexus::Quantity::FromRepresentation(
          numeric_limits<boost::float64_t>::infinity());
      }

      static constexpr Nexus::Quantity quiet_NaN() {
        return Nexus::Quantity::FromRepresentation(
          numeric_limits<boost::float64_t>::quiet_NaN());
      }

      static constexpr Nexus::Quantity signaling_NaN() {
        return Nexus::Quantity::FromRepresentation(
          numeric_limits<boost::float64_t>::signaling_NaN());
      }

      static constexpr Nexus::Quantity denorm_min() {
        return Nexus::Quantity::FromRepresentation(
          numeric_limits<boost::float64_t>::denorm_min());
      }
  };
}

#endif
