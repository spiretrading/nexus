#ifndef NEXUS_DEFINITIONS_QUANTITY_HPP
#define NEXUS_DEFINITIONS_QUANTITY_HPP
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <istream>
#include <limits>
#include <ostream>
#include <string>
#include <type_traits>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <boost/cstdfloat.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/rational.hpp>
#include <boost/throw_exception.hpp>

namespace Nexus {

  /** Used to represent a quantity up to 15 significant decimal places. */
  class Quantity {
    public:

      /** The number of decimal places that can be represented accurately. */
      static constexpr auto DECIMAL_PLACES = 6;

      /** The multiplier used. */
      static constexpr auto MULTIPLIER = std::int64_t(1000000);

      /** Constructs a Quantity with a value of 0. */
      constexpr Quantity() noexcept;

      /** Constructs a Quantity from an int32. */
      constexpr Quantity(std::int32_t value) noexcept;

      /** Constructs a Quantity from a uint32. */
      constexpr Quantity(std::uint32_t value) noexcept;

      /** Constructs a Quantity from an int64. */
      constexpr Quantity(std::int64_t value) noexcept;

      /** Constructs a Quantity from a uint64. */
      constexpr Quantity(std::uint64_t value) noexcept;

      /** Constructs a Quantity from a double. */
      constexpr Quantity(double value) noexcept;

      /** Returns a Quantity from its raw representation. */
      static constexpr Quantity from_representation(boost::float64_t value);

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
      constexpr boost::float64_t get_representation() const;

      constexpr auto operator<=>(const Quantity&) const = default;

    private:
      boost::float64_t m_value;
  };

  /**
   * Attempts to parse a Quantity.
   * @param value The input string representing a quantity.
   * @return The parsed Quantity if successful, or boost::none on failure.
   */
  inline boost::optional<Quantity> try_parse_quantity(std::string_view value) {
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
    auto left_hand = std::int64_t(0);
    auto has_decimals = false;
    while(i != value.end()) {
      if(*i >= '0' && *i <= '9') {
        left_hand *= 10;
        left_hand += *i - '0';
        ++i;
      } else if(*i == '.') {
        has_decimals = true;
        ++i;
        break;
      } else {
        return boost::none;
      }
    }
    auto exponent = std::int64_t(0);
    auto right_hand = std::int64_t(0);
    if(has_decimals) {
      auto q = value.end() - 1;
      while(*q == '0') {
        --q;
      }
      while(i <= q) {
        if(*i >= '0' && *i <= '9') {
          right_hand *= 10;
          right_hand += *i - '0';
          --exponent;
          ++i;
        } else {
          return boost::none;
        }
      }
    }
    auto lhs = Quantity::MULTIPLIER * static_cast<boost::float64_t>(left_hand);
    auto rhs = Quantity::MULTIPLIER * static_cast<boost::float64_t>(right_hand);
    while(exponent != 0) {
      rhs /= 10;
      ++exponent;
    }
    return Quantity::from_representation(sign * (lhs + rhs));
  }

  /**
   * Parses a Quantity.
   * @param value The input string representing a quantity.
   * @return The parsed Quantity.
   * @throws std::invalid_argument If the input cannot be parsed.
   */
  inline Quantity parse_quantity(std::string_view value) {
    if(auto money = try_parse_quantity(value)) {
      return *money;
    }
    boost::throw_with_location(
      std::invalid_argument("Invalid Quantity value given."));
  }

  inline std::ostream& operator <<(std::ostream& out, Quantity value) {
    auto unscaled_value = value.get_representation() / Quantity::MULTIPLIER;
    auto integer_part = boost::float64_t();
    auto fraction = std::modf(unscaled_value, &integer_part);
    if(fraction == 0) {
      return out << static_cast<std::int64_t>(integer_part);
    }
    auto ifs = boost::io::ios_flags_saver(out);
    out << std::fixed;
    out << unscaled_value;
    return out;
  }

  inline std::istream& operator >>(std::istream& in, Quantity& value) {
    auto symbol = std::string();
    in >> symbol;
    if(auto parsed_value = try_parse_quantity(symbol)) {
      value = *parsed_value;
      return in;
    }
    in.setstate(std::ios::failbit);
    return in;
  }

  template<typename T, typename U> requires
    std::is_convertible_v<T, Quantity> && std::is_same_v<U, Quantity>
  constexpr bool operator <(T lhs, U rhs) {
    return Quantity(lhs) < rhs;
  }

  template<typename T, typename U> requires
    std::is_convertible_v<T, Quantity> && std::is_same_v<U, Quantity>
  constexpr bool operator <=(T lhs, U rhs) {
    return Quantity(lhs) <= rhs;
  }

  template<typename T, typename U> requires
    std::is_convertible_v<T, Quantity>
  constexpr bool operator ==(T lhs, Quantity rhs) {
    return Quantity(lhs) == rhs;
  }

