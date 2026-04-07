#ifndef NEXUS_REGION_MAP_HPP
#define NEXUS_REGION_MAP_HPP
#include <algorithm>
#include <deque>
#include <iterator>
#include <memory>
#include <tuple>
#include <vector>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Serialization/ShuttleTuple.hpp>
#include "Nexus/Definitions/Region.hpp"

namespace Nexus {
namespace Details {
  template<typename T>
  struct Node {
    std::tuple<const Region, T> m_element;
    std::vector<std::unique_ptr<Node>> m_sub_regions;

    Node(Region region, T value);
    Node(const Node& node);
    Node& operator =(const Node& node);
  };

  template<typename T>
  Node<T>::Node(Region region, T value)
    : m_element(std::move(region), std::move(value)) {}

  template<typename T>
  Node<T>::Node(const Node& node)
      : m_element(node.m_element) {
    std::transform(node.m_sub_regions.begin(), node.m_sub_regions.end(),
      std::back_inserter(m_sub_regions), [] (const auto& region) {
        return std::make_unique<Node>(*region);
      });
  }

  template<typename T>
  Node<T>& Node<T>::operator =(const Node& node) {
    if(this == &node) {
      return *this;
    }
    const_cast<Region&>(std::get<0>(m_element)) = std::get<0>(node.m_element);
    std::get<1>(m_element) = std::get<1>(node.m_element);
    m_sub_regions.clear();
    std::transform(node.m_sub_regions.begin(), node.m_sub_regions.end(),
      std::back_inserter(m_sub_regions), [] (const auto& region) {
        return std::make_unique<Node>(*region);
      });
    return *this;
  }
}

  /** Associates a value with a given Region. */
  template<typename T>
  class RegionMap {
    public:

      /** The type of value to associate with a Region. */
      using Element = T;

      /** Allows iterating over a RegionMap. */
      class Iterator {
        public:
          using iterator_category = std::forward_iterator_tag;

          using value_type = std::tuple<const Region, Element>;

          using difference_type = std::ptrdiff_t;

          using pointer = value_type*;

          using reference = value_type&;

          /** Constructs an empty Iterator. */
          Iterator() = default;

          /** Increments the Iterator. */
          Iterator& operator ++();

          /** Increments the Iterator. */
          Iterator operator ++(int);

          /** Tests if two Iterators refer to the same element. */
          bool operator ==(const Iterator& rhs) const;

          /** Dereferences this Iterator. */
          std::tuple<const Region, Element>& operator *();

          /** Dereferences this Iterator. */
          std::tuple<const Region, Element>* operator ->();

        private:
          friend class RegionMap<Element>;
          std::deque<Details::Node<Element>*> m_nodes;

          Iterator(Details::Node<Element>& root);
      };

      /** Allows iterating over a RegionMap. */
      class ConstIterator {
        public:
          using iterator_category = std::forward_iterator_tag;

          using value_type = const std::tuple<const Region, Element>;

          using difference_type = std::ptrdiff_t;

          using pointer = value_type*;

          using reference = value_type&;

          /** Constructs an empty ConstIterator. */
          ConstIterator() = default;

          /** Increments the ConstIterator. */
          ConstIterator& operator ++();

          /** Increments the ConstIterator. */
          ConstIterator operator ++(int);

          /** Tests if two ConstIterators refer to the same element. */
          bool operator ==(const ConstIterator& rhs) const;

          /** Dereferences this ConstIterator. */
          const std::tuple<const Region, Element>& operator *();

          /** Dereferences this ConstIterator. */
          const std::tuple<const Region, Element>* operator ->();

        private:
          friend class RegionMap<Element>;
          std::deque<const Details::Node<Element>*> m_nodes;

          ConstIterator(const Details::Node<Element>& root);
      };

      /**
       * Constructs a RegionMap.
       * @param global The value associated with the 'global' Region.
       */
      explicit RegionMap(Element global);

      /**
       * Constructs a RegionMap.
       * @param name The name of the 'global' Region.
       * @param global The value associated with the 'global' Region.
       */
      RegionMap(std::string name, Element global);

      /** Returns the number of Regions represented. */
      std::size_t get_size() const;

      /**
       * Returns the value associated with a Region.
       * @param region The Region to retrieve the associated value of.
       * @return The value associated with the <i>region</i>.
       */
      const Element& get(const Region& region) const;

      /**
       * Returns the value associated with a Region.
       * @param region The Region to retrieve the associated value of.
       * @return The value associated with the <i>region</i>.
       */
      Element& get(const Region& region);

      /**
       * Sets a value to be associated with a Region.
       * @param region The Region to associate.
       * @param value The value to associate with the <i>region</i>.
       */
      void set(const Region& region, const Element& value);

      /**
       * Erases a Region.
       * @param region The Region to delete.
       */
      void erase(const Region& region);

      /**
       * Returns an iterator to the Region that most closely encapsulates a
       * specified Region.
       * @param region The Region to find.
       * @return A ConstIterator to the Region that most closely encapsulates
       *         the specified <i>region</i>.
       */
      Iterator find(const Region& region);

