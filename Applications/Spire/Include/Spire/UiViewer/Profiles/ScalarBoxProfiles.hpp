#ifndef SPIRE_UI_VIEWER_SCALAR_BOX_PROFILES_HPP
#define SPIRE_UI_VIEWER_SCALAR_BOX_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the DecimalBox. */
  UiProfile make_decimal_box_profile();

  /** Returns a UiProfile for the IntegerBox. */
  UiProfile make_integer_box_profile();

  /** Returns a UiProfile for the MoneyBox. */
  UiProfile make_money_box_profile();

  /** Returns a UiProfile for the PercentBox. */
  UiProfile make_percent_box_profile();

  /** Returns a UiProfile for the QuantityBox. */
  UiProfile make_quantity_box_profile();
}

#endif
