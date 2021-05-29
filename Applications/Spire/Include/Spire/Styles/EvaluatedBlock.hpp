#ifndef SPIRE_STYLES_EVALUATED_BLOCK_HPP
#define SPIRE_STYLES_EVALUATED_BLOCK_HPP
#include <vector>
#include "Spire/Styles/Block.hpp"
#include "Spire/Styles/CompositeProperty.hpp"
#include "Spire/Styles/EvaluatedProperty.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Stores a block of EvaluatedProperties. */
  class EvaluatedBlock {
    public:

      /** Constructs an empty EvaluatedBlock. */
      EvaluatedBlock() = default;

      /** Returns the list of EvaluatedProperties. */
      const std::vector<EvaluatedProperty>& get_properties() const&;

      /** Returns the list of EvaluatedProperties. */
      std::vector<EvaluatedProperty>&& get_properties() &&;

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
      bool operator ==(const EvaluatedBlock& block) const;

      bool operator !=(const EvaluatedBlock& block) const;

    private:
      std::vector<EvaluatedProperty> m_properties;
  };

  /**
   * Finds an evaluated property of a given type within an EvaluatedBlock.
   * @param block The block to search.
   */
  template<typename P>
  boost::optional<const typename P::Type&> find(const EvaluatedBlock& block) {
    for(auto& property : block.get_properties()) {
      if(property.get_property_type() == typeid(P)) {
        return property.as<typename P::Type>();
      }
    }
    return boost::none;
  }

  template<typename T>
  void EvaluatedBlock::remove() {
    if constexpr(is_composite_property_v<T>) {
      static_assert(false, "Not supported.");
    } else {
      auto i = std::find_if(m_properties.begin(), m_properties.end(),
        [] (const auto& property) {
          return property.get_property_type() == typeid(T);
        });
      if(i != m_properties.end()) {
        m_properties.erase(i);
      }
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
