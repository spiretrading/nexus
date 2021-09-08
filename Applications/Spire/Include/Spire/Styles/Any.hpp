#ifndef SPIRE_STYLES_ANY_HPP
#define SPIRE_STYLES_ANY_HPP
#include "Spire/Styles/SelectConnection.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects all widgets unconditionally. */
  class Any {
    public:
      bool operator ==(const Any& selector) const;

      bool operator !=(const Any& selector) const;
  };

  SelectConnection select(const Any& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);
}

#endif
