#ifndef SPIRE_COMPONENT_SELECTOR_HPP
#define SPIRE_COMPONENT_SELECTOR_HPP
#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"
#include "Spire/Styles/Stylist.hpp"

class QWidget;

namespace Spire::Styles {

  /** Used to select sub-components. */
  class ComponentSelector {
    public:

      /** Stores a ComponentId. */
      template<typename T, typename G>
      ComponentSelector(ComponentId<T, G> component);

      /** Returns the underlying ComponentId's type. */
      std::type_index get_type() const;

      /** Casts the underlying ComponentId to a specified type. */
      template<typename U>
      const U& as() const;

      bool operator ==(const ComponentSelector& selector) const;

      bool operator !=(const ComponentSelector& selector) const;

    private:
      std::any m_id;
      std::function<
        bool (const ComponentSelector&, const ComponentSelector&)> m_is_equal;
  };

  /** Returns the hash value of a ComponentSelector. */
  std::size_t hash_value(const ComponentSelector& element);

  /**
   * Used to identify a sub-component.
   * @param <T> The type of data associated with the element.
   * @param <G> A unique type tag.
   */
  template<typename T, typename G>
  class ComponentId {
    public:

      /** The type of data associated with the element. */
      using Type = T;

      /** A unique tag used to identify the element. */
      using Tag = G;

      /**
       * Constructs a ComponentId.
       * @param data The data associated with the element.
       */
      explicit ComponentId(Type data);

      /** Returns the associated data. */
      const Type& get_data() const;

      bool operator ==(const ComponentId& id) const;

      bool operator !=(const ComponentId& id) const;

    private:
      Type m_data;
  };

  template<typename G>
  class ComponentId<void, G> {
    public:
      using Tag = G;
      bool operator ==(const ComponentId& id) const;
      bool operator !=(const ComponentId& id) const;
  };

  /** A commonly used id to identify a component's body. */
  using Body = ComponentId<void, struct BodyTag>;

  SelectConnection select(const ComponentSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update);

namespace Details {
  bool register_id(std::type_index type, std::function<SelectConnection (
    const ComponentSelector&, const Stylist&, const SelectionUpdateSignal&)>
    callable);

  template<typename I>
  struct ComponentRegistry {
    using Id = I;
    using SelectCallable = std::function<SelectConnection (
      const QWidget&, const Id&, const SelectionUpdateSignal&)>;
    static inline std::unordered_map<std::type_index, SelectCallable>
      m_registry;
    static bool register_id(
      std::type_index widget, std::type_index id, SelectCallable callable);
    static SelectConnection select(const Id& id, const Stylist& base,
      const SelectionUpdateSignal& on_update);
  };

  template<typename I>
  bool ComponentRegistry<I>::register_id(
      std::type_index widget, std::type_index id, SelectCallable callable) {
    m_registry.insert(std::pair(widget, std::move(callable)));
    return Details::register_id(id,
      [] (const ComponentSelector& selector, const Stylist& base,
          const SelectionUpdateSignal& on_update) {
        return ComponentRegistry::select(selector.as<Id>(), base, on_update);
      });
  }

  template<typename I>
  SelectConnection ComponentRegistry<I>::select(const Id& id,
      const Stylist& base, const SelectionUpdateSignal& on_update) {
    auto i = m_registry.find(typeid(base.get_widget()));
    if(i != m_registry.end()) {
      return i->second(base.get_widget(), id, on_update);
    }
    return {};
  }
}

  template<typename W, typename I>
  struct ComponentFinder {};

  template<typename W, typename I>
  struct BaseComponentFinder : Details::ComponentRegistry<I> {
    using Widget = W;
    using Id = I;

    SelectConnection operator ()(const Widget& widget, const Id& id,
      const SelectionUpdateSignal& on_update) const;
    static bool register_id();
    static inline const auto initializer = register_id();
  };

  template<typename W, typename I>
  bool BaseComponentFinder<W, I>::register_id() {
    return Details::ComponentRegistry<Id>::register_id(
      typeid(Widget), typeid(Id), [] (const QWidget& widget, const Id& id,
          const SelectionUpdateSignal& on_update) {
        return BaseComponentFinder()(
          static_cast<const Widget&>(widget), id, on_update);
      });
  }

  template<typename T, typename G>
  SelectConnection select(const ComponentId<T, G>& id, const Stylist& base,
      const SelectionUpdateSignal& on_update) {
    return Details::ComponentRegistry<ComponentId<T, G>>::select(
      id, base, on_update);
  }

  template<typename T, typename G>
  ComponentSelector::ComponentSelector(ComponentId<T, G> id)
    : m_id(std::move(id)),
      m_is_equal(
        [] (const ComponentSelector& left, const ComponentSelector& right) {
          return left.get_type() == right.get_type() &&
            left.as<ComponentId<T, G>>() == right.as<ComponentId<T, G>>();
        }) {}

  template<typename U>
  const U& ComponentSelector::as() const {
    return std::any_cast<const U&>(m_id);
  }

  template<typename T, typename G>
  ComponentId<T, G>::ComponentId(Type data)
    : m_data(std::move(data)) {}

  template<typename T, typename G>
  const typename ComponentId<T, G>::Type& ComponentId<T, G>::get_data() const {
    return m_data;
  }

  template<typename T, typename G>
  bool ComponentId<T, G>::operator ==(const ComponentId& id) const {
    return m_data == id.m_data;
  }

  template<typename T, typename G>
  bool ComponentId<T, G>::operator !=(const ComponentId& id) const {
    return !(*this == id);
  }

  template<typename G>
  bool ComponentId<void, G>::operator ==(const ComponentId& id) const {
    return true;
  }

  template<typename G>
  bool ComponentId<void, G>::operator !=(const ComponentId& id) const {
    return !(*this == id);
  }
}

namespace std {
  template<>
  struct hash<Spire::Styles::ComponentSelector> {
    std::size_t operator ()(const Spire::Styles::ComponentSelector& id) const;
  };
}

#endif
