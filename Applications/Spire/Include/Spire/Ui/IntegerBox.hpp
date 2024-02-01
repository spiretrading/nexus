#ifndef SPIRE_INTEGER_BOX_HPP
#define SPIRE_INTEGER_BOX_HPP
#include "Spire/Ui/DecimalBoxAdaptor.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ScalarValueModel over ints. */
  using IntegerModel = ScalarValueModel<int>;

  /** A ScalarValueModel over optional ints. */
  using OptionalIntegerModel = ScalarValueModel<boost::optional<int>>;

  /** A LocalScalarValueModel over ints. */
  using LocalIntegerModel = LocalScalarValueModel<int>;

  /** A LocalScalarValueModel over optional ints. */
  using LocalOptionalIntegerModel = LocalScalarValueModel<boost::optional<int>>;

  /** Displays a widget for inputting integer values. */
  using IntegerBox = DecimalBoxAdaptor<int>;
}

#endif
