#ifndef SPIRE_STYLES_BLOCK_HPP
#define SPIRE_STYLES_BLOCK_HPP
#include <utility>
#include <vector>
#include <boost/optional/optional.hpp>
#include "Spire/Styles/CompositeProperty.hpp"
#include "Spire/Styles/Property.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Stores a block of style properties. */
  class Block {
    public:

      /** Constructs an empty Block. */
      Block() = default;

      /** Returns an iterator to the first property. */
      std::vector<Property>::const_iterator begin() const;

      /** Returns an iterator to one past the last property. */
      std::vector<Property>::const_iterator end() const;

      /** Sets a Property, overwriting any previously matching Property. */
      void set(Property property);

      /** Sets a Property, overwriting any previously matching Property. */
      template<typename... T>
      void set(CompositeProperty<T...> property);

      /** Removes a Property. */
      template<typename T>
      void remove();

      /**
       * Removes a Property.
       * @param property The property to remove.
       */
      void remove(const Property& property);

      /**
       * Removes all properties contained within a CompositeProperty.
       * @param property The property to remove.
       */
      template<typename... T>
      void remove(const CompositeProperty<T...>& property);

      /** Removes all properties. */
      void clear();

      /** Tests if two Block's contain the same Properties in the same order. */
      bool operator ==(const Block& block) const;

      bool operator !=(const Block& block) const;

    private:
      std::vector<Property> m_properties;
  };

  /**
   * Merges a Block in place by another by overwriting all properties.
   * @param block The Block being updated and overwritten by the merge.
   * @param other The Block providing the properties to merge and overwrite.
   */
  void merge(Block& block, Block other);

  /**
   * Finds a property of a given type within a Block.
   * @param block The block to search.
   */
  template<typename T>
  auto find(const Block& block) {
    if constexpr(std::is_enum_v<T>) {
      return find<EnumProperty<T>>(block);
    } else if constexpr(is_composite_property_v<T>) {
      return find(block, std::in_place_type<T>);
    } else {
      for(auto& property : block) {
        if(property.get_type() == typeid(T)) {
          return boost::optional<T>(property.as<T>());
        }
      }
      return boost::optional<T>();
    }
  }

  /**
   * Finds a property of a given type within a Block.
   * @param block The block to search.
   */
  boost::optional<Property> find(
    const Block& block, const std::type_index& type);

  /**
   * Finds a property of a given type within a Block.
   * @param block The block to search.
   */
  template<typename T>
  boost::optional<CompositeProperty<T>> find(
      const Block& block, std::in_place_type_t<CompositeProperty<T>>) {
    if(auto head = find<T>(block)) {
      return CompositeProperty(std::move(*head));
    }
    return boost::none;
  }

  /**
   * Finds a property of a given type within a Block.
   * @param block The block to search.
   */
  template<typename T, typename... U>
  boost::optional<CompositeProperty<T, U...>> find(
      const Block& block, std::in_place_type_t<CompositeProperty<T, U...>>) {
    if(auto head = find<T>(block)) {
      if(auto tail = find(block, std::in_place_type<CompositeProperty<U...>>)) {
        return CompositeProperty(
          std::move(*head), std::move(tail->get<U>())...);
      }
    }
    return boost::none;
  }

  template<typename... T>
  void Block::set(CompositeProperty<T...> property) {
    for_each(std::move(property), [&] (auto&& property) {
      set(std::forward<decltype(property)>(property));
    });
  }

  template<typename T>
  void Block::remove() {
    if constexpr(is_composite_property_v<T>) {
      static_assert(false, "Not supported.");
    } else {
      auto i = std::find_if(m_properties.begin(), m_properties.end(),
        [] (const auto& property) {
          return property.get_type() == typeid(T);
        });
      if(i != m_properties.end()) {
        m_properties.erase(i);
      }
    }
  }

  template<typename... T>
  void Block::remove(const CompositeProperty<T...>& property) {
    for_each(property, [] (const auto& property) {
      remove(property);
    });
  }
}

#endif
