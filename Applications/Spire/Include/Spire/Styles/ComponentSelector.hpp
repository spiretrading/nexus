#ifndef SPIRE_COMPONENT_SELECTOR_HPP
#define SPIRE_COMPONENT_SELECTOR_HPP
#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
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
      friend std::unordered_set<Stylist*>
        select(const ComponentSelector&, std::unordered_set<Stylist*>);
      std::any m_id;
      std::function<
        bool (const ComponentSelector&, const ComponentSelector&)> m_is_equal;
      std::function<std::unordered_set<Stylist*> (
        const ComponentSelector&, std::unordered_set<Stylist*>)> m_select;
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

  template<typename I>
  struct ComponentRegistry {
    using Id = I;
    using SelectCallable =
      std::function<std::unordered_set<Stylist*> (QWidget&, const Id&)>;

    static bool register_id(std::type_index widget, std::type_index id,
      SelectCallable callable);
    static std::unordered_set<Stylist*> select(const Id& id, Stylist&);

    static inline
      std::unordered_map<std::type_index, SelectCallable> m_registry;
  };

  template<typename W, typename I>
  struct BaseComponentFinder : ComponentRegistry<I> {
    using Widget = W;
    using Id = I;

    std::unordered_set<Stylist*>
      operator ()(Widget& widget, const Id& id) const;
    static bool register_id();

    static inline const auto initializer = register_id();
  };

  template<typename W, typename I>
  struct ComponentFinder {};

namespace Details {
  bool register_id(std::type_index type, std::function<std::unordered_set<
    Stylist*> (const ComponentSelector&, Stylist&)> callable);
}

  template<typename I>
  bool ComponentRegistry<I>::register_id(
      std::type_index widget, std::type_index id, SelectCallable callable) {
    m_registry.insert(std::pair(widget, std::move(callable)));
    return Details::register_id(id,
      [] (const ComponentSelector& selector, Stylist& source) {
        return ComponentRegistry::select(selector.as<Id>(), source);
      });
  }

  template<typename I>
  std::unordered_set<Stylist*>
      ComponentRegistry<I>::select(const Id& id, Stylist& source) {
    auto i = m_registry.find(typeid(source.get_widget()));
    if(i != m_registry.end()) {
      return i->second(source.get_widget(), id);
    }
    return {};
  }

  std::unordered_set<Stylist*> select(
    const ComponentSelector& selector, std::unordered_set<Stylist*> sources);

  template<typename T, typename G>
  std::unordered_set<Stylist*>
      select(const ComponentId<T, G>& id, Stylist& source) {
    return ComponentRegistry<ComponentId<T, G>>::select(id, source);
  }

  template<typename T, typename G>
  std::unordered_set<Stylist*> select(
      const ComponentId<T, G>& id, std::unordered_set<Stylist*> sources) {
    auto selection = std::unordered_set<Stylist*>();
    for(auto source : sources) {
      auto source_selection =
        ComponentRegistry<ComponentId<T, G>>::select(id, *source);
      selection.insert(source_selection.begin(), source_selection.end());
    }
    return selection;
  }

  template<typename W, typename I>
  bool BaseComponentFinder<W, I>::register_id() {
    return ComponentRegistry<Id>::register_id(typeid(Widget), typeid(Id),
      [] (QWidget& widget, const Id& id) {
        return BaseComponentFinder()(static_cast<Widget&>(widget), id);
      });
  }

  template<typename T, typename G>
  ComponentSelector::ComponentSelector(ComponentId<T, G> id)
    : m_id(std::move(id)),
      m_is_equal(
        [] (const ComponentSelector& left, const ComponentSelector& right) {
          return left.get_type() == right.get_type() &&
            left.as<ComponentId<T, G>>() == right.as<ComponentId<T, G>>();
        }),
      m_select([] (const ComponentSelector& selector,
          std::unordered_set<Stylist*> sources) {
        return select(selector.as<ComponentId<T, G>>(), std::move(sources));
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
