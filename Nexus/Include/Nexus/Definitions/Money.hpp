#ifndef NEXUS_MONEY_HPP
#define NEXUS_MONEY_HPP
#include <functional>
#include <istream>
#include <ostream>
#include <string>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <boost/functional/hash.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Quantity.hpp"

namespace Nexus {
namespace Details {
  template<typename T>
  struct MoneyDefinitions {

    /** Stores a value of 0.00. */
    static const T ZERO;

    /** Stores a value of 1.00. */
    static const T ONE;

    /** Stores a value of 0.01. */
    static const T CENT;

    /** Stores a value of 0.0001. */
    static const T BIP;

    auto operator <=>(const MoneyDefinitions& rhs) const = default;
  };
}

  /** Used to represent money without rounding or floating point issues. */
  class Money : private Details::MoneyDefinitions<Money> {
    public:

      /**
       * Returns a Money value from a string.
       * @param value The value to represent.
       * @return A Money value representing the specified <i>value</i>.
       */
      static boost::optional<Money> FromValue(const std::string& value);

      /** Constructs a Money value of ZERO. */
      constexpr Money() = default;

      /**
       * Constructs a Money value.
       * @param value The value to represent.
       */
      explicit constexpr Money(Quantity value);

      /** Converts this Money to a float. */
      explicit constexpr operator boost::float64_t() const;

      /** Converts this Money to a Quantity. */
      explicit constexpr operator Quantity() const;

      /**
       * Assignment operator.
       * @param rhs The right hand side of the operation.
       * @return <i>this</i>.
       */
      constexpr Money& operator =(Money rhs);

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

      using Details::MoneyDefinitions<Money>::ZERO;
      using Details::MoneyDefinitions<Money>::ONE;
      using Details::MoneyDefinitions<Money>::CENT;
      using Details::MoneyDefinitions<Money>::BIP;
    private:
      template<typename T> friend constexpr Money operator *(T lhs, Money rhs);
      template<typename T> friend constexpr Money operator /(Money lhs, T rhs);
      friend std::ostream& operator <<(std::ostream& out, Money value);
      friend Money Abs(Money value);
      friend Money Floor(Money value, int decimalPlaces);
      friend Money Ceil(Money value, int decimalPlaces);
      friend Money Truncate(Money value, int decimalPlaces);
      friend Money Round(Money value, int decimalPlaces);
      template<typename, typename> friend struct Beam::Serialization::Send;
      template<typename, typename> friend struct Beam::Serialization::Receive;
      Quantity m_value;
  };

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
  constexpr Money operator *(T lhs, Money rhs) {
    return Money{lhs * rhs.m_value};
  }

  /**
   * Divides a Money instance by a scalar quantity.
   * @param lhs The Money instance to be divided.
   * @param rhs The scalar quantity.
   * @return <i>lhs</i> / <i>rhs</i>.
   */
  template<typename T>
  constexpr Money operator /(Money lhs, T rhs) {
    return Money{lhs.m_value / rhs};
  }

  /**
   * Returns the absolute value.
   * @param value The value.
   */
  inline Money Abs(Money value) {
    return Money{Abs(value.m_value)};
  }

  /**
   * Returns the floor.
   * @param value The value to floor.
   * @param decimalPlaces The decimal place to floor to.
   */
  inline Money Floor(Money value, int decimalPlaces) {
    return Money{Floor(value.m_value, decimalPlaces)};
  }

  /**
   * Returns the ceiling.
   * @param value The value to ceil.
   * @param decimalPlaces The decimal place to ceil to.
   */
  inline Money Ceil(Money value, int decimalPlaces) {
    return Money{Ceil(value.m_value, decimalPlaces)};
  }

  /**
   * Returns the truncated value.
   * @param value The value to truncate.
   * @param decimalPlaces The decimal place to truncate.
   */
  inline Money Truncate(Money value, int decimalPlaces) {
    return Money{Truncate(value.m_value, decimalPlaces)};
  }

  /**
   * Returns the rounded value.
   * @param value The value to round.
   * @param decimalPlaces The decimal place to round to.
   */
  inline Money Round(Money value, int decimalPlaces) {
    return Money{Round(value.m_value, decimalPlaces)};
  }

  inline std::size_t hash_value(Money money) noexcept {
    return std::hash<Quantity>()(static_cast<Quantity>(money));
  }

  inline std::ostream& operator <<(std::ostream& out, Money value) {
    auto fraction = value.m_value - Floor(value.m_value, 0);
    auto s = boost::lexical_cast<std::string>(value.m_value);
    if(fraction == 0) {
      return out << s << ".00";
    }
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
    auto parsedValue = Money::FromValue(symbol);
    if(!parsedValue.is_initialized()) {
      in.setstate(std::ios::failbit);
      return in;
    }
    value = *parsedValue;
    return in;
  }

  inline boost::optional<Money> Money::FromValue(const std::string& value) {
    auto quantity = Quantity::FromValue(value);
    if(!quantity.is_initialized()) {
      return boost::none;
    }
    return Money{*quantity};
  }

  inline constexpr Money::Money(Quantity value)
    : m_value(value) {}

  inline constexpr Money::operator boost::float64_t() const {
    return static_cast<boost::float64_t>(m_value);
  }

  inline constexpr Money::operator Quantity() const {
    return m_value;
  }

  inline constexpr Money& Money::operator =(Money rhs) {
    m_value = rhs.m_value;
    return *this;
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

namespace Details {
  template<typename T>
  const T MoneyDefinitions<T>::ZERO(0);

  template<typename T>
  const T MoneyDefinitions<T>::ONE(1);

  template<typename T>
  const T MoneyDefinitions<T>::CENT(T(1) / 100);

  template<typename T>
  const T MoneyDefinitions<T>::BIP(T(1) / 10000);
}
}

namespace Beam::Serialization {
  template<>
  struct IsStructure<Nexus::Money> : std::false_type {};

  template<>
  struct Send<Nexus::Money> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::Money value) const {
      shuttle.Send(name, value.m_value);
    }
  };

  template<>
  struct Receive<Nexus::Money> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::Money& value) const {
      auto representation = Nexus::Quantity();
      shuttle.Shuttle(name, representation);
      value = Nexus::Money{representation};
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
