#ifndef SPIRE_STYLES_ANY_HPP
#define SPIRE_STYLES_ANY_HPP
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects all widgets unconditionally. */
  class Any {
    public:
      bool is_match(const Any& selector) const;
  };
}

#endif
