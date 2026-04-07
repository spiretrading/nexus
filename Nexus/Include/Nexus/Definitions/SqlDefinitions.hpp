#ifndef NEXUS_DEFINITIONS_SQL_DEFINITIONS_HPP
#define NEXUS_DEFINITIONS_SQL_DEFINITIONS_HPP
#include <Viper/Conversions.hpp>
#include <Viper/DataTypes/NativeToDataType.hpp>
#include <Beam/Sql/Conversions.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Viper {
  template<>
  inline const auto native_to_data_type_v<Nexus::CountryCode> = small_uint;

  template<>
  inline const auto native_to_data_type_v<Nexus::CurrencyId> = small_uint;

  template<>
  inline const auto native_to_data_type_v<Nexus::Quantity> = f64;

  template<>
  inline const auto native_to_data_type_v<Nexus::Money> = f64;

  template<>
  struct ToSql<Nexus::CountryCode> {
    void operator ()(Nexus::CountryCode value, std::string& column) const {
      to_sql(static_cast<std::uint16_t>(value), column);
    }
  };

  template<>
  struct FromSql<Nexus::CountryCode> {
    auto operator ()(const RawColumn& column) const {
      return Nexus::CountryCode(from_sql<std::uint16_t>(column));
    }
  };

  template<>
  struct ToSql<Nexus::Venue> {
    void operator ()(Nexus::Venue value, std::string& column) const {
      to_sql(value.get_code(), column);
    }
  };

  template<>
  struct FromSql<Nexus::Venue> {
    auto operator ()(const RawColumn& column) const {
      return Nexus::Venue(from_sql<Nexus::Venue::Code>(column));
    }
  };

  template<>
  struct ToSql<Nexus::CurrencyId> {
    void operator ()(Nexus::CurrencyId value, std::string& column) const {
      to_sql(static_cast<std::uint16_t>(value), column);
    }
  };

  template<>
  struct FromSql<Nexus::CurrencyId> {
    auto operator ()(const RawColumn& column) const {
      return Nexus::CurrencyId(from_sql<std::uint16_t>(column));
    }
  };

  template<>
  struct ToSql<Nexus::Quantity> {
    void operator ()(Nexus::Quantity value, std::string& column) const {
      to_sql(value.get_representation(), column);
    }
  };

  template<>
  struct FromSql<Nexus::Quantity> {
    auto operator ()(const RawColumn& column) const {
      return Nexus::Quantity::from_representation(
        from_sql<boost::float64_t>(column));
    }
  };

  template<>
  struct ToSql<Nexus::Money> {
    void operator ()(Nexus::Money value, std::string& column) const {
      to_sql(static_cast<Nexus::Quantity>(value), column);
    }
  };

  template<>
  struct FromSql<Nexus::Money> {
    auto operator ()(const RawColumn& column) const {
      return Nexus::Money(from_sql<Nexus::Quantity>(column));
    }
  };
}

#endif
