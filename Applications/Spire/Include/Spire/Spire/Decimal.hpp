#ifndef SPIRE_DECIMAL_HPP
#define SPIRE_DECIMAL_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ScalarValueModel.hpp"

namespace Spire {

  /** Represents the floating point type used by the DecimalBox. */
  using Decimal = boost::multiprecision::number<
    boost::multiprecision::cpp_dec_float<15>>;

  /** Type of model used by the DecimalBox. */
  using DecimalModel = ScalarValueModel<Decimal>;

  /** A ScalarValueModel over optional Decimals. */
  using OptionalDecimalModel = ScalarValueModel<boost::optional<Decimal>>;

  /** A LocalScalarValueModel over Decimals. */
  using LocalDecimalModel = LocalScalarValueModel<Decimal>;

  /** A LocalScalarValueModel over optional Decimals. */
  using LocalOptionalDecimalModel =
    LocalScalarValueModel<boost::optional<Decimal>>;
}

#endif
