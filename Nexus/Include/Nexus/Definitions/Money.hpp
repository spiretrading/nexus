#ifndef NEXUS_MONEY_HPP
#define NEXUS_MONEY_HPP
#include <cstdint>
#include <cstdlib>
#include <istream>
#include <ostream>
#include <string>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Utilities/Math.hpp>
#include <boost/optional/optional.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {
namespace Details {
  template<typename T>
  struct MoneyStaticDefinitions {

    //! Stores a value of 0.
    static const T ZERO;

    //! Stores a value of 1.
    static const T ONE;

    //! Stores a value of 0.01.
    static const T CENT;

    //! Stores a value of 0.0001.
    static const T BIP;

    //! Stores the smallest unit of Money.
    static const T EPSILON;
  };

  template<typename T>
  const T MoneyStaticDefinitions<T>::ZERO(T::FromRepresentation(0));

  template<typename T>
  const T MoneyStaticDefinitions<T>::ONE(T::FromRepresentation(T::MULTIPLIER));

  template<typename T>
  const T MoneyStaticDefinitions<T>::CENT(T::FromRepresentation(
    T::MULTIPLIER / 100));

  template<typename T>
  const T MoneyStaticDefinitions<T>::BIP(T::FromRepresentation(
    T::MULTIPLIER / 10000));

  template<typename T>
  const T MoneyStaticDefinitions<T>::EPSILON(T::FromRepresentation(1));
}

  /*! \class Money
      \brief Used to represent money without rounding or floating point issues.
   */
  class Money : private Details::MoneyStaticDefinitions<Money> {
    public:

      //! The number of decimal places that can be represented.
      static const unsigned int DECIMAL_PLACES = 6;

      //! The multiplier used.
      static const std::int64_t MULTIPLIER = 1000000;

      //! Returns a Money value from its representation.
      /*!
        \param value The value.
        \return A Money value representing the specified <i>value</i>.
      */
      static Money FromRepresentation(std::int64_t value);

      //! Returns a Money value from a double.
      /*!
        \param value The value to represent.
        \return A Money value instance representing the specified <i>value</i>.
      */
      static Money FromValue(double value);

      //! Returns a Money value from a string.
      /*!
        \param value The value to represent.
        \return A Money value representing the specified <i>value</i>.
      */
      static boost::optional<Money> FromValue(const std::string& value);

      //! Constructs an uninitialized Money value.
      Money();

      //! Returns the underlying representation.
      std::int64_t GetRepresentation() const;

      //! Returns the string representation of this value.
      std::string ToString() const;

      //! Less than test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is less than <i>rhs</i>.
      */
      bool operator <(Money rhs) const;

      //! Less than or equal test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is less than or equal to <i>rhs</i>.
      */
      bool operator <=(Money rhs) const;

      //! Tests for equality.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is equal to <i>rhs</i>.
      */
      bool operator ==(Money rhs) const;

      //! Tests for inequality.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is not equal to <i>rhs</i>.
      */
      bool operator !=(Money rhs) const;

      //! Greater than or equal test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is greater than or equal to
                <i>rhs</i>.
      */
      bool operator >=(Money rhs) const;

      //! Greater than test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is greater than <i>rhs</i>.
      */
      bool operator >(Money rhs) const;

      //! Assignment operator.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Money& operator =(Money rhs);

      //! Adds two Money instances together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> + <i>rhs</i>.
      */
      Money operator +(Money rhs) const;

      //! Increases this Money instance.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Money& operator +=(Money rhs);

      //! Subtracts two Money instances together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> - <i>rhs</i>.
      */
      Money operator -(Money rhs) const;

      //! Decreases this Money instance.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Money& operator -=(Money rhs);

      //! Divides two Money instances together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> / <i>rhs</i>.
      */
      double operator /(Money rhs) const;

      //! Multiplies this Money instance.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      template<typename T>
      Money& operator *=(T rhs);

      //! Divides this Money instance.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      template<typename T>
      Money& operator /=(T rhs);

      //! Returns the unary negation of this Money instance.
      /*!
        \return -<i>this</i>.
      */
      Money operator -() const;

      using Details::MoneyStaticDefinitions<Money>::ZERO;
      using Details::MoneyStaticDefinitions<Money>::ONE;
      using Details::MoneyStaticDefinitions<Money>::CENT;
      using Details::MoneyStaticDefinitions<Money>::BIP;
      using Details::MoneyStaticDefinitions<Money>::EPSILON;
    private:
      std::int64_t m_value;

      explicit Money(std::int64_t value);
  };

