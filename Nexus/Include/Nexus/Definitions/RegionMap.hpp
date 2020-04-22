#ifndef NEXUS_REGIONMAP_HPP
#define NEXUS_REGIONMAP_HPP
#include <deque>
#include <iterator>
#include <tuple>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Region.hpp"

namespace Nexus {
namespace Details {
  template<typename T>
  struct Node {
    std::tuple<const Region, T> m_element;
    std::vector<std::unique_ptr<Node>> m_subRegions;

    Node(const Region& region, const T& value);
    Node(const Node& node);
    Node& operator =(const Node& node);
  };

  template<typename T>
  Node<T>::Node(const Region& region, const T& value)
      : m_element(region, value) {}

  template<typename T>
  Node<T>::Node(const Node& node)
      : m_element(node.m_element) {
    for(auto i = node.m_subRegions.begin(); i != node.m_subRegions.end(); ++i) {
      m_subRegions.push_back(std::make_unique<Node>(**i));
    }
  }

  template<typename T>
  Node<T>& Node<T>::operator =(const Node& node) {
    if(this == &node) {
      return *this;
    }
    const_cast<Region&>(std::get<0>(m_element)) = std::get<0>(node.m_element);
    std::get<1>(m_element) = std::get<1>(node.m_element);
    m_subRegions.clear();
    for(auto i = node.m_subRegions.begin(); i != node.m_subRegions.end(); ++i) {
      m_subRegions.push_back(std::make_unique<Node>(**i));
    }
    return *this;
  }
}

  /*! \class RegionMap
      \brief Associates a value with a given Region.
   */
  template<typename T>
  class RegionMap {
    public:

      //! The type of value to associate with a Region.
      typedef T Element;

      /*! \class Iterator
          \brief Allows iterating over a RegionMap.
       */
      class Iterator : public std::iterator<std::forward_iterator_tag,
          std::tuple<const Region, T>> {
        public:

          //! Constructs an empty Iterator.
          Iterator();

          //! Increments the Iterator.
          Iterator& operator ++();

          //! Increments the Iterator.
          Iterator operator ++(int);

          //! Tests if two Iterators refer to the same element.
          bool operator ==(const Iterator& rhs);

          //! Tests if two Iterators refer to different elements.
          bool operator !=(const Iterator& rhs);

          //! Dereferences this Iterator.
          std::tuple<const Region, T>& operator *();

        private:
          friend class RegionMap<T>;
          std::deque<Details::Node<T>*> m_nodes;

          Iterator(Details::Node<T>& root);
      };

      /*! \class ConstIterator
          \brief Allows iterating over a RegionMap.
       */
      class ConstIterator : public std::iterator<std::forward_iterator_tag,
          const std::tuple<const Region, T>> {
        public:

          //! Constructs an empty ConstIterator.
          ConstIterator();

          //! Increments the ConstIterator.
          ConstIterator& operator ++();

          //! Increments the ConstIterator.
          ConstIterator operator ++(int);

          //! Tests if two ConstIterators refer to the same element.
          bool operator ==(const ConstIterator& rhs);

          //! Tests if two ConstIterators refer to different elements.
          bool operator !=(const ConstIterator& rhs);

          //! Dereferences this ConstIterator.
          const std::tuple<const Region, T>& operator *();

        private:
          friend class RegionMap<T>;
          std::deque<const Details::Node<T>*> m_nodes;

          ConstIterator(const Details::Node<T>& root);
      };

      //! Constructs a RegionMap.
      /*!
        \param globalValue The value associated with the 'global' Region.
      */
      explicit RegionMap(const T& globalValue);

      //! Constructs a RegionMap.
      /*!
        \param name The name of the 'global' Region.
        \param globalValue The value associated with the 'global' Region.
      */
      RegionMap(const std::string& name, const T& globalValue);

      //! Returns the number of Regions represented.
      std::size_t GetSize() const;

      //! Returns the value associated with a Region.
      /*!
        \param region The Region to retrieve the associated value of.
        \return The value associated with the <i>region</i>.
      */
      const T& Get(const Region& region) const;

      //! Returns the value associated with a Region.
      /*!
        \param region The Region to retrieve the associated value of.
        \return The value associated with the <i>region</i>.
      */
      T& Get(const Region& region);

      //! Sets a value to be associated with a Region.
      /*!
        \param region The Region to associate.
        \param value The value to associate with the <i>region</i>.
      */
      void Set(const Region& region, const T& value);

      //! Erases a Region.
      /*!
        \param region The Region to delete.
      */
      void Erase(const Region& region);

      //! Returns an iterator to the Region that most closely encapsulates a
      //! specified Region.
      /*!
        \param region The Region to find.
        \return A ConstIterator to the Region that most closely encapsulates the
                specified <i>region</i>.
      */
      Iterator Find(const Region& region);

      //! Returns an iterator to the Region that most closely encapsulates a
      //! specified Region.
      /*!
        \param region The Region to find.
        \return A ConstIterator to the Region that most closely encapsulates the
                specified <i>region</i>.
      */
      ConstIterator Find(const Region& region) const;