      /**
       * Returns an iterator to the Region that most closely encapsulates a
       * specified Region.
       * @param region The Region to find.
       * @return A ConstIterator to the Region that most closely encapsulates
       *         the specified <i>region</i>.
       */
      ConstIterator find(const Region& region) const;

      /** Returns an Iterator to the global Region. */
      Iterator begin();

      /** Returns a ConstIterator to the global Region. */
      ConstIterator begin() const;

      /** Returns an Iterator to the end of this container. */
      Iterator end();

      /** Returns a ConstIterator to the end of this container. */
      ConstIterator end() const;

    private:
      friend struct Beam::DataShuttle;
      friend struct Beam::Shuttle<RegionMap>;
      Details::Node<Element> m_root;
      std::size_t m_size;

      RegionMap();
      void insert(Details::Node<Element>& root, Region region, Element value);
      static Details::Node<Element>& find(
        Details::Node<Element>& root, const Region& region);
      static const Details::Node<Element>& find(
        const Details::Node<Element>& root, const Region& region);
      std::pair<Details::Node<Element>*, Details::Node<Element>*> find(
        Details::Node<Element>* parent, Details::Node<Element>* root,
        const Region& region);
  };

  template<typename T>
  typename RegionMap<T>::Iterator& RegionMap<T>::Iterator::operator ++() {
    auto node = m_nodes.front();
    m_nodes.pop_front();
    std::transform(node->m_sub_regions.begin(), node->m_sub_regions.end(),
      std::back_inserter(m_nodes), [] (const auto& region) {
        return region.get();
      });
    return *this;
  }

