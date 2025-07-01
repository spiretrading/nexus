#ifndef NEXUS_MONEY_HPP
#define NEXUS_MONEY_HPP
#include <functional>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string_view>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Quantity.hpp"

namespace Nexus {

  /** Used to represent money. */
  class Money {
    public:

      /** Stores a value of 0.00. */
      static const Money ZERO;

      /** Stores a value of 1.00. */
      static const Money ONE;

      /** Stores a value of 0.01. */
      static const Money CENT;

      /** Stores a value of 0.0001. */
      static const Money BIP;

      /** Constructs a Money value of ZERO. */
      constexpr Money() = default;

      /**
       * Constructs a Money value.
       * @param value The value to represent.
       */
      explicit constexpr Money(Quantity value) noexcept;

      /** Converts this Money to a float. */
      explicit constexpr operator boost::float64_t() const;

      /** Converts this Money to a Quantity. */
      explicit constexpr operator Quantity() const;

      /**
       * Adds two Money instances together.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i> + <i>rhs</i>.
       */
      constexpr Money operator +(Money rhs) const;

      /**
       * Increases this Money instance.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i>.
       */
      constexpr Money& operator +=(Money rhs);

      /**
       * Subtracts two Money instances together.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i> - <i>rhs</i>.
       */
      constexpr Money operator -(Money rhs) const;

      /**
       * Decreases this Money instance.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i>.
       */
      constexpr Money& operator -=(Money rhs);

      /**
       * Divides two Money instances together.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i> / <i>rhs</i>.
       */
      constexpr double operator /(Money rhs) const;

      /**
       * Multiplies this Money instance.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i>.
       */
      template<typename T>
      constexpr Money& operator *=(T rhs);

      /**
       * Divides this Money instance.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i>.
       */
      template<typename T>
      constexpr Money& operator /=(T rhs);

      /**
       * Returns the unary negation of this Money instance.
       * @return -<i>this</i>.
       */
      constexpr Money operator -() const;

      auto operator <=>(const Money& rhs) const = default;

    private:
      Quantity m_value;
  };

  /**
   * Attempts to parse a monetary amount.
   * @param value The input string representing a money amount.
   * @return The parsed Money if successful, or boost::none on failure.
   */
  inline boost::optional<Money> try_parse_money(std::string_view value) {
    if(auto quantity = try_parse_quantity(value)) {
      return Money(*quantity);
    }
    return boost::none;
  }

  /**
   * Parses a monetary amount.
   * @param value The input string representing a money amount.
   * @return The parsed Money object.
   * @throws std::invalid_argument If the input cannot be parsed as Money.
   */
  inline Money parse_money(std::string_view value) {
    if(auto money = try_parse_money(value)) {
      return *money;
    }
    throw std::invalid_argument("Invalid Money value given.");
  }

  /**
   * Returns the modulus of two Money objects.
   * @param lhs The left hand side.
   * @param rhs The right hand side.
   * @return <i>lhs</i> % <i>rhs</i>
   */
  inline Money operator %(Money lhs, Money rhs) {
    return Money(static_cast<Quantity>(lhs) % static_cast<Quantity>(rhs));
  }

  /**
   * Returns the modulus of two Money objects.
   * @param lhs The left hand side.
   * @param rhs The right hand side.
   * @return <i>lhs</i> % <i>rhs</i>
   */
  inline Money fmod(Money lhs, Money rhs) {
    return Money(static_cast<Quantity>(lhs) % static_cast<Quantity>(rhs));
  }

  /**
   * Multiplies a Money instance by a scalar quantity.
   * @param lhs The scalar quantity.
   * @param rhs The Money instance to be multiplied.
   * @return <i>lhs</i> * <i>rhs</i>.
   */
  template<typename T>
  constexpr std::enable_if_t<
    std::is_same_v<decltype(std::declval<T>() * std::declval<Quantity>()),
      Quantity>, Money> operator *(T lhs, Money rhs) {
    return Money(lhs * static_cast<Quantity>(rhs));
  }

