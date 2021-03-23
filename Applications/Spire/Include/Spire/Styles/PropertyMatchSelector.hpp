#ifndef SPIRE_STYLES_PROPERTY_MATCH_SELECTOR_HPP
#define SPIRE_STYLES_PROPERTY_MATCH_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Property.hpp"
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

      bool is_match(const PropertyMatchSelector& selector) const;

    private:
      Property m_property;
  };

  /**
   * Returns a selector that matches a widget whose style contains a matching
   * property.
   * @param property The property to match.
   */
  PropertyMatchSelector matches(Property property);
}

#endif
