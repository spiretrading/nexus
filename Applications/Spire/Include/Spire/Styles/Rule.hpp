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

      /** Constructs a Rule with the All selector. */
      explicit Rule(Block block);

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

    private:
      Selector m_selector;
      Block m_block;
  };
}

#endif
