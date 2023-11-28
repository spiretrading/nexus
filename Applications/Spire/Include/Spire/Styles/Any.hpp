#ifndef SPIRE_STYLES_ANY_HPP
#define SPIRE_STYLES_ANY_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects all widgets unconditionally. */
  class Any {
    public:
      bool operator ==(const Any& selector) const = default;
  };

  SelectConnection select(const Any& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);
}

namespace std {
  template<>
  struct hash<Spire::Styles::Any> {
    std::size_t operator ()(const Spire::Styles::Any& selector);
  };
}

#endif
