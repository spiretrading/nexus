#ifndef SPIRE_STYLES_RULE_HPP
#define SPIRE_STYLES_RULE_HPP
#include "Spire/Styles/Block.hpp"
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Represents a single styling rule, consisting of a selector that determines
   * what and when the rule applies, and a block containing the styling to
   * apply.
   */
  class Rule {
    public:

      /** Constructs an empty Rule with the All selector. */
      Rule();

      /**
       * Constructs a Rule with the All selector.
       * @param block The block containing the styling.
       */
      explicit Rule(Block block);

      /**
       * Constructs an empty Rule.
       * @param selector The rule's selector.
       */
      explicit Rule(Selector selector);

      /**
       * Constructs a Rule.
       * @param selector The rule's selector.
       * @param block The block containing the styling.
       */
      Rule(Selector selector, Block block);

      /** Returns the rule's selector. */
      const Selector& get_selector() const;

      /** Returns the styling block. */
      const Block& get_block() const;

      /** Returns the styling block. */
      Block& get_block();

      /** Clears this rule's block. */
      void clear();

      /** Sets a property on the Block. */
      Rule& set(Property property);

      /** Sets a property on the Block. */
      template<typename... T>
      Rule& set(CompositeProperty<T...> property);

    private:
      Selector m_selector;
      Block m_block;
  };

  template<typename... T>
  Rule& Rule::set(CompositeProperty<T...> property) {
    m_block.set(std::move(property));
    return *this;
  }
}

#endif
