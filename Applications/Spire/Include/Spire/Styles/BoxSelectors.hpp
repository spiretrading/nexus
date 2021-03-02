#ifndef SPIRE_STYLES_BOX_SELECTORS_HPP
#define SPIRE_STYLES_BOX_SELECTORS_HPP
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** The state of being hovered. */
  using Hovered = StateSelector<void, struct StateSelectorTag>;
}

#endif