  template<typename T>
  typename RegionMap<T>::Iterator RegionMap<T>::Iterator::operator ++(int) {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  template<typename T>
  bool RegionMap<T>::Iterator::operator ==(const Iterator& rhs) const {
    return (m_nodes.empty() && rhs.m_nodes.empty()) ||
      (!m_nodes.empty() && !rhs.m_nodes.empty() &&
      m_nodes.front() == rhs.m_nodes.front());
  }

  template<typename T>
  std::tuple<const Region, typename RegionMap<T>::Element>&
      RegionMap<T>::Iterator::operator *() {
    return m_nodes.front()->m_element;
  }

  template<typename T>
  std::tuple<const Region, typename RegionMap<T>::Element>*
      RegionMap<T>::Iterator::operator ->() {
    return &m_nodes.front()->m_element;
  }

  template<typename T>
  RegionMap<T>::Iterator::Iterator(Details::Node<Element>& root) {
    m_nodes.push_back(&root);
  }

  template<typename T>
  typename RegionMap<T>::ConstIterator&
      RegionMap<T>::ConstIterator::operator ++() {
    auto node = m_nodes.front();
    m_nodes.pop_front();
    std::transform(node->m_sub_regions.begin(), node->m_sub_regions.end(),
      std::back_inserter(m_nodes), [] (const auto& region) {
        return region.get();
      });
    return *this;
  }

  template<typename T>
  typename RegionMap<T>::ConstIterator
      RegionMap<T>::ConstIterator::operator ++(int) {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  template<typename T>
  bool RegionMap<T>::ConstIterator::operator ==(
      const ConstIterator& rhs) const {
    return (m_nodes.empty() && rhs.m_nodes.empty()) ||
      (!m_nodes.empty() && !rhs.m_nodes.empty() &&
      m_nodes.front() == rhs.m_nodes.front());
  }

  template<typename T>
  const std::tuple<const Region, typename RegionMap<T>::Element>&
      RegionMap<T>::ConstIterator::operator *() {
    return m_nodes.front()->m_element;
  }

  template<typename T>
  const std::tuple<const Region, typename RegionMap<T>::Element>*
      RegionMap<T>::ConstIterator::operator ->() {
    return &m_nodes.front()->m_element;
  }

  template<typename T>
  RegionMap<T>::ConstIterator::ConstIterator(
      const Details::Node<Element>& root) {
    m_nodes.push_back(&root);
  }

  template<typename T>
  RegionMap<T>::RegionMap(Element global)
    : m_root(Region::GLOBAL, std::move(global)),
      m_size(1) {}

  template<typename T>
  RegionMap<T>::RegionMap(std::string name, Element global)
    : m_root(Region::make_global(std::move(name)), std::move(global)),
      m_size(1) {}

  template<typename T>
  std::size_t RegionMap<T>::get_size() const {
    return m_size;
  }

  template<typename T>
  const typename RegionMap<T>::Element&
      RegionMap<T>::get(const Region& region) const {
    return std::get<1>(find(m_root, region).m_element);
  }

  template<typename T>
  typename RegionMap<T>::Element& RegionMap<T>::get(const Region& region) {
    return std::get<1>(find(m_root, region).m_element);
  }

  template<typename T>
  void RegionMap<T>::set(const Region& region, const Element& value) {
    if(region == std::get<0>(m_root.m_element)) {
      std::get<1>(m_root.m_element) = value;
      return;
    }
    insert(m_root, region, value);
  }

  template<typename T>
  void RegionMap<T>::erase(const Region& region) {
    auto node = find(nullptr, &m_root, region);
    if(std::get<0>(node.second->m_element) != region) {
      return;
    }
    if(!node.first) {
      return;
    }
    std::move(node.second->m_sub_regions.begin(),
      node.second->m_sub_regions.end(),
      std::back_inserter(node.first->m_sub_regions));
    for(auto i = node.first->m_sub_regions.begin();
        i != node.first->m_sub_regions.end(); ++i) {
      if(i->get() == node.second) {
        node.first->m_sub_regions.erase(i);
        --m_size;
        break;
      }
    }
  }

  template<typename T>
  typename RegionMap<T>::Iterator RegionMap<T>::find(const Region& region) {
    auto& node = find(m_root, region);
    return Iterator(node);
  }

  template<typename T>
  typename RegionMap<T>::ConstIterator RegionMap<T>::find(
      const Region& region) const {
    auto& node = find(m_root, region);
    return ConstIterator(node);
  }

  template<typename T>
  typename RegionMap<T>::Iterator RegionMap<T>::begin() {
    return Iterator(m_root);
  }

  template<typename T>
  typename RegionMap<T>::ConstIterator RegionMap<T>::begin() const {
    return ConstIterator(m_root);
  }

  template<typename T>
  typename RegionMap<T>::Iterator RegionMap<T>::end() {
    return Iterator();
  }

  template<typename T>
  typename RegionMap<T>::ConstIterator RegionMap<T>::end() const {
    return ConstIterator();
  }

  template<typename T>
  RegionMap<T>::RegionMap()
    : m_root(Region::GLOBAL, {}),
      m_size(1) {}

  template<typename T>
  void RegionMap<T>::insert(
      Details::Node<Element>& root, Region region, Element value) {
    auto sub_regions = std::vector<std::unique_ptr<Details::Node<Element>>>();
    auto i = root.m_sub_regions.begin();
    while(i != root.m_sub_regions.end()) {
      if(region < std::get<0>((*i)->m_element)) {
        insert(**i, std::move(region), std::move(value));
        return;
      } else if(region == std::get<0>((*i)->m_element)) {
        std::get<1>((*i)->m_element) = std::move(value);
        return;
      } else if(region > std::get<0>((*i)->m_element)) {
        sub_regions.push_back(std::move(*i));
        i = root.m_sub_regions.erase(i);
      } else {
        ++i;
      }
    }
    auto sub_region = std::make_unique<Details::Node<Element>>(
      std::move(region), std::move(value));
    sub_region->m_sub_regions.swap(sub_regions);
    root.m_sub_regions.push_back(std::move(sub_region));
    ++m_size;
  }

  template<typename T>
  Details::Node<typename RegionMap<T>::Element>& RegionMap<T>::find(
      Details::Node<Element>& root, const Region& region) {
    for(auto& sub_region : root.m_sub_regions) {
      if(region <= std::get<0>(sub_region->m_element)) {
        return find(*sub_region, region);
      }
    }
    return root;
  }

  template<typename T>
  const Details::Node<typename RegionMap<T>::Element>& RegionMap<T>::find(
      const Details::Node<Element>& root, const Region& region) {
    return find(const_cast<Details::Node<Element>&>(root), region);
  }

  template<typename T>
  std::pair<Details::Node<typename RegionMap<T>::Element>*,
      Details::Node<typename RegionMap<T>::Element>*> RegionMap<T>::find(
      Details::Node<Element>* parent, Details::Node<Element>* root,
      const Region& region) {
    for(auto& sub_region : root->m_sub_regions) {
      if(region <= std::get<0>(sub_region->m_element)) {
        return find(root, sub_region.get(), region);
      }
    }
    return {parent, root};
  }
}

namespace Beam {
  template<typename T>
  constexpr auto is_structure<Nexus::RegionMap<T>> = false;

  template<typename T>
  struct Send<Nexus::RegionMap<T>> {
    template<IsSender S>
    void operator ()(S& sender, const char* name,
        const Nexus::RegionMap<T>& value) const {
      sender.start_sequence(name, static_cast<int>(value.get_size()));
      for(auto i = value.begin(); i != value.end(); ++i) {
        sender.send(*i);
      }
      sender.end_sequence();
    }
  };

  template<typename T>
  struct Receive<Nexus::RegionMap<T>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, Nexus::RegionMap<T>& value) const {
      auto size = 0;
      receiver.start_sequence(name, size);
      for(auto i = 0; i < size; ++i) {
        auto entry = std::tuple<Nexus::Region, T>();
        receiver.shuttle(entry);
        value.set(
          std::move(std::get<0>(entry)), std::move(std::get<1>(entry)));
      }
      receiver.end_sequence();
    }
  };
}

#endif
