#ifndef SPIRE_STYLES_BLOCK_HPP
#define SPIRE_STYLES_BLOCK_HPP
#include <vector>
#include <boost/optional/optional.hpp>
#include "Spire/Styles/Property.hpp"
#include "Spire/Styles/Rule.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Stores a block of style properties. */
  class Block {
    public:

      /** Constructs an empty Block. */
      Block();

      /** Constructs a Block with a specified rule. */
      Block(Rule rule);

      /** Returns the Block's Rule. */
      const Rule& get_rule() const;

      /** Returns the list of Properties contained within this Block. */
      const std::vector<Property>& get_properties() const;

      /** Sets a Property, overwriting any previously matching Property. */
      void set(Property property);

      /** Removes a Property. */
      template<typename T>
      void remove();

      /**
       * Removes a Property.
       * @param property The property to remove.
       */
      void remove(const Property& property);

    private:
      Rule m_rule;
      std::vector<Property> m_properties;
  };

  /**
   * Finds a property of a given type within a Block.
   * @param block The block to search.
   */
  template<typename T>
  boost::optional<T> find(const Block& block) {
    for(auto& property : block.get_properties()) {
      if(property.get_type() == typeid(T)) {
        return property.as<T>();
      }
    }
    return boost::none;
  }

  template<typename T>
  void Block::remove() {
    auto i = std::find_if(m_properties.begin(), m_properties.end(),
      [] (const auto& property) {
        return property.get_type() == typeid(T);
      });
    if(i != m_properties.end()) {
      m_properties.erase(i);
    }
  }
}

#endif