  template<typename T, typename U> requires
    std::is_convertible_v<T, Quantity> && std::is_same_v<U, Quantity>
  constexpr bool operator !=(T lhs, U rhs) {
    return Quantity(lhs) != rhs;
  }

  template<typename T, typename U> requires
    std::is_convertible_v<T, Quantity> && std::is_same_v<U, Quantity>
  constexpr bool operator >(T lhs, U rhs) {
    return Quantity(lhs) > rhs;
  }

  template<typename T, typename U> requires
    std::is_convertible_v<T, Quantity> && std::is_same_v<U, Quantity>
  constexpr bool operator >=(T lhs, U rhs) {
    return Quantity(lhs) >= rhs;
  }

  template<typename T, typename U> requires
    std::is_convertible_v<T, Quantity> && std::is_same_v<U, Quantity>
  constexpr Quantity operator +(T lhs, U rhs) {
    return Quantity(lhs) + rhs;
  }

  template<typename T, typename U> requires
    std::is_convertible_v<T, Quantity> && std::is_same_v<U, Quantity>
  constexpr Quantity operator -(T lhs, U rhs) {
    return Quantity(lhs) - rhs;
  }

  template<typename T, typename U> requires
    std::is_convertible_v<T, Quantity> && std::is_same_v<U, Quantity>
  constexpr Quantity operator *(T lhs, U rhs) {
    return Quantity(lhs) * rhs;
  }

  template<typename T>
  constexpr Quantity operator *(const boost::rational<T>& lhs, Quantity rhs) {
    return (lhs.numerator() * rhs) / lhs.denominator();
  }

  template<typename T, typename U> requires
    std::is_convertible_v<T, Quantity> && std::is_same_v<U, Quantity>
  constexpr Quantity operator /(T lhs, U rhs) {
    return Quantity(lhs) / rhs;
  }

  /**
   * Returns the modulus of two Quantities.
   * @param lhs The left hand side.
   * @param rhs The right hand side.
   * @return <i>lhs</i> % <i>rhs</i>
   */
  inline Quantity operator %(Quantity lhs, Quantity rhs) {
    return Quantity::from_representation(
      std::fmod(lhs.get_representation(), rhs.get_representation()));
  }

  /**
   * Returns the modulus of two Quantities.
   * @param lhs The left hand side.
   * @param rhs The right hand side.
   * @return <i>lhs</i> % <i>rhs</i>
   */
  inline Quantity fmod(Quantity lhs, Quantity rhs) {
    return Quantity::from_representation(
      std::fmod(lhs.get_representation(), rhs.get_representation()));
  }

  /**
   * Returns the absolute value.
   * @param value The value.
   */
  inline Quantity abs(Quantity value) {
    return Quantity::from_representation(std::abs(value.get_representation()));
  }

  /**
   * Returns the floor.
   * @param value The value to floor.
   */
  inline Quantity floor(Quantity value) {
    return Quantity::from_representation(
      std::floor(value.get_representation() / Quantity::MULTIPLIER) *
        Quantity::MULTIPLIER);
  }

  /**
   * Returns the greatest multiple of a given value less than or equal to the
   * specified value.
   * @param value The value to floor.
   * @param multiple The multiple to floor to.
   * @return The greatest multiple of <i>multiple</i> less than or equal to
   *         <i>value</i>.
   */
  inline Quantity floor_to(Quantity value, Quantity multiple) {
    if(multiple == 0) {
      return Quantity(0);
    }
    return Quantity::from_representation(
      std::floor(value.get_representation() / multiple.get_representation()) *
        multiple.get_representation());
  }

  /**
   * Returns the ceiling.
   * @param value The value to ceil.
   */
  inline Quantity ceil(Quantity value) {
    return Quantity::from_representation(
      std::ceil(value.get_representation() / Quantity::MULTIPLIER) *
        Quantity::MULTIPLIER);
  }

  /**
   * Returns the smallest multiple of a given value greater than or equal to the
   * specified value.
   * @param value The value to ceil.
   * @param multiple The multiple to ceil to.
   * @return The smallest multiple of <i>multiple</i> greater than or equal to
   *         <i>value</i>.
   */
  inline Quantity ceil_to(Quantity value, Quantity multiple) {
    if(multiple == 0) {
      return Quantity(0);
    }
    return Quantity::from_representation(
      std::ceil(value.get_representation() / multiple.get_representation()) *
        multiple.get_representation());
  }

  /**
   * Returns the rounded value.
   * @param value The value to round.
   */
  inline Quantity round(Quantity value) {
    return Quantity::from_representation(
      std::round(value.get_representation() / Quantity::MULTIPLIER) *
        Quantity::MULTIPLIER);
  }

  /**
   * Returns the multiple of a given value nearest to the specified value.
   * @param value The value to round.
   * @param multiple The multiple to round to.
   * @return The multiple of <i>multiple</i> nearest to <i>value</i>.
   */
  inline Quantity round_to(Quantity value, Quantity multiple) {
    if(multiple == 0) {
      return Quantity(0);
    }
    return Quantity::from_representation(
      std::round(value.get_representation() / multiple.get_representation()) *
        multiple.get_representation());
  }

