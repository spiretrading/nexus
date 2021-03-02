#ifndef SPIRE_STYLES_BOX_SELECTORS_HPP
#define SPIRE_STYLES_BOX_SELECTORS_HPP
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Whether the widget is or belongs to the active window. */
  using Active = StateSelector<void, struct ActiveSelectorTag>;

  /** Whether the widget is disabled. */
  using Disabled = StateSelector<void, struct DisabledSelectorTag>;

  /** Whether the widget is being hovered. */
  using Hovered = StateSelector<void, struct StateSelectorTag>;
}

#endif