#ifdef _MSC_VER
  // TODO: MSVC requires explicit instantiation.
  static auto MoneyZero = Details::MoneyStaticDefinitions<Money>::ZERO;
  static auto MoneyOne = Details::MoneyStaticDefinitions<Money>::ONE;
  static auto MoneyCent = Details::MoneyStaticDefinitions<Money>::CENT;
  static auto MoneyBip = Details::MoneyStaticDefinitions<Money>::BIP;
  static auto MoneyEpsilon = Details::MoneyStaticDefinitions<Money>::EPSILON;
#endif

  //! Multiplies a Money instance by a scalar quantity.
  /*!
    \param lhs The scalar quantity.
    \param rhs The Money instance to be multiplied.
    \return <i>lhs</i> * <i>rhs</i>.
  */
  template<typename T>
  Money operator *(T lhs, Money rhs) {
    return Money::FromRepresentation(
      static_cast<std::int64_t>(lhs * rhs.GetRepresentation()));
  }

  //! Multiplies a Money instance by a rational quantity.
  /*!
    \param lhs The rational quantity.
    \param rhs The Money instance to be multiplied.
    \return <i>lhs</i> * <i>rhs</i>.
  */
  template<typename T>
  Money operator *(const boost::rational<T>& lhs, Money rhs) {
    return Money::FromRepresentation(static_cast<std::int64_t>(
      (lhs.numerator() * rhs.GetRepresentation()) / lhs.denominator()));
  }

  //! Divides a Money instance by a scalar quantity.
  /*!
    \param lhs The Money instance to be divided.
    \param rhs The scalar quantity.
    \return <i>lhs</i> / <i>rhs</i>.
  */
  template<typename T>
  Money operator /(Money lhs, T rhs) {
    return Money::FromRepresentation(
      static_cast<std::int64_t>(lhs.GetRepresentation() / rhs));
  }

  //! Returns the absolute value.
  /*!
    \param value The value.
  */
  inline Money Abs(Money value) {
    return Money::FromRepresentation(std::abs(value.GetRepresentation()));
  }

  //! Returns the floor.
  /*!
    \param value The value to floor.
    \param decimalPlaces The decimal place to floor to.
  */
  Money Floor(Money value, int decimalPlaces);

  //! Returns the ceiling.
  /*!
    \param value The value to ceil.
    \param decimalPlaces The decimal place to ceil to.
  */
  Money Ceil(Money value, int decimalPlaces);

  //! Returns the truncated value.
  /*!
    \param value The value to truncate.
    \param decimalPlaces The decimal place to truncate.
  */
  inline Money Truncate(Money value, int decimalPlaces) {
    auto decimalPoint = Beam::PowerOfTen(Money::DECIMAL_PLACES - decimalPlaces);
    return decimalPoint * (value / decimalPoint);
  }

  //! Returns the rounded value.
  /*!
    \param value The value to round.
    \param decimalPlaces The decimal place to round to.
  */
  inline Money Round(Money value, int decimalPlaces) {
    auto absValue = Abs(value);
    auto floorValue = Floor(value, decimalPlaces);
    auto decimalPoint = Beam::PowerOfTen(decimalPlaces);
    if(absValue - floorValue >= ((Money::ONE / 2) / decimalPoint)) {
      floorValue += Money::ONE / Beam::PowerOfTen(decimalPlaces);
    }
    return floorValue;
  }

  //! Returns the floating point representation of the value.
  /*!
    \param value The value to get the floating point representation of.
    \return The floating point representation of the <i>value</i>.
  */
  inline double ToDouble(Money value) {
    return value.GetRepresentation() /
      static_cast<double>(Money::MULTIPLIER);
  }

  inline std::ostream& operator <<(std::ostream& out, Money value) {
    return out << value.ToString();
  }

  inline std::istream& operator >>(std::istream& in, Money& value) {
    std::string symbol;
    in >> symbol;
    auto parsedValue = Money::FromValue(symbol);
    if(!parsedValue.is_initialized()) {
      in.setstate(std::ios::failbit);
      return in;
    }
    value = *parsedValue;
    return in;
  }

  inline Money Money::FromRepresentation(std::int64_t value) {
    return Money{value};
  }

  inline Money Money::FromValue(double value) {
    return Money(static_cast<std::int64_t>(value * MULTIPLIER));
  }

  inline boost::optional<Money> Money::FromValue(const std::string& value) {
    if(value.empty()) {
      return boost::none;
    }
    auto i = value.begin();
    std::int64_t sign;
    if(*i == '-') {
      sign = -1;
      ++i;
    } else {
      sign = 1;
      if(*i == '+') {
        ++i;
      }
    }
    std::int64_t leftHand = 0;
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
    std::int64_t rightHand = 0;
    auto multiplier = Beam::PowerOfTen(DECIMAL_PLACES);
    if(hasDecimals) {
      while(i != value.end()) {
        if(*i >= '0' && *i <= '9') {
          rightHand *= 10;
          rightHand += *i - '0';
          ++i;
          multiplier /= 10;
        } else {
          return boost::none;
        }
      }
    }
    auto finalValue = sign * (leftHand * Beam::PowerOfTen(DECIMAL_PLACES) +
      rightHand * multiplier);
    return Money{static_cast<std::int64_t>(finalValue)};
  }

  inline Money::Money()
      : m_value{0} {}

  inline std::int64_t Money::GetRepresentation() const {
    return m_value;
  }

  inline Money::Money(std::int64_t value)
      : m_value{value} {}

  inline std::string Money::ToString() const {
    char buffer[64];
    auto split = std::lldiv(m_value, MULTIPLIER);
    auto length = 0;
    if(split.quot >= 0 && split.rem < 0) {
      buffer[0] = '-';
      ++length;
    }
    split.rem = std::abs(split.rem);
    length += std::sprintf(buffer + length, "%lld", split.quot);
    buffer[length] = '.';
    ++length;
    for(std::size_t i = 0; i < 2 || split.rem != 0; ++i) {
      buffer[length] = '0' + static_cast<char>(
        split.rem / Beam::PowerOfTen(DECIMAL_PLACES - i - 1));
      ++length;
      split.rem %= Beam::PowerOfTen(DECIMAL_PLACES - i - 1);
    }
    buffer[length] = '\0';
    return buffer;
  }

  inline bool Money::operator <(Money rhs) const {
    return m_value < rhs.m_value;
  }

  inline bool Money::operator <=(Money rhs) const {
    return m_value <= rhs.m_value;
  }

  inline bool Money::operator ==(Money rhs) const {
    return m_value == rhs.m_value;
  }

  inline bool Money::operator !=(Money rhs) const {
    return m_value != rhs.m_value;
  }

  inline bool Money::operator >=(Money rhs) const {
    return m_value >= rhs.m_value;
  }

  inline bool Money::operator >(Money rhs) const {
    return m_value > rhs.m_value;
  }

  inline Money& Money::operator =(Money rhs) {
    m_value = rhs.m_value;
    return *this;
  }

  inline Money Money::operator +(Money rhs) const {
    return Money(m_value + rhs.m_value);
  }

  inline Money& Money::operator +=(Money rhs) {
    m_value += rhs.m_value;
    return *this;
  }

  inline Money Money::operator -(Money rhs) const {
    return Money(m_value - rhs.m_value);
  }

  inline Money& Money::operator -=(Money rhs) {
    m_value -= rhs.m_value;
    return *this;
  }

  inline double Money::operator /(Money rhs) const {
    return m_value / static_cast<double>(rhs.m_value);
  }

  template<typename T>
  Money& Money::operator *=(T rhs) {
    m_value *= rhs;
    return *this;
  }

  template<typename T>
  Money& Money::operator /=(T rhs) {
    m_value /= rhs;
    return *this;
  }

  inline Money Money::operator -() const {
    return Money(-m_value);
  }

  inline Money Floor(Money value, int decimalPlaces) {
    if(value < Money::ZERO) {
      return -Ceil(-value, decimalPlaces);
    }
    return Beam::PowerOfTen(Money::DECIMAL_PLACES - decimalPlaces) *
      (value / Beam::PowerOfTen(Money::DECIMAL_PLACES - decimalPlaces));
  }

  inline Money Ceil(Money value, int decimalPlaces) {
    if(value < Money::ZERO) {
      return -Floor(-value, decimalPlaces);
    }
    auto floor = Floor(value, decimalPlaces);
    if(value - floor == Money::ZERO) {
      return floor;
    }
    return floor + Beam::PowerOfTen(Money::DECIMAL_PLACES - decimalPlaces) *
      Money::EPSILON;
  }
}

namespace Beam {
  inline std::string ToString(Nexus::Money value) {
    return value.ToString();
  }

namespace Serialization {
  template<>
  struct IsStructure<Nexus::Money> : std::false_type {};

  template<>
  struct Send<Nexus::Money> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const Nexus::Money& value) const {
      shuttle.Send(name, value.GetRepresentation());
    }
  };

  template<>
  struct Receive<Nexus::Money> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::Money& value) const {
      std::int64_t representation;
      shuttle.Shuttle(name, representation);
      value = Nexus::Money::FromRepresentation(representation);
    }
  };
}
}

namespace std {
  template<>
  class numeric_limits<Nexus::Money> {
    public:
      static Nexus::Money min() {
        return Nexus::Money::FromRepresentation(
          std::numeric_limits<std::int64_t>::min());
      }

      static Nexus::Money max() {
        return Nexus::Money::FromRepresentation(
          std::numeric_limits<std::int64_t>::max());
      }
  };
}

#endif
