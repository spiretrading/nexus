#ifndef SPIRE_STATE_SELECTOR_HPP
#define SPIRE_STATE_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Styles.hpp"
#include "Spire/Styles/Stylist.hpp"

namespace Spire::Styles {

  /**
   * Selects a widget that is in a given state.
   * @param <T> The type of data associated with the state.
   * @param <G> A unique type tag.
   */
  template<typename T, typename G>
  class StateSelector {
    public:

      /** The type of data associated with the state. */
      using Type = T;

      /** A unique tag used to identify the state. */
      using Tag = G;

      /**
       * Constructs a StateSelector.
       * @param data The data associated with the state.
       */
      explicit StateSelector(Type data);

      /** Returns the associated data. */
      const Type& get_data() const;

      bool operator ==(const StateSelector& selector) const;

      bool operator !=(const StateSelector& selector) const;

    private:
      Type m_data;
  };

  template<typename G>
  class StateSelector<void, G> {
    public:
      using Tag = G;

      bool operator ==(const StateSelector& selector) const;

      bool operator !=(const StateSelector& selector) const;
  };

  /** Selects the widget that is or belongs to the active window. */
  using Active = StateSelector<void, struct ActiveSelectorTag>;

  /** Selects the disabled widget. */
  using Disabled = StateSelector<void, struct DisabledSelectorTag>;

  /** Selects the hovered widget. */
  using Hover = StateSelector<void, struct HoverSelectorTag>;

  /** Selects the focused widget. */
  using Focus = StateSelector<void, struct FocusSelectorTag>;

  /** Selects a widget if it was focused using a non-pointing device. */
  using FocusVisible = StateSelector<void, struct FocusVisibleSelectorTag>;

  template<typename T, typename G>
  std::unordered_set<Stylist*> select(const StateSelector<T, G>& selector,
      std::unordered_set<Stylist*> sources) {
    for(auto i = sources.begin(); i != sources.end();) {
      auto& source = **i;
      if(!source.is_match(selector)) {
        i = sources.erase(i);
      } else {
        ++i;
      }
    }
    return sources;
  }

  template<typename T, typename G>
  StateSelector<T, G>::StateSelector(Type data)
    : m_data(std::move(data)) {}

  template<typename T, typename G>
  const typename StateSelector<T, G>::Type&
      StateSelector<T, G>::get_data() const {
    return m_data;
  }

  template<typename T, typename G>
  bool StateSelector<T, G>::operator ==(const StateSelector& selector) const {
    return m_data == selector.m_data;
  }

  template<typename T, typename G>
  bool StateSelector<T, G>::operator !=(const StateSelector& selector) const {
    return !(*this == selector);
  }

  template<typename G>
  bool StateSelector<void, G>::operator ==(
      const StateSelector& selector) const {
    return true;
  }

  template<typename G>
  bool StateSelector<void, G>::operator !=(
      const StateSelector& selector) const {
    return !(*this == selector);
  }
}

#endif
