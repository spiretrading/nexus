#ifndef SPIRE_STATE_SELECTOR_HPP
#define SPIRE_STATE_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Represents a selector for a given state.
   * @param <T> The data associated with the state.
   * @param <G> A unique type tag.
   */
  template<typename T, typename G>
  class StateSelector {
    public:

      /** A unique tag used to identify the state. */
      using Tag = T;

      /** Constructs a StateSelector. */
      StateSelector() = default;
  };
}

#endif
