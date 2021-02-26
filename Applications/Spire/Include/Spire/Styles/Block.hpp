#ifndef SPIRE_STYLES_BLOCK_HPP
#define SPIRE_STYLES_BLOCK_HPP
#include <vector>
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

    private:
      Rule m_rule;
      std::vector<Property> m_properties;
  };
}

#endif
