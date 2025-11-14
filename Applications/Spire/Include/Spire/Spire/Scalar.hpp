#ifndef SPIRE_SCALAR_HPP
#define SPIRE_SCALAR_HPP
#include <cstdint>
#include <functional>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Encapsulates a generic scalar value. */
  class Scalar {
    public:

      /** Lists a Scalar's possible data types. */
      enum class Type {

        /** No data type is represented. */
        NONE = 0,

        /** A Quantity is represented. */
        QUANTITY,

        /** A Money value is represented. */
        MONEY,

        /** A ptime value is represented. */
        TIMESTAMP,

        /** A time_duration is represented. */
        DURATION
      };

      /** Constructs a value of 0. */
      Scalar() noexcept = default;

      /**
       * Constructs a Scalar from a Money instance.
       * @param value The value to represent.
       */
      explicit Scalar(Nexus::Money value) noexcept;

      /**
       * Constructs a Scalar from a Quantity.
       * @param value The value to represent.
       */
      explicit Scalar(Nexus::Quantity value) noexcept;

      /**
       * Constructs a Scalar from a date/time.
       * @param value The value to represent.
       */
      explicit Scalar(boost::posix_time::ptime value) noexcept;

      /**
       * Constructs a Scalar from a time duration.
       * @param value The value to represent.
       */
      explicit Scalar(boost::posix_time::time_duration value) noexcept;

      explicit operator boost::posix_time::ptime() const;
      explicit operator Nexus::Money() const;
      explicit operator Nexus::Quantity() const;
      explicit operator boost::posix_time::time_duration() const;
      auto operator <=>(const Scalar& rhs) const = default;
      Scalar operator +(Scalar rhs) const;
      Scalar& operator +=(Scalar rhs);
      Scalar operator -(Scalar rhs) const;
      Scalar& operator -=(Scalar rhs);
      template<typename T>
      Scalar& operator *=(T rhs);
      Scalar operator %(Scalar rhs) const;
      double operator /(Scalar rhs) const;
      template<typename T>
      Scalar& operator /=(T rhs);
      Scalar operator -() const;

    private:
      Nexus::Quantity m_value;
  };

  /**
   * Multiplies a Scalar by a quantity.
   * @param lhs The quantity.
   * @param rhs The Scalar to be multiplied.
   * @return <i>lhs</i> * <i>rhs</i>.
   */
  template<typename T>
  Scalar operator *(T lhs, Scalar rhs) {
    return Scalar(lhs * static_cast<Nexus::Quantity>(rhs));
  }

  /**
   * Divides a Scalar by a quantity.
   * @param lhs The Scalar instance to be divided.
   * @param rhs The quantity.
   * @return <i>lhs</i> / <i>rhs</i>.
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
    size_t operator()(Spire::Scalar value) const noexcept {
      return static_cast<size_t>(static_cast<Nexus::Quantity>(value));
    }
  };
}

#endif
