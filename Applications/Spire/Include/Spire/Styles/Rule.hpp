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

      /** Specifies how properties are overridden by the parent style. */
      enum class Override {

        /** All properties are overridden by the parent (default). */
        INCLUSIVE,

        /** Only properties that are not defined by this rule are overridden. */
        EXCLUSIVE,

        /** No properties are overridden. */
        NONE
      };

      /** Constructs an empty Rule with the All selector. */
      Rule();

      /**
       * Constructs a Rule with the All selector.
       * @param block The block containing the styling.
       */
      explicit Rule(Block block);

      /**
       * Constructs an empty Rule with the All selector and an override rule.
       * @param override The override rule to use.
       */
      explicit Rule(Override override);

      /**
       * Constructs an empty Rule.
       * @param selector The rule's selector.
       */
      explicit Rule(Selector selector);

      /**
       * Constructs a Rule with the All selector and an override rule.
       * @param block The block containing the styling.
       * @param override The override rule to use.
       */
      Rule(Block block, Override override);

      /**
       * Constructs a Rule.
       * @param selector The rule's selector.
       * @param block The block containing the styling.
       */
      Rule(Selector selector, Block block);

      /**
       * Constructs a Rule.
       * @param selector The rule's selector.
       * @param override The override rule to use.
       */
      Rule(Selector selector, Override override);

      /**
       * Constructs a Rule.
       * @param selector The rule's selector.
       * @param block The block containing the styling.
       * @param override The override rule to use.
       */
      Rule(Selector selector, Block block, Override override);

      /** Returns the rule's selector. */
      const Selector& get_selector() const;

      /** Returns the styling block. */
      const Block& get_block() const;

      /** Returns the styling block. */
      Block& get_block();

      /** Returns the override rule. */
      Override get_override() const;

      /** Sets the override rule. */
      Rule& set_override(Override override);

      /** Sets a property on the Block. */
      Rule& set(Property property);

      /** Sets a property on the Block. */
      template<typename... T>
      Rule& set(CompositeProperty<T...> property);

    private:
      Selector m_selector;
      Block m_block;
      Override m_override;
  };

  template<typename... T>
  Rule& Rule::set(CompositeProperty<T...> property) {
    m_block.set(std::move(property));
    return *this;
  }
}

#endif
