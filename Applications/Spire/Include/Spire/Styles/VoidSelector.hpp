#ifndef SPIRE_STYLES_VOID_SELECTOR_HPP
#define SPIRE_STYLES_VOID_SELECTOR_HPP
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selector that never matches anything. */
  class VoidSelector {
    public:
      bool is_match(const VoidSelector& selector) const;
  };
}

#endif
