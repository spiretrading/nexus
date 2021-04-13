#ifndef SPIRE_STYLES_ANY_HPP
#define SPIRE_STYLES_ANY_HPP
#include <vector>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects all widgets unconditionally. */
  class Any {
    public:
      bool operator ==(const Any& selector) const;

      bool operator !=(const Any& selector) const;
  };

  std::vector<Stylist*> select(const Any& selector, Stylist& source);
}

#endif
