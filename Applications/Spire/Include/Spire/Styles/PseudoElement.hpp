#ifndef SPIRE_PSEUDO_ELEMENT_HPP
#define SPIRE_PSEUDO_ELEMENT_HPP
#include <any>
#include <functional>
#include <typeindex>
#include <utility>
#include "Spire/Styles/SelectConnection.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {
namespace Details {
  SelectConnection select_pseudo_element(const PseudoElement& element,
    const Stylist& base, const SelectionUpdateSignal& on_update);
}

  /** Represents a sub-section of a widget. */
  class PseudoElement {
    public:

      /** Stores a PseudoElementSelector. */
      template<typename T, typename G>
      PseudoElement(PseudoElementSelector<T, G> element);

      /** Returns the underlying PseudoElement's type. */
      std::type_index get_type() const;

      /** Casts the underlying PseudoElement to a specified type. */
      template<typename U>
      const U& as() const;

      bool operator ==(const PseudoElement& selector) const;

      bool operator !=(const PseudoElement& selector) const;

    private:
      friend SelectConnection select(
        const PseudoElement&, const Stylist&, const SelectionUpdateSignal&);
      std::any m_pseudo_element;
      std::function<bool (const PseudoElement&, const PseudoElement&)>
        m_is_equal;
      std::function<SelectConnection (
        const PseudoElement&, const Stylist&, const SelectionUpdateSignal&)>
        m_select;
  };

  /**
   * Selects a specific part of a widget.
   * @param <T> The type of data associated with the element.
   * @param <G> A unique type tag.
   */
  template<typename T, typename G>
  class PseudoElementSelector {
    public:

      /** The type of data associated with the element. */
      using Type = T;

      /** A unique tag used to identify the element. */
      using Tag = G;

      /**
       * Constructs a PseudoElement.
       * @param data The data associated with the element.
       */
      explicit PseudoElementSelector(Type data);

      /** Returns the associated data. */
      const Type& get_data() const;

      bool operator ==(const PseudoElementSelector& selector) const;

      bool operator !=(const PseudoElementSelector& selector) const;

    private:
      Type m_data;
  };

  template<typename G>
  class PseudoElementSelector<void, G> {
    public:
      using Tag = G;

      bool operator ==(const PseudoElementSelector& selector) const;

      bool operator !=(const PseudoElementSelector& selector) const;
  };

  /** Returns the hash value of a PseudoElement. */
  std::size_t hash_value(const PseudoElement& element);

  SelectConnection select(const PseudoElement& element, const Stylist& base,
    const SelectionUpdateSignal& on_update);

  template<typename T, typename G>
  SelectConnection select(const PseudoElementSelector<T, G>& selector,
      const Stylist& base, const SelectionUpdateSignal& on_update) {
    return Details::select_pseudo_element(
      PseudoElement(selector), base, on_update);
  }

  template<typename T, typename G>
  PseudoElement::PseudoElement(PseudoElementSelector<T, G> element)
    : m_pseudo_element(std::move(element)),
      m_is_equal([] (const PseudoElement& left, const PseudoElement& right) {
        return left.get_type() == right.get_type() &&
          left.as<PseudoElementSelector<T, G>>() ==
            right.as<PseudoElementSelector<T, G>>();
      }),
      m_select([] (const PseudoElement& element, const Stylist& base,
          const SelectionUpdateSignal& on_update) {
        return select(
          element.as<PseudoElementSelector<T, G>>(), base, on_update);
      }) {}

  template<typename U>
  const U& PseudoElement::as() const {
    return std::any_cast<const U&>(m_pseudo_element);
  }

  template<typename T, typename G>
  PseudoElementSelector<T, G>::PseudoElementSelector(Type data)
    : m_data(std::move(data)) {}

  template<typename T, typename G>
  const typename PseudoElementSelector<T, G>::Type&
      PseudoElementSelector<T, G>::get_data() const {
    return m_data;
  }

  template<typename T, typename G>
  bool PseudoElementSelector<T, G>::operator ==(
      const PseudoElementSelector& element) const {
    return m_data == element.m_data;
  }

  template<typename T, typename G>
  bool PseudoElementSelector<T, G>::operator !=(
      const PseudoElementSelector& element) const {
    return !(*this == element);
  }

  template<typename G>
  bool PseudoElementSelector<void, G>::operator ==(
      const PseudoElementSelector& element) const {
    return true;
  }

  template<typename G>
  bool PseudoElementSelector<void, G>::operator !=(
      const PseudoElementSelector& element) const {
    return !(*this == element);
  }
}

namespace std {
  template<>
  struct hash<Spire::Styles::PseudoElement> {
    std::size_t operator ()(const Spire::Styles::PseudoElement& element) const;
  };
}

#endif
