#ifndef SPIRE_SCALAR_HPP
#define SPIRE_SCALAR_HPP
#include <cstdint>
#include <functional>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Encapsulates a generic scalar value. */
  class Scalar {
    public:

      /** Lists a Scalar's possible data types. */
      enum class Type {

        //! No data type is represented.
        NONE = 0,

        //! A Quantity is represented.
        QUANTITY,

        //! A Money value is represented.
        MONEY,

        //! A ptime value is represented.
        TIMESTAMP,

        //! A time_duration is represented.
        DURATION
      };

      //! Constructs a value of 0.
      Scalar() = default;

      //! Constructs a Scalar from a Money instance.
      /*!
        \param value The value to represent.
      */
      explicit Scalar(Nexus::Money value);

      //! Constructs a Scalar from a Quantity.
      /*!
        \param value The value to represent.
      */
      explicit Scalar(Nexus::Quantity value);

      //! Constructs a Scalar from a date/time.
      /*!
        \param value The value to represent.
      */
      explicit Scalar(boost::posix_time::ptime value);

      //! Constructs a Scalar from a time duration.
      /*!
        \param value The value to represent.
      */
      explicit Scalar(boost::posix_time::time_duration value);

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
      bool operator <(Scalar rhs) const;

      //! Less than or equal test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is less than or equal to <i>rhs</i>.
      */
      bool operator <=(Scalar rhs) const;

      //! Tests for equality.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is equal to <i>rhs</i>.
      */
      bool operator ==(Scalar rhs) const;

      //! Tests for inequality.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is not equal to <i>rhs</i>.
      */
      bool operator !=(Scalar rhs) const;

      //! Greater than or equal test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is greater than or equal to
                <i>rhs</i>.
      */
      bool operator >=(Scalar rhs) const;

      //! Greater than test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is greater than <i>rhs</i>.
      */
      bool operator >(Scalar rhs) const;

      //! Adds two Scalars together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> + <i>rhs</i>.
      */
      Scalar operator +(Scalar rhs) const;

      //! Increases this Scalar.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Scalar& operator +=(Scalar rhs);

      //! Subtracts two Scalars together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> - <i>rhs</i>.
      */
      Scalar operator -(Scalar rhs) const;

      //! Decreases this Scalar.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Scalar& operator -=(Scalar rhs);

      //! Multiplies this Scalar.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      template<typename T>
      Scalar& operator *=(T rhs);

      //! Takes the modulus of two Scalars.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> % <i>rhs</i>.
      */
      Scalar operator %(Scalar rhs) const;

      //! Divides two Scalars together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> / <i>rhs</i>.
      */
      double operator /(Scalar rhs) const;

      //! Divides this Scalar.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      template<typename T>
      Scalar& operator /=(T rhs);

      //! Returns the unary negation of this Scalar.
      /*!
        \return -<i>this</i>.
      */
      Scalar operator -() const;

    private:
      Nexus::Quantity m_value;
  };

  //! Multiplies a Scalar by a quantity.
  /*!
    \param lhs The quantity.
    \param rhs The Scalar to be multiplied.
    \return <i>lhs</i> * <i>rhs</i>.
  */
  template<typename T>
  Scalar operator *(T lhs, Scalar rhs) {
    return Scalar(lhs * static_cast<Nexus::Quantity>(rhs));
  }

  //! Divides a Scalar by a quantity.
  /*!
    \param lhs The Scalar instance to be divided.
    \param rhs The quantity.
    \return <i>lhs</i> / <i>rhs</i>.
  */
  template<typename T>
  Scalar operator /(Scalar lhs, T rhs) {
    return Scalar(static_cast<Nexus::Quantity>(lhs) / rhs);
  }

  template<typename T>
  Scalar& Scalar::operator *=(T rhs) {
    m_value *= rhs;
    return *this;
  }

  template<typename T>
  Scalar& Scalar::operator /=(T rhs) {
    m_value /= rhs;
    return *this;
  }
}

namespace std {
  template <>
  struct hash<Spire::Scalar> {
    size_t operator()(Spire::Scalar value) const {
      return static_cast<size_t>(static_cast<Nexus::Quantity>(value));
    }
  };
}

#endif
