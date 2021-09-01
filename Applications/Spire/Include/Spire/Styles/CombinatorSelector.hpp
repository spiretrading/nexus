#ifndef SPIRE_STYLES_COMBINATOR_SELECTOR_HPP
#define SPIRE_STYLES_COMBINATOR_SELECTOR_HPP
#include <functional>
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Provides support for composing two selectors together. */
  class CombinatorSelector {
    public:

      /**
       * The type of callable used to indicate that the base added a selection.
       * The consumer of this selector shall return a set of Stylists to
       * match against.
       * @param stylist The stylist that was selected by the base.
       * @return A set of Stylists to match against.
       */
      using SelectionBuilder = std::function<
        std::unordered_set<const Stylist*> (const Stylist& stylist)>;

      /**
       * Constructs a CombinatorSelector.
       * @param base The selector for the base widget.
       * @param match The selector that matches widgets produced by the
       *        <i>base</i>.
       * @param selection_builder The callable used to build up the selection
       *        to match against.
       */
      CombinatorSelector(
        Selector base, Selector match, SelectionBuilder selection_builder);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the match selector. */
      const Selector& get_match() const;

      /** Returns the selection builder. */
      const SelectionBuilder& get_selection_builder() const;

      bool operator ==(const CombinatorSelector& selector) const;

      bool operator !=(const CombinatorSelector& selector) const;

    private:
      Selector m_base;
      Selector m_match;
      SelectionBuilder m_selection_builder;
  };

  SelectConnection select(const CombinatorSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update);
}

#endif
