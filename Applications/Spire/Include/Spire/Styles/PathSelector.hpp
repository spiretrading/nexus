#ifndef SPIRE_PATH_SELECTOR_HPP
#define SPIRE_PATH_SELECTOR_HPP
#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include "Spire/Styles/Styles.hpp"
#include "Spire/Styles/Stylist.hpp"

class QWidget;

namespace Spire::Styles {

  /** Used to select a specific sub-component identified through a path. */
  class PathSelector {
    public:

      /** Stores a Path. */
      template<typename T, typename G>
      PathSelector(Path<T, G> path);

      /** Returns the underlying Path's type. */
      std::type_index get_type() const;

      /** Casts the underlying path to a specified type. */
      template<typename U>
      const U& as() const;

      bool operator ==(const PathSelector& selector) const;

      bool operator !=(const PathSelector& selector) const;

    private:
      static std::unordered_map<std::type_index,
        std::vector<Stylist*> (*)(void*, void*)> m_path_selectors;
      friend std::vector<Stylist*> select(const PathSelector&, Stylist&);
      std::any m_path;
      std::function<bool (const PathSelector&, const PathSelector&)> m_is_equal;
      std::function<std::vector<Stylist*> (const PathSelector&, Stylist&)>
        m_select;
  };

  /** Returns the hash value of a PathSelector. */
  std::size_t hash_value(const PathSelector& element);

  /**
   * Specifies a path to a sub-component.
   * @param <T> The type of data associated with the element.
   * @param <G> A unique type tag.
   */
  template<typename T, typename G>
  class Path {
    public:

      /** The type of data associated with the element. */
      using Type = T;

      /** A unique tag used to identify the element. */
      using Tag = G;

      /**
       * Constructs a Path.
       * @param data The data associated with the element.
       */
      explicit Path(Type data);

      /** Returns the associated data. */
      const Type& get_data() const;

      bool operator ==(const Path& path) const;

      bool operator !=(const Path& path) const;

    private:
      Type m_data;
  };

  template<typename G>
  class Path<void, G> {
    public:
      using Tag = G;

      bool operator ==(const Path& selector) const;

      bool operator !=(const Path& selector) const;
  };

  /** A commonly used path to identify the main body of a component. */
  using Body = Path<void, struct BodyTag>;

  template<typename P>
  struct PathFinderRegistry {
    using Path = P;
    using SelectCallable =
      std::function<std::vector<Stylist*> (QWidget&, const Path&)>;

    static bool register_path(std::type_index widget, std::type_index path,
      SelectCallable callable);
    static std::vector<Stylist*> select(const Path& path, Stylist& source);

    static inline
      std::unordered_map<std::type_index, SelectCallable> m_registry;
  };

  template<typename W, typename P>
  struct BasePathFinder : PathFinderRegistry<P> {
    using Widget = W;
    using Path = P;

    std::vector<Stylist*> operator ()(Widget& widget, const Path& path) const;
    static bool register_path();

    static inline const auto initializer = register_path();
  };

  template<typename W, typename P>
  struct PathFinder {};

namespace Details {
  bool register_path(std::type_index type,
    std::function<std::vector<Stylist*> (const PathSelector&, Stylist&)>);
}

  template<typename P>
  bool PathFinderRegistry<P>::register_path(
      std::type_index widget, std::type_index path, SelectCallable callable) {
    m_registry.insert(std::pair(widget, std::move(callable)));
    return Details::register_path(path,
      [] (const PathSelector& selector, Stylist& source) {
        return PathFinderRegistry::select(selector.as<Path>(), source);
      });
  }

  template<typename P>
  std::vector<Stylist*>
      PathFinderRegistry<P>::select(const Path& path, Stylist& source) {
    auto i = m_registry.find(typeid(source.get_widget()));
    if(i == m_registry.end()) {
      return {};
    }
    return i->second(source.get_widget(), path);
  }

  std::vector<Stylist*> select(const PathSelector& selector, Stylist& source);

  template<typename T, typename G>
  std::vector<Stylist*> select(const Path<T, G>& path, Stylist& source) {
    return PathFinderRegistry<Path<T, G>>::select(path, source);
  }

  template<typename W, typename P>
  bool BasePathFinder<W, P>::register_path() {
    return PathFinderRegistry::register_path(typeid(Widget), typeid(Path),
      [] (QWidget& widget, const Path& path) {
        return BasePathFinder()(static_cast<Widget&>(widget), path);
      });
  }

  template<typename T, typename G>
  PathSelector::PathSelector(Path<T, G> path)
    : m_path(std::move(path)),
      m_is_equal([] (const PathSelector& left, const PathSelector& right) {
        return left.get_type() == right.get_type() &&
          left.as<Path<T, G>>() == right.as<Path<T, G>>();
      }),
      m_select([] (const PathSelector& element, Stylist& stylist) {
        return select(element.as<Path<T, G>>(), stylist);
      }) {}

  template<typename U>
  const U& PathSelector::as() const {
    return std::any_cast<const U&>(m_path);
  }

  template<typename T, typename G>
  Path<T, G>::Path(Type data)
    : m_data(std::move(data)) {}

  template<typename T, typename G>
  const typename Path<T, G>::Type& Path<T, G>::get_data() const {
    return m_data;
  }

  template<typename T, typename G>
  bool Path<T, G>::operator ==(const Path& path) const {
    return m_data == path.m_data;
  }

  template<typename T, typename G>
  bool Path<T, G>::operator !=(const Path& path) const {
    return !(*this == path);
  }

  template<typename G>
  bool Path<void, G>::operator ==(const Path& path) const {
    return true;
  }

  template<typename G>
  bool Path<void, G>::operator !=(const Path& path) const {
    return !(*this == path);
  }
}

namespace std {
  template<>
  struct hash<Spire::Styles::PathSelector> {
    std::size_t operator ()(const Spire::Styles::PathSelector& path) const;
  };
}

#endif
