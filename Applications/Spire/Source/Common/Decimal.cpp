#include "Spire/Spire/Decimal.hpp"
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

template<>
Decimal Spire::to_decimal(const int& value) {
  return Decimal(value);
}

template<>
int Spire::from_decimal(const Decimal& value) {
  return value.convert_to<int>();
}

template<>
Decimal Spire::to_decimal(const std::uint32_t& value) {
  return Decimal(value);
}

template<>
std::uint32_t Spire::from_decimal(const Decimal& value) {
  return value.convert_to<std::uint32_t>();
}

template<>
Decimal Spire::to_decimal(const Money& value) {
  return Decimal(lexical_cast<std::string>(value));
}

template<>
Money Spire::from_decimal(const Decimal& value) {
  return *Money::FromValue(value.str());
}

template<>
Decimal Spire::to_decimal(const Quantity& value) {
  return Decimal(lexical_cast<std::string>(value));
}

template<>
Quantity Spire::from_decimal(const Decimal& value) {
  return *Quantity::FromValue(value.str());
}