      //! Returns an Iterator to the global Region.
      Iterator Begin();

      //! Returns a ConstIterator to the global Region.
      ConstIterator Begin() const;

      //! Returns an Iterator to the end of this container.
      Iterator End();

      //! Returns a ConstIterator to the end of this container.
      ConstIterator End() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      friend struct Beam::Serialization::Shuttle<RegionMap>;
      Details::Node<T> m_root;
      std::size_t m_size;

      RegionMap(Beam::Serialization::ReceiveBuilder);
      void Insert(Details::Node<T>& root, const Region& region, const T& value);
      static Details::Node<T>& Find(Details::Node<T>& root,
        const Region& region);
      static const Details::Node<T>& Find(const Details::Node<T>& root,
        const Region& region);
      std::pair<Details::Node<T>*, Details::Node<T>*> FindPair(
        Details::Node<T>* parent, Details::Node<T>* root, const Region& region);
  };

  template<typename T>
  RegionMap<T>::Iterator::Iterator() {}

  template<typename T>
  typename RegionMap<T>::Iterator& RegionMap<T>::Iterator::operator ++() {
    Details::Node<T>* node = m_nodes.front();
    m_nodes.pop_front();
    for(auto i = node->m_subRegions.begin();
        i != node->m_subRegions.end(); ++i) {
      m_nodes.push_back(i->get());
    }
    return *this;
  }

  template<typename T>
  typename RegionMap<T>::Iterator RegionMap<T>::Iterator::operator ++(int) {
    Iterator tmp(*this);
    operator ++();
    return tmp;
  }

  template<typename T>
  bool RegionMap<T>::Iterator::operator ==(const Iterator& rhs) {
    return (m_nodes.empty() && rhs.m_nodes.empty()) ||
      (!m_nodes.empty() && !rhs.m_nodes.empty() &&
      m_nodes.front() == rhs.m_nodes.front());
  }

  template<typename T>
  bool RegionMap<T>::Iterator::operator !=(const Iterator& rhs) {
    return !(*this == rhs);
  }

  template<typename T>
  std::tuple<const Region, T>& RegionMap<T>::Iterator::operator *() {
    return static_cast<std::tuple<const Region, T>&>(
      m_nodes.front()->m_element);
  }

  template<typename T>
  RegionMap<T>::Iterator::Iterator(Details::Node<T>& root) {
    m_nodes.push_back(&root);
  }

  template<typename T>
  RegionMap<T>::ConstIterator::ConstIterator() {}

  template<typename T>
  typename RegionMap<T>::ConstIterator& RegionMap<T>::ConstIterator::
      operator ++() {
    const Details::Node<T>* node = m_nodes.front();
    m_nodes.pop_front();
    for(auto i = node->m_subRegions.begin();
        i != node->m_subRegions.end(); ++i) {
      m_nodes.push_back(i->get());
    }
    return *this;
  }

  template<typename T>
  typename RegionMap<T>::ConstIterator RegionMap<T>::ConstIterator::
      operator ++(int) {
    ConstIterator tmp(*this);
    operator ++();
    return tmp;
  }

  template<typename T>
  bool RegionMap<T>::ConstIterator::operator ==(const ConstIterator& rhs) {
    return (m_nodes.empty() && rhs.m_nodes.empty()) ||
      (!m_nodes.empty() && !rhs.m_nodes.empty() &&
      m_nodes.front() == rhs.m_nodes.front());
  }

  template<typename T>
  bool RegionMap<T>::ConstIterator::operator !=(const ConstIterator& rhs) {
    return !(*this == rhs);
  }

  template<typename T>
  const std::tuple<const Region, T>& RegionMap<T>::ConstIterator::operator *() {
    return m_nodes.front()->m_element;
  }

  template<typename T>
  RegionMap<T>::ConstIterator::ConstIterator(const Details::Node<T>& root) {
    m_nodes.push_back(&root);
  }

  template<typename T>
  RegionMap<T>::RegionMap(const T& globalValue)
    : m_root(Region::Global(), globalValue),
      m_size(1) {}

  template<typename T>
  RegionMap<T>::RegionMap(const std::string& name, const T& globalValue)
    : m_root(Region::Global(name), globalValue),
      m_size(1) {}

  template<typename T>
  std::size_t RegionMap<T>::GetSize() const {
    return m_size;
  }

  template<typename T>
  const T& RegionMap<T>::Get(const Region& region) const {
    return std::get<1>(Find(m_root, region).m_element);
  }

  template<typename T>
  T& RegionMap<T>::Get(const Region& region) {
    return std::get<1>(Find(m_root, region).m_element);
  }

  template<typename T>
  void RegionMap<T>::Set(const Region& region, const T& value) {
    if(region == std::get<0>(m_root.m_element)) {
      std::get<1>(m_root.m_element) = value;
      return;
    }
    Insert(m_root, region, value);
  }

