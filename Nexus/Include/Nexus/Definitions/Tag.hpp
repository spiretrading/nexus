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

  /** Stores a key/value pair used to send generic parameters. */
  class Tag {
    public:

      /** Specifies the types of values that can be stored by a Tag. */
      using Type = boost::variant<int, double, Quantity, Money, char,
        std::string, boost::gregorian::date, boost::posix_time::time_duration,
        boost::posix_time::ptime>;

      /** The index of the int type. */
      static constexpr auto INT_INDEX = 0;

      /** The index of the double type. */
      static constexpr auto DOUBLE_INDEX = 1;

      /** The index of the Quantity type. */
      static constexpr auto QUANTITY_INDEX = 2;

      /** The index of the Money type. */
      static constexpr auto MONEY_INDEX = 3;

      /** The index of the char type. */
      static constexpr auto CHAR_INDEX = 4;

      /** The index of the string type. */
      static constexpr auto STRING_INDEX = 5;

      /** The index of the date type. */
      static constexpr auto DATE_INDEX = 6;

      /** The index of the time type. */
      static constexpr auto TIME_INDEX = 7;

      /** The index of the date-time type. */
      static constexpr auto DATE_TIME_INDEX = 8;

      /** Constructs an uninitialized Tag. */
      Tag() = default;

      /**
       * Constructs a Tag.
       * @param key The Tag's key.
       * @param value The tag's value.
       */
      Tag(int key, Type value);

      /** Returns the key. */
      int GetKey() const;

      /** Returns the value. */
      const Type& GetValue() const;

      bool operator ==(const Tag& tag) const = default;

    private:
      friend struct Beam::Serialization::Shuttle<Tag>;
      int m_key;
      Type m_value;
  };

  inline std::ostream& operator <<(std::ostream& out, const Tag& value) {
    return out << '(' << value.GetKey() << ' ' << value.GetValue() << ')';
  }

  inline Tag::Tag(int key, Type value)
    : m_key(key),
      m_value(std::move(value)) {}

  inline int Tag::GetKey() const {
    return m_key;
  }

  inline const Tag::Type& Tag::GetValue() const {
    return m_value;
  }
}

namespace Beam::Serialization {
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

#endif
