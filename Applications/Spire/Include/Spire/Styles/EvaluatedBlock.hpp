#ifndef SPIRE_STYLES_EVALUATED_BLOCK_HPP
#define SPIRE_STYLES_EVALUATED_BLOCK_HPP
#include <vector>
#include "Spire/Styles/Block.hpp"
#include "Spire/Styles/CompositeProperty.hpp"
#include "Spire/Styles/EvaluatedProperty.hpp"

namespace Spire::Styles {

  /** Stores a block of EvaluatedProperties. */
  class EvaluatedBlock {
    public:

      /** Constructs an empty EvaluatedBlock. */
      EvaluatedBlock() = default;

      /** Returns an iterator to the first EvaluatedProperty. */
      std::vector<EvaluatedProperty>::const_iterator begin() const;

      /** Returns an iterator to one past the last EvaluatedProperty. */
      std::vector<EvaluatedProperty>::const_iterator end() const;

      /**
       * Returns a Block containing all Properties that correspond to all the
       * stored EvaluatedProperties.
       */
      Block to_block() const;

      /**
       * Sets an EvaluatedProperty, overwriting any previously matching
       * EvaluatedProperty.
       */
      void set(EvaluatedProperty property);

      /** Removes an EvaluatedProperty. */
      template<typename T>
      void remove();

      /**
       * Removes an EvaluatedProperty.
       * @param property The property to remove.
       */
      void remove(const EvaluatedProperty& property);

      /**
       * Removes a Property.
       * @param property The property to remove.
       */
      void remove(const Property& property);

      /**
       * Removes all properties contained within a CompositeProperty.
       * @param property The property to remove.
       */
      template<typename... T>
      void remove(const CompositeProperty<T...>& property);

      /**
       * Tests if two EvaluatedBlock's contain the same EvaluatedProperties in
       * the same order.
       */
      bool operator ==(const EvaluatedBlock& block) const = default;

    private:
      std::vector<EvaluatedProperty> m_properties;
  };

  template<typename T>
  void EvaluatedBlock::remove() {
    static_assert(!is_composite_property_v<T>, "Not supported.");
    auto i = std::find_if(m_properties.begin(), m_properties.end(),
      [] (const auto& property) {
        return property.get_property_type() == typeid(T);
      });
    if(i != m_properties.end()) {
      m_properties.erase(i);
    }
  }

  template<typename... T>
  void EvaluatedBlock::remove(const CompositeProperty<T...>& property) {
    for_each(property, [&] (const auto& property) {
      remove(property);
    });
  }
}

#endif
