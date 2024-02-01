#ifndef SPIRE_STATE_SELECTOR_HPP
#define SPIRE_STATE_SELECTOR_HPP
#include <utility>
#include <boost/signals2/connection.hpp>
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

      bool operator ==(const StateSelector& selector) const = default;

    private:
      Type m_data;
  };

  template<typename G>
  class StateSelector<void, G> {
    public:
      using Tag = G;

      bool operator ==(const StateSelector& selector) const = default;
  };

  /** Selects the widget that is or belongs to the active window. */
  using Active = StateSelector<void, struct ActiveSelectorTag>;

  /** Selects the disabled widget. */
  using Disabled = StateSelector<void, struct DisabledSelectorTag>;

  /** Selects the hovered widget. */
  using Hover = StateSelector<void, struct HoverSelectorTag>;

  /** Selects the focused widget. */
  using Focus = StateSelector<void, struct FocusSelectorTag>;

  /** Selects the widget that has focus or has a child that has focus. */
  using FocusIn = StateSelector<void, struct FocusInSelectorTag>;

  /** Selects a widget if it was focused using a non-pointing device. */
  using FocusVisible = StateSelector<void, struct FocusVisibleSelectorTag>;

  /** Selects the Dragged widget. */
  using Drag = StateSelector<void, struct DragSelectorTag>;

  template<typename T, typename G>
  SelectConnection select(const StateSelector<T, G>& selector,
      const Stylist& base, const SelectionUpdateSignal& on_update) {
    auto connection = boost::signals2::scoped_connection(
      base.connect_match_signal(selector, [=, &base] (auto is_match) {
        if(is_match) {
          on_update({&base}, {});
        } else {
          on_update({}, {&base});
        }
      }));
    if(base.is_match(selector)) {
      on_update({&base}, {});
    }
    return SelectConnection(std::move(connection));
  }

  SelectConnection select(const Disabled& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);

  SelectConnection select(const Focus& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);

  SelectConnection select(const FocusIn& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);

  SelectConnection select(const FocusVisible& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);

  SelectConnection select(const Hover& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);

  template<typename T, typename G>
  StateSelector<T, G>::StateSelector(Type data)
    : m_data(std::move(data)) {}

  template<typename T, typename G>
  const typename StateSelector<T, G>::Type&
      StateSelector<T, G>::get_data() const {
    return m_data;
  }
}

namespace std {
  template<typename T, typename G>
  struct hash<Spire::Styles::StateSelector<T, G>> {
    std::size_t operator ()(
        const Spire::Styles::StateSelector<T, G>& selector) {
      return std::hash<T>()(selector.get_data());
    }
  };

  template<typename G>
  struct hash<Spire::Styles::StateSelector<void, G>> {
    std::size_t operator ()(
        const Spire::Styles::StateSelector<void, G>& selector) const {
      return 1;
    }
  };
}

#endif
