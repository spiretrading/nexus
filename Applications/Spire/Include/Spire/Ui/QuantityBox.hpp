#ifndef SPIRE_QUANTITY_BOX_HPP
#define SPIRE_QUANTITY_BOX_HPP
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Ui/DecimalBoxAdaptor.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ScalarValueModel over unsigned ints. */
  using QuantityModel = ScalarValueModel<std::uint32_t>;

  /** A ScalarValueModel over optional unsigned ints. */
  using OptionalQuantityModel =
    ScalarValueModel<boost::optional<std::uint32_t>>;

  /** A LocalScalarValueModel over unsigned ints. */
  using LocalQuantityModel = LocalScalarValueModel<std::uint32_t>;

  /** A LocalScalarValueModel over optional unsigned ints. */
  using LocalOptionalQuantityModel =
    LocalScalarValueModel<boost::optional<std::uint32_t>>;

  /** Displays a widget for inputting Quantity values. */
  using QuantityBox = DecimalBoxAdaptor<std::uint32_t>;
}

#endif
