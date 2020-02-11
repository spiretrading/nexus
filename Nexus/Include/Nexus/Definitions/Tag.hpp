#ifndef NEXUS_TAG_HPP
#define NEXUS_TAG_HPP
#include <ostream>
#include <string>
#include <Beam/Serialization/ShuttleVariant.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/variant.hpp>
#include "Nexus/Definitions/Money.hpp"

namespace Nexus {

  /*! \class Tag
      \brief Stores a key/value pair used to send generic parameters.
   */
  class Tag {
    public:

      //! Specifies the types of values that can be stored by a Tag.
      using Type = boost::variant<int, double, Quantity, Money, char,
        std::string, boost::gregorian::date, boost::posix_time::time_duration,
        boost::posix_time::ptime>;

      //! The index of the int type.
      static const int INT_INDEX = 0;

      //! The index of the double type.
      static const int DOUBLE_INDEX = 1;

      //! The index of the Quantity type.
      static const int QUANTITY_INDEX = 2;

      //! The index of the Money type.
      static const int MONEY_INDEX = 3;

      //! The index of the char type.
      static const int CHAR_INDEX = 4;

      //! The index of the string type.
      static const int STRING_INDEX = 5;

      //! The index of the date type.
      static const int DATE_INDEX = 6;

      //! The index of the time type.
      static const int TIME_INDEX = 7;

      //! The index of the date-time type.
      static const int DATE_TIME_INDEX = 8;

      //! Constructs an uninitialized Tag.
      Tag() = default;

      //! Constructs a Tag.
      /*!
        \param key The Tag's key.
        \param value The tag's value.
      */
      Tag(int key, const Type& value);

      //! Returns <code>true</code> iff this Tag is equal to another.
      /*!
        \param tag The tag to test for equality.
        \return <code>true</code> iff this Tag is equal to <code>rhs</code>.
      */
      bool operator ==(const Tag& tag) const;

      //! Returns the key.
      int GetKey() const;

      //! Returns the value.
      const Type& GetValue() const;

    private:
      friend struct Beam::Serialization::Shuttle<Tag>;
      int m_key;
      Type m_value;
  };

  inline std::ostream& operator <<(std::ostream& out, const Tag& value) {
    return out << '(' << value.GetKey() << ' ' << value.GetValue() << ')';
  }

  inline Tag::Tag(int key, const Type& value)
      : m_key{key},
        m_value{value} {}

  inline bool Tag::operator ==(const Tag& tag) const {
    return std::tie(m_key, m_value) == std::tie(tag.m_key, tag.m_value);
  }

  inline int Tag::GetKey() const {
    return m_key;
  }

  inline const Tag::Type& Tag::GetValue() const {
    return m_value;
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Nexus::Tag> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::Tag& value,
        unsigned int version) {
      shuttle.Shuttle("key", value.m_key);
      shuttle.Shuttle("value", value.m_value);
    }
  };
}
}

#endif
