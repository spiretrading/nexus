#ifndef SPIRE_CHARTVALUE_HPP
#define SPIRE_CHARTVALUE_HPP
#include <cstdint>
#include <functional>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Charting/Charting.hpp"

namespace Spire {

  /*! \class ChartValue
      \brief Stores a value that can be displayed in a chart.
   */
  class ChartValue {
    public:

      //! Builds a ChartValue from its internal representation.
      /*!
        \param value The internal representation of the ChartValue.
      */
      static ChartValue FromRepresentation(std::int64_t value);

      //! Constructs a value of 0.
      ChartValue();

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
        \param value.
      */
      explicit ChartValue(const boost::posix_time::ptime& value);

      //! Constructs a ChartValue from a time duration.
      /*!
        \param value.
      */
      explicit ChartValue(const boost::posix_time::time_duration& value);

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

      //! Returns the date/time value represented by this value.
      boost::posix_time::ptime ToDateTime() const;

      //! Returns the Money value represented by this value.
      Nexus::Money ToMoney() const;

      //! Returns the Quantity represented by this value.
      Nexus::Quantity ToQuantity() const;

      //! Returns the time_duration value represented by this value.
      boost::posix_time::time_duration ToTimeDuration() const;

      //! Returns the internal representation of this ChartValue.
      std::int64_t GetRepresentation() const;

    private:
      std::int64_t m_value;
  };

  //! Multiplies a ChartValue by a scalar quantity.
  /*!
    \param lhs The scalar quantity.
    \param rhs The ChartValue to be multiplied.
    \return <i>lhs</i> * <i>rhs</i>.
  */
  template<typename T>
  ChartValue operator *(T lhs, ChartValue rhs) {
    return ChartValue::FromRepresentation(
      static_cast<std::int64_t>(lhs * rhs.GetRepresentation()));
  }

  //! Divides a ChartValue by a scalar quantity.
  /*!
    \param lhs The ChartValue instance to be divided.
    \param rhs The scalar quantity.
    \return <i>lhs</i> / <i>rhs</i>.
  */
  template<typename T>
  ChartValue operator /(ChartValue lhs, T rhs) {
    return ChartValue::FromRepresentation(
      static_cast<std::int64_t>(lhs.GetRepresentation() / rhs));
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

namespace Beam {
namespace Serialization {
  template<>
  struct IsStructure<Spire::ChartValue> : std::false_type {};

  template<>
  struct Send<Spire::ChartValue> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Spire::ChartValue value) const {
      shuttle.Send(name, value.GetRepresentation());
    }
  };

  template<>
  struct Receive<Spire::ChartValue> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Spire::ChartValue& value) const {
      std::int64_t representation;
      shuttle.Shuttle(name, representation);
      value = Spire::ChartValue::FromRepresentation(representation);
    }
  };
}
}

namespace std {
  template <>
  struct hash<Spire::ChartValue> {
    size_t operator()(Spire::ChartValue value) const {
      return static_cast<size_t>(value.GetRepresentation());
    }
  };
};

#endif