  /**
   * Divides a Money instance by a scalar quantity.
   * @param lhs The Money instance to be divided.
   * @param rhs The scalar quantity.
   * @return <i>lhs</i> / <i>rhs</i>.
   */
  template<typename T>
  constexpr std::enable_if_t<
    std::is_same_v<decltype(std::declval<Quantity>() / std::declval<T>()),
      Quantity>, Money> operator /(Money lhs, T rhs) {
    return Money(static_cast<Quantity>(lhs) / rhs);
  }

  /**
   * Returns the absolute value.
   * @param value The value.
   */
  inline Money abs(Money value) {
    return Money(abs(static_cast<Quantity>(value)));
  }

  /**
   * Returns the floor.
   * @param value The value to floor.
   * @param decimal_places The decimal place to floor to.
   */
  inline Money floor(Money value, int decimal_places) {
    return Money(floor(static_cast<Quantity>(value), decimal_places));
  }

  /**
   * Returns the ceiling.
   * @param value The value to ceil.
   * @param decimal_places The decimal place to ceil to.
   */
  inline Money ceil(Money value, int decimal_places) {
    return Money(ceil(static_cast<Quantity>(value), decimal_places));
  }

  /**
   * Returns the truncated value.
   * @param value The value to truncate.
   * @param decimal_places The decimal place to truncate.
   */
  inline Money truncate(Money value, int decimal_places) {
    return Money(truncate(static_cast<Quantity>(value), decimal_places));
  }

  /**
   * Returns the rounded value.
   * @param value The value to round.
   * @param decimal_places The decimal place to round to.
   */
  inline Money round(Money value, int decimal_places) {
    return Money(round(static_cast<Quantity>(value), decimal_places));
  }

  inline std::size_t hash_value(Money money) noexcept {
    return std::hash<Quantity>()(static_cast<Quantity>(money));
  }

  inline std::ostream& operator <<(std::ostream& out, Money value) {
    auto fraction =
      static_cast<Quantity>(value) - floor(static_cast<Quantity>(value), 0);
    if(fraction == 0) {
      return out << static_cast<Quantity>(value) << ".00";
    }
    auto s = boost::lexical_cast<std::string>(static_cast<Quantity>(value));
    if(s.size() > 1 && *(s.end() - 1) == '.') {
      return out << s << "00";
    } else if(s.size() > 2 && *(s.end() - 2) == '.') {
      return out << s << "0";
    } else {
      while(s.size() > 3) {
        if(s.back() == '0' && *(s.end() - 3) != '.') {
          s.pop_back();
        } else {
          break;
        }
      }
      return out << s;
    }
  }

  inline std::istream& operator >>(std::istream& in, Money& value) {
    auto symbol = std::string();
    in >> symbol;
    if(auto parsed_value = try_parse_money(symbol)) {
      value = *parsed_value;
      return in;
    }
    in.setstate(std::ios::failbit);
    return in;
  }

  inline constexpr Money::Money(Quantity value) noexcept
    : m_value(value) {}

  inline constexpr Money::operator boost::float64_t() const {
    return static_cast<boost::float64_t>(m_value);
  }

  inline constexpr Money::operator Quantity() const {
    return m_value;
  }

  inline constexpr Money Money::operator +(Money rhs) const {
    return Money(m_value + rhs.m_value);
  }

  inline constexpr Money& Money::operator +=(Money rhs) {
    m_value += rhs.m_value;
    return *this;
  }

  inline constexpr Money Money::operator -(Money rhs) const {
    return Money(m_value - rhs.m_value);
  }

  inline constexpr Money& Money::operator -=(Money rhs) {
    m_value -= rhs.m_value;
    return *this;
  }

  inline constexpr double Money::operator /(Money rhs) const {
    return static_cast<double>(m_value / rhs.m_value);
  }

  template<typename T>
  constexpr Money& Money::operator *=(T rhs) {
    m_value *= rhs;
    return *this;
  }

  template<typename T>
  constexpr Money& Money::operator /=(T rhs) {
    m_value /= rhs;
    return *this;
  }

  inline constexpr Money Money::operator -() const {
    return Money(-m_value);
  }

  inline const Money Money::ZERO(0);
  inline const Money Money::ONE(1);
  inline const Money Money::CENT(Money(1) / 100);
  inline const Money Money::BIP(Money(1) / 10000);
}

namespace Beam::Serialization {
  template<>
  struct IsStructure<Nexus::Money> : std::false_type {};

