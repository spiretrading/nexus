#ifndef SPIRE_STYLES_PROPERTY_MATCH_SELECTOR_HPP
#define SPIRE_STYLES_PROPERTY_MATCH_SELECTOR_HPP
#include "Spire/Styles/Property.hpp"
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget whose style contains a matching property. */
  class PropertyMatchSelector {
    public:

      /**
       * Constructs a PropertyMatchSelector.
       * @param property The property to match.
       */
      explicit PropertyMatchSelector(Property property);

      /** Returns the property that must match. */
      const Property& get_property() const;

      bool operator ==(const PropertyMatchSelector& selector) const = default;

    private:
      Property m_property;
  };

  /**
   * Returns a selector that matches a widget whose style contains a matching
   * property.
   * @param property The property to match.
   */
  PropertyMatchSelector matches(Property property);

  SelectConnection select(const PropertyMatchSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update);
}

namespace std {
  template<>
  struct hash<Spire::Styles::PropertyMatchSelector> {
    std::size_t operator ()(
      const Spire::Styles::PropertyMatchSelector& selector) const;
  };
}

#endif
