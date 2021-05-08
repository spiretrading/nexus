#ifndef SPIRE_PATH_SELECTOR_HPP
#define SPIRE_PATH_SELECTOR_HPP
#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include "Spire/Styles/Styles.hpp"

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

  template<typename P>
  struct BasePathFinder {
    static find_registry
  };

  template<typename W, typename P>
  struct PathFinderRegistry : BasePathFinder<P> {
    using Widget = W;
    using Path = P;

    private:
      static bool register_path();
      static const auto initializer = register_path();
  };

  template<typename W, typename P>
  struct PathFinder {};

  /** A commonly used path to identify the main body of a component. */
  using Body = Path<void, struct BodyTag>;

  /** Returns the hash value of a PathSelector. */
  std::size_t hash_value(const PathSelector& element);

  std::vector<Stylist*> select(const PathSelector& selector, Stylist& source);

  template<typename T, typename G>
  std::vector<Stylist*> select(const Path<T, G>& selector, Stylist& source) {
    BasePathFinder<Path<T, G>>::find_registry(typeid(*source.m_widget));
    return {};
  }

  template<typename T, typename G>
  PathSelector::PathSelector(Path<T, G> path)
    : m_path(std::move(path)),
      m_is_equal([] (const PathSelector& left, const PathSelector& right) {
        if(left.get_type() != right.get_type()) {
          return false;
        }
        return left.as<Path<T, G>>() == right.as<Path<T, G>>();
      }),
      m_select([] (const PathSelector& element, Stylist& stylist) {
        return select(element.as<PathSelector<T, G>>(), stylist);
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
    auto operator ()(const Spire::Styles::PathSelector& path) const {
      return Spire::Styles::hash_value(path);
    }
  };
}

#endif
