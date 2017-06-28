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
#include "Nexus/Definitions/Quantity.hpp"

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
  };

  template<typename T>
  const T MoneyStaticDefinitions<T>::ZERO(0);

  template<typename T>
  const T MoneyStaticDefinitions<T>::ONE(1);

  template<typename T>
  const T MoneyStaticDefinitions<T>::CENT(ONE / 100);

  template<typename T>
  const T MoneyStaticDefinitions<T>::BIP(CENT / 100);
}

  /*! \class Money
      \brief Used to represent money without rounding or floating point issues.
   */
  class Money : private Details::MoneyStaticDefinitions<Money> {
    public:

      //! Returns a Money value from a string.
      /*!
        \param value The value to represent.
        \return A Money value representing the specified <i>value</i>.
      */
      static boost::optional<Money> FromValue(const std::string& value);

      //! Constructs a Money value of ZERO.
      Money() = default;

      //! Constructs a Money value.
      /*!
        \param value The value to represent.
      */
      explicit Money(Quantity value);

      //! Converts this Money to a float.
      explicit operator boost::float64_t() const;

      //! Converts this Money to a Quantity.
      explicit operator Quantity() const;

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
    private:
      friend struct Details::MoneyStaticDefinitions<Money>;
      template<typename T> friend Money operator *(T lhs, Money rhs);
      template<typename T> friend Money operator /(Money lhs, T rhs);
      friend Money Abs(Money value);
      friend Money Floor(Money value, int decimalPlaces);
      friend Money Ceil(Money value, int decimalPlaces);
      friend Money Truncate(Money value, int decimalPlaces);
      friend Money Round(Money value, int decimalPlaces);
      template<typename, typename> friend struct Beam::Serialization::Send;
      template<typename, typename> friend struct Beam::Serialization::Receive;
      Quantity m_value;
  };

#ifdef _MSC_VER
  // TODO: MSVC requires explicit instantiation.
  static auto MoneyZero = Details::MoneyStaticDefinitions<Money>::ZERO;
  static auto MoneyOne = Details::MoneyStaticDefinitions<Money>::ONE;
  static auto MoneyCent = Details::MoneyStaticDefinitions<Money>::CENT;
  static auto MoneyBip = Details::MoneyStaticDefinitions<Money>::BIP;
#endif

  //! Multiplies a Money instance by a scalar quantity.
  /*!
    \param lhs The scalar quantity.
    \param rhs The Money instance to be multiplied.
    \return <i>lhs</i> * <i>rhs</i>.
  */
  template<typename T>
  Money operator *(T lhs, Money rhs) {
    return Money{lhs * rhs.m_value};
  }

  //! Divides a Money instance by a scalar quantity.
  /*!
    \param lhs The Money instance to be divided.
    \param rhs The scalar quantity.
    \return <i>lhs</i> / <i>rhs</i>.
  */
  template<typename T>
  Money operator /(Money lhs, T rhs) {
    return Money{lhs.m_value / rhs};
  }

  //! Returns the absolute value.
  /*!
    \param value The value.
  */
  inline Money Abs(Money value) {
    return Money{Abs(value.m_value)};
  }

  //! Returns the floor.
  /*!
    \param value The value to floor.
    \param decimalPlaces The decimal place to floor to.
  */
  inline Money Floor(Money value, int decimalPlaces) {
    return Money{Floor(value.m_value, decimalPlaces)};
  }

  //! Returns the ceiling.
  /*!
    \param value The value to ceil.
    \param decimalPlaces The decimal place to ceil to.
  */
  inline Money Ceil(Money value, int decimalPlaces) {
    return Money{Ceil(value.m_value, decimalPlaces)};
  }

  //! Returns the truncated value.
  /*!
    \param value The value to truncate.
    \param decimalPlaces The decimal place to truncate.
  */
  inline Money Truncate(Money value, int decimalPlaces) {
    return Money{Truncate(value.m_value, decimalPlaces)};
  }

  //! Returns the rounded value.
  /*!
    \param value The value to round.
    \param decimalPlaces The decimal place to round to.
  */
  inline Money Round(Money value, int decimalPlaces) {
    return Money{Round(value.m_value, decimalPlaces)};
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

  inline boost::optional<Money> Money::FromValue(const std::string& value) {
    auto quantity = Quantity::FromValue(value);
    if(!quantity.is_initialized()) {
      return boost::none;
    }
    return Money{*quantity};
  }

  inline Money::Money(Quantity value)
      : m_value{value} {}

  inline Money::operator boost::float64_t() const {
    return static_cast<boost::float64_t>(m_value);
  }

  inline Money::operator Quantity() const {
    return m_value;
  }

  inline std::string Money::ToString() const {
    auto fraction = m_value - Floor(m_value, 0);
    auto s = Beam::ToString(m_value);
    if(s.size() > 1 && *(s.end() - 1) == '.') {
      return s + "00";
    } else if(s.size() > 2 && *(s.end() - 2) == '.') {
      return s + "0";
    } else {
      while(s.size() > 3) {
        if(s.back() == '0' && *(s.end() - 3) != '.') {
          s.pop_back();
        } else {
          break;
        }
      }
      return s;
    }
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
    return static_cast<double>(m_value / rhs.m_value);
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
    return Money{-m_value};
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
      shuttle.Send(name, value.m_value);
    }
  };

  template<>
  struct Receive<Nexus::Money> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::Money& value) const {
      Nexus::Quantity representation;
      shuttle.Shuttle(name, representation);
      value = Nexus::Money{representation};
    }
  };
}
}

namespace std {
  template<>
  class numeric_limits<Nexus::Money> {
    public:
      static Nexus::Money min() {
        return Nexus::Money{numeric_limits<Nexus::Quantity>::min()};
      }

      static Nexus::Money max() {
        return Nexus::Money{numeric_limits<Nexus::Quantity>::max()};
      }
  };
}

#endif