  template<>
  struct Send<Nexus::Money> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, const char* name, Nexus::Money value) const {
      shuttle.Send(name, static_cast<Nexus::Quantity>(value));
    }
  };

  template<>
  struct Receive<Nexus::Money> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::Money& value) const {
      auto representation = Nexus::Quantity();
      shuttle.Shuttle(name, representation);
      value = Nexus::Money(representation);
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::Money> {
    std::size_t operator ()(Nexus::Money value) const noexcept {
      return Nexus::hash_value(value);
    }
  };

  template<>
  class numeric_limits<Nexus::Money> {
    public:
      static constexpr bool is_specialized = true;
      static constexpr bool is_signed =
        numeric_limits<Nexus::Quantity>::is_signed;
      static constexpr bool is_integer =
        numeric_limits<Nexus::Quantity>::is_integer;
      static constexpr bool is_exact =
        numeric_limits<Nexus::Quantity>::is_exact;
      static constexpr bool has_infinity =
        numeric_limits<Nexus::Quantity>::has_infinity;
      static constexpr bool has_quiet_NaN =
        numeric_limits<Nexus::Quantity>::has_quiet_NaN;
      static constexpr bool has_signaling_NaN =
        numeric_limits<Nexus::Quantity>::has_signaling_NaN;
      static constexpr bool has_denorm =
        numeric_limits<Nexus::Quantity>::has_denorm;
      static constexpr bool has_denorm_loss =
        numeric_limits<Nexus::Quantity>::has_denorm_loss;
      static constexpr std::float_round_style round_style =
        numeric_limits<Nexus::Quantity>::round_style;
      static constexpr bool is_iec559 =
        numeric_limits<Nexus::Quantity>::is_iec559;
      static constexpr bool is_bounded =
        numeric_limits<Nexus::Quantity>::is_bounded;
      static constexpr bool is_modulo =
        numeric_limits<Nexus::Quantity>::is_modulo;
      static constexpr int digits = numeric_limits<Nexus::Quantity>::digits;
      static constexpr int digits10 =
        numeric_limits<Nexus::Quantity>::digits10;
      static constexpr int max_digits10 =
        numeric_limits<Nexus::Quantity>::max_digits10;
      static constexpr int radix =
        numeric_limits<Nexus::Quantity>::radix;
      static constexpr int min_exponent =
        numeric_limits<Nexus::Quantity>::min_exponent;
      static constexpr int min_exponent10 =
        numeric_limits<Nexus::Quantity>::min_exponent10;
      static constexpr int max_exponent =
        numeric_limits<Nexus::Quantity>::max_exponent;
      static constexpr int max_exponent10 =
        numeric_limits<Nexus::Quantity>::max_exponent10;
      static constexpr bool traps =
        numeric_limits<Nexus::Quantity>::traps;
      static constexpr bool tinyness_before =
        numeric_limits<Nexus::Quantity>::tinyness_before;

      static constexpr Nexus::Money min() {
        return Nexus::Money(numeric_limits<Nexus::Quantity>::min());
      }

      static constexpr Nexus::Money lowest() {
        return Nexus::Money(numeric_limits<Nexus::Quantity>::lowest());
      }

      static constexpr Nexus::Money max() {
        return Nexus::Money(numeric_limits<Nexus::Quantity>::max());
      }

      static constexpr Nexus::Money epsilon() {
        return Nexus::Money(numeric_limits<Nexus::Quantity>::epsilon());
      }

      static constexpr Nexus::Money round_error() {
        return Nexus::Money(numeric_limits<Nexus::Quantity>::round_error());
      }

      static constexpr Nexus::Money infinity() {
        return Nexus::Money(numeric_limits<Nexus::Quantity>::infinity());
      }

      static constexpr Nexus::Money quiet_NaN() {
        return Nexus::Money(numeric_limits<Nexus::Quantity>::quiet_NaN());
      }

      static constexpr Nexus::Money signaling_NaN() {
        return Nexus::Money(numeric_limits<Nexus::Quantity>::signaling_NaN());
      }

      static constexpr Nexus::Money denorm_min() {
        return Nexus::Money(numeric_limits<Nexus::Quantity>::denorm_min());
      }
  };
}

#endif
