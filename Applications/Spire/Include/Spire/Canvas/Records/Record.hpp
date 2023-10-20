#ifndef SPIRE_RECORD_HPP
#define SPIRE_RECORD_HPP
#include <tuple>
#include <vector>
#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/Tasks/Task.hpp"

namespace Spire {

  /** Represents a basic data structure. */
  class Record {
    public:

      /** Defines the types allowed for a member of a Record. */
      using Field = boost::variant<Record, bool, Nexus::Quantity, double,
        boost::posix_time::ptime, boost::posix_time::time_duration, std::string,
        Nexus::CurrencyId, Nexus::MarketCode, Nexus::Money, Nexus::OrderStatus,
        Nexus::OrderType, Nexus::Security, Nexus::Side, Nexus::TimeInForce,
        Beam::Queries::Range, Beam::Queries::Sequence>;

      /** Constructs an empty Record. */
      Record() = default;

      /**
       * Constructs a Record.
       * @param fields The Fields of this Record.
       */
      explicit Record(std::vector<Field> fields);

      /**
       * Constructs a Record from a tuple.
       * @param fields The tuple containing the Fields of this Record.
       */
      template<typename... T>
      explicit Record(const std::tuple<T...>& fields);

      /** Returns the Fields. */
      const std::vector<Field>& GetFields() const;

      bool operator ==(const Record& rhs) const = default;

    private:
      struct AddField {
        std::vector<Field>& m_fields;

        AddField(std::vector<Field>& fields);
        template<typename T>
        void operator()(T& t) const;
      };
      std::vector<Field> m_fields;
  };

  template<typename T>
  void Record::AddField::operator()(T& t) const {
    m_fields.push_back(t);
  }

  template<typename... T>
  Record::Record(const std::tuple<T...>& fields) {
    boost::fusion::for_each(fields, AddField(m_fields));
  }
}

namespace std {
  template<>
  struct std::hash<Spire::Record> {
    size_t operator()(const Spire::Record& record) const noexcept;
  };
}

#endif
