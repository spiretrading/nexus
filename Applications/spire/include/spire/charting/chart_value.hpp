#ifndef SPIRE_CHART_VALUE_HPP
#define SPIRE_CHART_VALUE_HPP
#include <cstdint>
#include <functional>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "spire/charting/charting.hpp"

namespace Spire {

  /** Stores a value that can be displayed in a chart. */
  class ChartValue {
    public:

      /** Lists the data types a value can represent. */
      enum class Type {

        //! No data type is represented.
        NONE = 0,

        //! A Quantity is represented.
        QUANTITY,

        //! A Money value is represented.
        MONEY,

        //! A ptime value is represented.
        TIME_STAMP,

        //! A time_duration is represented.
        DURATION
      };

      //! Constructs a value of 0.
      ChartValue() = default;

      //! Constructs a ChartValue from a Money instance.
      /*!
        \param value The value to represent.
      */
      explicit ChartValue(Nexus::Money value);

      //! Constructs a ChartValue from a Quantity.
      /*!
        \param value The value to represent.
      */
      explicit ChartValue(Nexus::Quantity value);

      //! Constructs a ChartValue from a date/time.
      /*!
        \param value The value to represent.
      */
      explicit ChartValue(boost::posix_time::ptime value);

      //! Constructs a ChartValue from a time duration.
      /*!
        \param value The value to represent.
      */
      explicit ChartValue(boost::posix_time::time_duration value);

      //! Returns the date/time value represented by this value.
      explicit operator boost::posix_time::ptime() const;

      //! Returns the Money value represented by this value.
      explicit operator Nexus::Money() const;

      //! Returns the Quantity represented by this value.
      explicit operator Nexus::Quantity() const;

      //! Returns the time_duration value represented by this value.
      explicit operator boost::posix_time::time_duration() const;

      //! Less than test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is less than <i>rhs</i>.
      */
      bool operator <(ChartValue rhs) const;

      //! Less than or equal test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is less than or equal to <i>rhs</i>.
      */
      bool operator <=(ChartValue rhs) const;

      //! Tests for equality.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is equal to <i>rhs</i>.
      */
      bool operator ==(ChartValue rhs) const;

      //! Tests for inequality.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is not equal to <i>rhs</i>.
      */
      bool operator !=(ChartValue rhs) const;

      //! Greater than or equal test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is greater than or equal to
                <i>rhs</i>.
      */
      bool operator >=(ChartValue rhs) const;

      //! Greater than test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is greater than <i>rhs</i>.
      */
      bool operator >(ChartValue rhs) const;

      //! Adds two ChartValues together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> + <i>rhs</i>.
      */
      ChartValue operator +(ChartValue rhs) const;

      //! Increases this ChartValue.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      ChartValue& operator +=(ChartValue rhs);

      //! Subtracts two ChartValues together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> - <i>rhs</i>.
      */
      ChartValue operator -(ChartValue rhs) const;

      //! Decreases this ChartValue.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      ChartValue& operator -=(ChartValue rhs);

      //! Multiplies this ChartValue.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      template<typename T>
      ChartValue& operator *=(T rhs);

      //! Takes the modulus of two ChartValues.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> % <i>rhs</i>.
      */
      ChartValue operator %(ChartValue rhs) const;

      //! Divides two ChartValues together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> / <i>rhs</i>.
      */
      double operator /(ChartValue rhs) const;

      //! Divides this ChartValue.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      template<typename T>
      ChartValue& operator /=(T rhs);

      //! Returns the unary negation of this ChartValue.
      /*!
        \return -<i>this</i>.
      */
      ChartValue operator -() const;

    private:
      Nexus::Quantity m_value;
  };

  //! Multiplies a ChartValue by a scalar quantity.
  /*!
    \param lhs The scalar quantity.
    \param rhs The ChartValue to be multiplied.
    \return <i>lhs</i> * <i>rhs</i>.
  */
  template<typename T>
  ChartValue operator *(T lhs, ChartValue rhs) {
    return ChartValue(lhs * rhs.ToQuantity());
  }

  //! Divides a ChartValue by a scalar quantity.
  /*!
    \param lhs The ChartValue instance to be divided.
    \param rhs The scalar quantity.
    \return <i>lhs</i> / <i>rhs</i>.
  */
  template<typename T>
  ChartValue operator /(ChartValue lhs, T rhs) {
    return ChartValue(lhs.ToQuantity() / rhs);
  }

  template<typename T>
  ChartValue& ChartValue::operator *=(T rhs) {
    m_value *= rhs;
    return *this;
  }

  template<typename T>
  ChartValue& ChartValue::operator /=(T rhs) {
    m_value /= rhs;
    return *this;
  }
}

namespace std {
  template <>
  struct hash<Spire::ChartValue> {
    size_t operator()(Spire::ChartValue value) const {
      return static_cast<size_t>(static_cast<Nexus::Quantity>(value));
    }
  };
}

#endif
