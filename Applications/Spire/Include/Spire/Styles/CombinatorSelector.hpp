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
       * The consumer of this selector shall return a SelectConnection used to
       * keep track of additions and removals from the base.
       * @param stylist The stylist that was selected by the base.
       * @param on_update The callable used to update the selection from the
       *        base.
       * @return The SelectConnection used to keep track of changes to
       *         selections.
       */
      using SelectionBuilder = std::function<SelectConnection (
        const Stylist& stylist, const SelectionUpdateSignal& on_update)>;

      /**
       * The type used by a SelectionBuilder that performs a single selection
       * and produces no update to the selection.
       * @param stylist The stylist that was selected by the base.
       * @return The set of Stylists selected from the base.
       */
      using StaticSelectionBuilder = std::function<
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

      /**
       * Constructs a CombinatorSelector using a selection builder that performs
       * a single selection and produces no update to the selection.
       * @param base The selector for the base widget.
       * @param match The selector that matches widgets produced by the
       *        <i>base</i>.
       * @param selection_builder The callable used to build up the selection
       *        to match against.
       */
      CombinatorSelector(Selector base, Selector match,
        StaticSelectionBuilder selection_builder);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the match selector. */
      const Selector& get_match() const;

      /** Returns the selection builder. */
      const SelectionBuilder& get_selection_builder() const;

      bool operator ==(const CombinatorSelector& selector) const = default;

    private:
      Selector m_base;
      Selector m_match;
      SelectionBuilder m_selection_builder;
  };

  SelectConnection select(const CombinatorSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update);
}

namespace std {
  template<>
  struct hash<Spire::Styles::CombinatorSelector> {
    std::size_t operator ()(
      const Spire::Styles::CombinatorSelector& selector) const;
  };
}

#endif
