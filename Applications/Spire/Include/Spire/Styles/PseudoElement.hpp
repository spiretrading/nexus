#ifndef SPIRE_PSEUDO_ELEMENT_HPP
#define SPIRE_PSEUDO_ELEMENT_HPP
#include <utility>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Selects a specific part of a widget.
   * @param <T> The type of data associated with the element.
   * @param <G> A unique type tag.
   */
  template<typename T, typename G>
  class PseudoElement {
    public:

      /** The type of data associated with the element. */
      using Type = T;

      /** A unique tag used to identify the element. */
      using Tag = G;

      /**
       * Constructs a PseudoElement.
       * @param data The data associated with the element.
       */
      explicit PseudoElement(Type data);

      /** Returns the associated data. */
      const Type& get_data() const;

    private:
      Type m_data;
  };

  template<typename G>
  class PseudoElement<void, G> {
    public:
      using Tag = G;
  };

  template<typename T, typename G>
  PseudoElement<T, G>::PseudoElement(Type data)
    : m_data(std::move(data)) {}

  template<typename T, typename G>
  const typename PseudoElement<T, G>::Type&
      PseudoElement<T, G>::get_data() const {
    return m_data;
  }
}

#endif