  template<typename T>
  void RegionMap<T>::Erase(const Region& region) {
    std::pair<Details::Node<T>*, Details::Node<T>*> node =
      FindPair(nullptr, &m_root, region);
    if(std::get<0>(node.second->m_element) != region) {
      return;
    }
    if(node.first == nullptr) {
      return;
    }
    for(auto i = node.second->m_subRegions.begin();
        i != node.second->m_subRegions.end(); ++i) {
      node.first->m_subRegions.push_back(std::move(*i));
    }
    for(auto i = node.first->m_subRegions.begin();
        i != node.first->m_subRegions.end(); ++i) {
      if(i->get() == node.second) {
        node.first->m_subRegions.erase(i);
        --m_size;
        break;
      }
    }
  }

  template<typename T>
  typename RegionMap<T>::Iterator RegionMap<T>::Find(const Region& region) {
    Details::Node<T>& node = Find(m_root, region);
    return Iterator(node);
  }

  template<typename T>
  typename RegionMap<T>::ConstIterator RegionMap<T>::Find(
      const Region& region) const {
    const Details::Node<T>& node = Find(m_root, region);
    return ConstIterator(node);
  }

  template<typename T>
  typename RegionMap<T>::Iterator RegionMap<T>::Begin() {
    return Iterator(m_root);
  }

  template<typename T>
  typename RegionMap<T>::ConstIterator RegionMap<T>::Begin() const {
    return ConstIterator(m_root);
  }

  template<typename T>
  typename RegionMap<T>::Iterator RegionMap<T>::End() {
    return Iterator();
  }

  template<typename T>
  typename RegionMap<T>::ConstIterator RegionMap<T>::End() const {
    return ConstIterator();
  }

  template<typename T>
  RegionMap<T>::RegionMap(Beam::Serialization::ReceiveBuilder)
    : m_root(Region::Global(), T()),
      m_size(1) {}

  template<typename T>
  void RegionMap<T>::Insert(Details::Node<T>& root, const Region& region,
      const T& value) {
    std::vector<std::unique_ptr<Details::Node<T>>> subRegions;
    auto i = root.m_subRegions.begin();
    while(i != root.m_subRegions.end()) {
      if(region < std::get<0>((*i)->m_element)) {
        Insert(**i, region, value);
        return;
      } else if(region == std::get<0>((*i)->m_element)) {
        std::get<1>((*i)->m_element) = value;
        return;
      } else if(region > std::get<0>((*i)->m_element)) {
        subRegions.push_back(std::move(*i));
        i = root.m_subRegions.erase(i);
      } else {
        ++i;
      }
    }
    auto subRegion = std::make_unique<Details::Node<T>>(region, value);
    subRegion->m_subRegions.swap(subRegions);
    root.m_subRegions.push_back(std::move(subRegion));
    ++m_size;
  }

  template<typename T>
  Details::Node<T>& RegionMap<T>::Find(Details::Node<T>& root,
      const Region& region) {
    for(auto i = root.m_subRegions.begin(); i != root.m_subRegions.end(); ++i) {
      if(region <= std::get<0>((*i)->m_element)) {
        return Find(**i, region);
      }
    }
    return root;
  }

  template<typename T>
  const Details::Node<T>& RegionMap<T>::Find(const Details::Node<T>& root,
      const Region& region) {
    return Find(const_cast<Details::Node<T>&>(root), region);
  }

  template<typename T>
  std::pair<Details::Node<T>*, Details::Node<T>*> RegionMap<T>::FindPair(
      Details::Node<T>* parent, Details::Node<T>* root, const Region& region) {
    for(auto i = root->m_subRegions.begin();
        i != root->m_subRegions.end(); ++i) {
      if(region <= std::get<0>((*i)->m_element)) {
        return FindPair(root, i->get(), region);
      }
    }
    return std::make_pair(parent, root);
  }
}

namespace Beam {
namespace Serialization {
  template<typename T>
  struct IsStructure<Nexus::RegionMap<T>> : std::false_type {};

  template<typename T>
  struct IsDefaultConstructable<Nexus::RegionMap<T>> : std::false_type {};

  template<typename T>
  struct Send<Nexus::RegionMap<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const Nexus::RegionMap<T>& value) const {
      shuttle.StartSequence(name, static_cast<int>(value.GetSize()));
      for(auto i = value.Begin(); i != value.End(); ++i) {
        shuttle.Shuttle(*i);
      }
      shuttle.EndSequence();
    }
  };

  template<typename T>
  struct Receive<Nexus::RegionMap<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::RegionMap<T>& value) const {
      int size;
      shuttle.StartSequence(name, size);
      for(int i = 0; i < size; ++i) {
        std::tuple<Nexus::Region, T> entry;
        shuttle.Shuttle(entry);
        value.Set(std::get<0>(entry), std::get<1>(entry));
      }
      shuttle.EndSequence();
    }
  };
}
}

#endif