  /**
   * Returns the truncated value.
   * @param value The value to truncate.
   */
  inline Quantity truncate(Quantity value) {
    return Quantity::from_representation(
      std::trunc(value.get_representation() / Quantity::MULTIPLIER) *
        Quantity::MULTIPLIER);
  }

  /**
   * Returns the multiple of a given value closest to zero from the specified
   * value.
   * @param value The value to truncate.
   * @param multiple The multiple to truncate to.
   * @return The multiple of <i>multiple</i> closest to zero from <i>value</i>.
   */
  inline Quantity truncate_to(Quantity value, Quantity multiple) {
    if(value < 0) {
      return ceil_to(value, multiple);
    } else {
      return floor_to(value, multiple);
    }
  }

  inline constexpr Quantity::Quantity() noexcept
    : m_value(0) {}

  inline constexpr Quantity::Quantity(std::int32_t value) noexcept
    : m_value(static_cast<boost::float64_t>(MULTIPLIER * value)) {}

  inline constexpr Quantity::Quantity(std::uint32_t value) noexcept
    : m_value(static_cast<boost::float64_t>(MULTIPLIER * value)) {}

  inline constexpr Quantity::Quantity(std::int64_t value) noexcept
    : m_value(static_cast<boost::float64_t>(MULTIPLIER * value)) {}

  inline constexpr Quantity::Quantity(std::uint64_t value) noexcept
    : m_value(static_cast<boost::float64_t>(MULTIPLIER * value)) {}

  inline constexpr Quantity::Quantity(double value) noexcept
    : m_value(static_cast<boost::float64_t>(MULTIPLIER * value)) {}

  inline constexpr Quantity Quantity::from_representation(
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
    return from_representation(m_value + rhs.m_value);
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
    return from_representation(m_value - rhs.m_value);
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
    return from_representation(m_value * (rhs.m_value / MULTIPLIER));
  }

  inline constexpr Quantity& Quantity::operator *=(Quantity rhs) {
    m_value *= (rhs.m_value / MULTIPLIER);
    return *this;
  }

  inline constexpr Quantity Quantity::operator /(Quantity rhs) const {
    return from_representation(MULTIPLIER * (m_value / rhs.m_value));
  }

  inline constexpr Quantity& Quantity::operator /=(Quantity rhs) {
    m_value = MULTIPLIER * (m_value / rhs.m_value);
    return *this;
  }

  inline constexpr Quantity Quantity::operator -() const {
    return from_representation(-m_value);
  }

  inline constexpr boost::float64_t Quantity::get_representation() const {
    return m_value;
  }
}

namespace Beam {
  template<>
  constexpr auto is_structure<Nexus::Quantity> = false;

  template<>
  struct Send<Nexus::Quantity> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const Nexus::Quantity& value) const {
      sender.send(name, value.get_representation());
    }
  };

  template<>
  struct Receive<Nexus::Quantity> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, Nexus::Quantity& value) const {
      value = Nexus::Quantity::from_representation(
        receive<boost::float64_t>(receiver, name));
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::Quantity> {
    std::size_t operator ()(Nexus::Quantity value) const noexcept {
      auto source = static_cast<boost::float64_t>(value);
      return std::hash<boost::float64_t>()(source);
    }
  };

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
        return Nexus::Quantity::from_representation(
          numeric_limits<boost::float64_t>::min());
      }

      static constexpr Nexus::Quantity lowest() {
        return Nexus::Quantity::from_representation(
          numeric_limits<boost::float64_t>::lowest());
      }

      static constexpr Nexus::Quantity max() {
        return Nexus::Quantity::from_representation(
          numeric_limits<boost::float64_t>::max());
      }

      static constexpr Nexus::Quantity epsilon() {
        return Nexus::Quantity::from_representation(
          numeric_limits<boost::float64_t>::epsilon());
      }

      static constexpr Nexus::Quantity round_error() {
        return Nexus::Quantity::from_representation(
          numeric_limits<boost::float64_t>::round_error());
      }

      static constexpr Nexus::Quantity infinity() {
        return Nexus::Quantity::from_representation(
          numeric_limits<boost::float64_t>::infinity());
      }

      static constexpr Nexus::Quantity quiet_NaN() {
        return Nexus::Quantity::from_representation(
          numeric_limits<boost::float64_t>::quiet_NaN());
      }

      static constexpr Nexus::Quantity signaling_NaN() {
        return Nexus::Quantity::from_representation(
          numeric_limits<boost::float64_t>::signaling_NaN());
      }

      static constexpr Nexus::Quantity denorm_min() {
        return Nexus::Quantity::from_representation(
          numeric_limits<boost::float64_t>::denorm_min());
      }
  };
}

#endif
