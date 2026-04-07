#ifndef SPIRE_PSEUDO_ELEMENT_HPP
#define SPIRE_PSEUDO_ELEMENT_HPP
#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include "Spire/Styles/SelectConnection.hpp"

namespace Spire::Styles {
  class PseudoElement;
  template<typename T, typename G> class PseudoElementSelector;

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

    private:
      friend SelectConnection select(
        const PseudoElement&, const Stylist&, const SelectionUpdateSignal&);
      friend struct std::hash<PseudoElement>;
      struct Operations {
        std::function<bool (const PseudoElement&, const PseudoElement&)>
          m_is_equal;
        std::function<SelectConnection (
          const PseudoElement&, const Stylist&, const SelectionUpdateSignal&)>
          m_select;
        std::function<std::size_t (const PseudoElement&)> m_hash;
      };
      static std::unordered_map<std::type_index, Operations> m_operations;
      std::any m_pseudo_element;
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

      bool operator ==(const PseudoElementSelector& selector) const = default;

    private:
      Type m_data;
  };

  template<typename G>
  class PseudoElementSelector<void, G> {
    public:
      using Tag = G;

      bool operator ==(const PseudoElementSelector& selector) const = default;
  };

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
      : m_pseudo_element(std::move(element)) {
    auto operations = m_operations.find(typeid(PseudoElementSelector<T, G>));
    if(operations == m_operations.end()) {
      m_operations.emplace_hint(operations, typeid(PseudoElementSelector<T, G>),
        Operations(
          [] (const PseudoElement& left, const PseudoElement& right) {
            return left.get_type() == right.get_type() &&
              left.as<PseudoElementSelector<T, G>>() ==
                right.as<PseudoElementSelector<T, G>>();
          },
          [] (const PseudoElement& element, const Stylist& base,
              const SelectionUpdateSignal& on_update) {
            return select(
              element.as<PseudoElementSelector<T, G>>(), base, on_update);
          },
          [] (const PseudoElement& element) {
            return std::hash<PseudoElementSelector<T, G>>()(
              element.as<PseudoElementSelector<T, G>>());
          }));
    }
  }

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
}

namespace std {
  template<>
  struct hash<Spire::Styles::PseudoElement> {
    std::size_t operator ()(
      const Spire::Styles::PseudoElement& element) const noexcept;
  };

  template<typename T, typename G>
  struct hash<Spire::Styles::PseudoElementSelector<T, G>> {
    std::size_t operator ()(
        const Spire::Styles::PseudoElementSelector<T, G>& selector)
          const noexcept {
      return std::hash<T>()(selector.get_data());
    }
  };

  template<typename G>
  struct hash<Spire::Styles::PseudoElementSelector<void, G>> {
    std::size_t operator ()(
        const Spire::Styles::PseudoElementSelector<void, G>& selector)
          const noexcept {
      return 0xd0970fbcaa28c137;
    }
  };
}

#endif
