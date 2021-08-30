#ifndef SPIRE_STYLES_ANY_HPP
#define SPIRE_STYLES_ANY_HPP
#include <unordered_set>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects all widgets unconditionally. */
  class Any {
    public:
      bool operator ==(const Any& selector) const;

      bool operator !=(const Any& selector) const;
  };

  std::unordered_set<Stylist*> select(
    const Any& selector, std::unordered_set<Stylist*> sources);
}

#endif
