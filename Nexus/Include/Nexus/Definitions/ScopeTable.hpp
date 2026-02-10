#ifndef NEXUS_SCOPE_TABLE_HPP
#define NEXUS_SCOPE_TABLE_HPP
#include <algorithm>
#include <deque>
#include <iterator>
#include <memory>
#include <tuple>
#include <vector>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Serialization/ShuttleTuple.hpp>
#include "Nexus/Definitions/Scope.hpp"

namespace Nexus {
namespace Details {
  template<typename T>
  struct Node {
    std::tuple<const Scope, T> m_element;
    std::vector<std::unique_ptr<Node>> m_sub_scopes;

    Node(Scope scope, T value);
    Node(const Node& node);
    Node& operator =(const Node& node);
  };

  template<typename T>
  Node<T>::Node(Scope scope, T value)
    : m_element(std::move(scope), std::move(value)) {}

  template<typename T>
  Node<T>::Node(const Node& node)
      : m_element(node.m_element) {
    std::transform(node.m_sub_scopes.begin(), node.m_sub_scopes.end(),
      std::back_inserter(m_sub_scopes), [] (const auto& scope) {
        return std::make_unique<Node>(*scope);
      });
  }

  template<typename T>
  Node<T>& Node<T>::operator =(const Node& node) {
    if(this == &node) {
      return *this;
    }
    const_cast<Scope&>(std::get<0>(m_element)) = std::get<0>(node.m_element);
    std::get<1>(m_element) = std::get<1>(node.m_element);
    m_sub_scopes.clear();
    std::transform(node.m_sub_scopes.begin(), node.m_sub_scopes.end(),
      std::back_inserter(m_sub_scopes), [] (const auto& scope) {
        return std::make_unique<Node>(*scope);
      });
    return *this;
  }
}

  /** Associates a value with a given Scope. */
  template<typename T>
  class ScopeTable {
    public:

      /** The type of value to associate with a Scope. */
      using Element = T;

      /** Allows iterating over a ScopeTable. */
      class Iterator {
        public:
          using iterator_category = std::forward_iterator_tag;
          using value_type = std::tuple<const Scope, Element>;
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
          std::tuple<const Scope, Element>& operator *();

          /** Dereferences this Iterator. */
          std::tuple<const Scope, Element>* operator ->();

        private:
          friend class ScopeTable<Element>;
          std::deque<Details::Node<Element>*> m_nodes;

          Iterator(Details::Node<Element>& root);
      };

      /** Allows iterating over a ScopeTable. */
      class ConstIterator {
        public:
          using iterator_category = std::forward_iterator_tag;
          using value_type = const std::tuple<const Scope, Element>;
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
          const std::tuple<const Scope, Element>& operator *();

          /** Dereferences this ConstIterator. */
          const std::tuple<const Scope, Element>* operator ->();

        private:
          friend class ScopeTable<Element>;
          std::deque<const Details::Node<Element>*> m_nodes;

          ConstIterator(const Details::Node<Element>& root);
      };

      /**
       * Constructs a ScopeTable.
       * @param global The value associated with the 'global' Scope.
       */
      explicit ScopeTable(Element global);

      /**
       * Constructs a ScopeTable.
       * @param name The name of the 'global' Scope.
       * @param global The value associated with the 'global' Scope.
       */
      ScopeTable(std::string name, Element global);

      /** Returns the number of Scopes represented. */
      std::size_t get_size() const;

      /**
       * Returns the value associated with a Scope.
       * @param scope The Scope to retrieve the associated value of.
       * @return The value associated with the <i>scope</i>.
       */
      const Element& get(const Scope& scope) const;

      /**
       * Returns the value associated with a Scope.
       * @param scope The Scope to retrieve the associated value of.
       * @return The value associated with the <i>scope</i>.
       */
      Element& get(const Scope& scope);

      /**
       * Sets a value to be associated with a Scope.
       * @param scope The Scope to associate.
       * @param value The value to associate with the <i>scope</i>.
       */
      void set(const Scope& scope, const Element& value);

      /**
       * Erases a Scope.
       * @param scope The Scope to delete.
       */
      void erase(const Scope& scope);

      /**
       * Returns an iterator to the Scope that most closely encapsulates a
       * specified Scope.
       * @param scope The Scope to find.
       * @return A ConstIterator to the Scope that most closely encapsulates
       *         the specified <i>scope</i>.
       */
      Iterator find(const Scope& scope);

      /**
       * Returns an iterator to the Scope that most closely encapsulates a
       * specified Scope.
       * @param scope The Scope to find.
       * @return A ConstIterator to the Scope that most closely encapsulates
       *         the specified <i>scope</i>.
       */
      ConstIterator find(const Scope& scope) const;

      /** Returns an Iterator to the global Scope. */
      Iterator begin();

      /** Returns a ConstIterator to the global Scope. */
      ConstIterator begin() const;

      /** Returns an Iterator to the end of this container. */
      Iterator end();

      /** Returns a ConstIterator to the end of this container. */
      ConstIterator end() const;

    private:
      friend struct Beam::DataShuttle;
      friend struct Beam::Shuttle<ScopeTable>;
      Details::Node<Element> m_root;
      std::size_t m_size;

      ScopeTable();
      void insert(Details::Node<Element>& root, Scope scope, Element value);
      static Details::Node<Element>& find(
        Details::Node<Element>& root, const Scope& scope);
      static const Details::Node<Element>& find(
        const Details::Node<Element>& root, const Scope& scope);
      std::pair<Details::Node<Element>*, Details::Node<Element>*> find(
        Details::Node<Element>* parent, Details::Node<Element>* root,
        const Scope& scope);
  };

  template<typename T>
  typename ScopeTable<T>::Iterator& ScopeTable<T>::Iterator::operator ++() {
    auto node = m_nodes.front();
    m_nodes.pop_front();
    std::transform(node->m_sub_scopes.begin(), node->m_sub_scopes.end(),
      std::back_inserter(m_nodes), [] (const auto& scope) {
        return scope.get();
      });
    return *this;
  }

  template<typename T>
  typename ScopeTable<T>::Iterator ScopeTable<T>::Iterator::operator ++(int) {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  template<typename T>
  bool ScopeTable<T>::Iterator::operator ==(const Iterator& rhs) const {
    return (m_nodes.empty() && rhs.m_nodes.empty()) ||
      (!m_nodes.empty() && !rhs.m_nodes.empty() &&
        m_nodes.front() == rhs.m_nodes.front());
  }

  template<typename T>
  std::tuple<const Scope, typename ScopeTable<T>::Element>&
      ScopeTable<T>::Iterator::operator *() {
    return m_nodes.front()->m_element;
  }

  template<typename T>
  std::tuple<const Scope, typename ScopeTable<T>::Element>*
      ScopeTable<T>::Iterator::operator ->() {
    return &m_nodes.front()->m_element;
  }

  template<typename T>
  ScopeTable<T>::Iterator::Iterator(Details::Node<Element>& root) {
    m_nodes.push_back(&root);
  }

  template<typename T>
  typename ScopeTable<T>::ConstIterator&
      ScopeTable<T>::ConstIterator::operator ++() {
    auto node = m_nodes.front();
    m_nodes.pop_front();
    std::transform(node->m_sub_scopes.begin(), node->m_sub_scopes.end(),
      std::back_inserter(m_nodes), [] (const auto& scope) {
        return scope.get();
      });
    return *this;
  }

  template<typename T>
  typename ScopeTable<T>::ConstIterator
      ScopeTable<T>::ConstIterator::operator ++(int) {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  template<typename T>
  bool ScopeTable<T>::ConstIterator::operator ==(
      const ConstIterator& rhs) const {
    return (m_nodes.empty() && rhs.m_nodes.empty()) ||
      (!m_nodes.empty() && !rhs.m_nodes.empty() &&
        m_nodes.front() == rhs.m_nodes.front());
  }

  template<typename T>
  const std::tuple<const Scope, typename ScopeTable<T>::Element>&
      ScopeTable<T>::ConstIterator::operator *() {
    return m_nodes.front()->m_element;
  }

  template<typename T>
  const std::tuple<const Scope, typename ScopeTable<T>::Element>*
      ScopeTable<T>::ConstIterator::operator ->() {
    return &m_nodes.front()->m_element;
  }

  template<typename T>
  ScopeTable<T>::ConstIterator::ConstIterator(
      const Details::Node<Element>& root) {
    m_nodes.push_back(&root);
  }

  template<typename T>
  ScopeTable<T>::ScopeTable(Element global)
    : m_root(Scope::GLOBAL, std::move(global)),
      m_size(1) {}

  template<typename T>
  ScopeTable<T>::ScopeTable(std::string name, Element global)
    : m_root(Scope::make_global(std::move(name)), std::move(global)),
      m_size(1) {}

  template<typename T>
  std::size_t ScopeTable<T>::get_size() const {
    return m_size;
  }

  template<typename T>
  const typename ScopeTable<T>::Element&
      ScopeTable<T>::get(const Scope& scope) const {
    return std::get<1>(find(m_root, scope).m_element);
  }

  template<typename T>
  typename ScopeTable<T>::Element& ScopeTable<T>::get(const Scope& scope) {
    return std::get<1>(find(m_root, scope).m_element);
  }

  template<typename T>
  void ScopeTable<T>::set(const Scope& scope, const Element& value) {
    if(scope == std::get<0>(m_root.m_element)) {
      std::get<1>(m_root.m_element) = value;
      return;
    }
    insert(m_root, scope, value);
  }

  template<typename T>
  void ScopeTable<T>::erase(const Scope& scope) {
    auto node = find(nullptr, &m_root, scope);
    if(std::get<0>(node.second->m_element) != scope) {
      return;
    }
    if(!node.first) {
      return;
    }
    std::move(node.second->m_sub_scopes.begin(),
      node.second->m_sub_scopes.end(),
      std::back_inserter(node.first->m_sub_scopes));
    for(auto i = node.first->m_sub_scopes.begin();
        i != node.first->m_sub_scopes.end(); ++i) {
      if(i->get() == node.second) {
        node.first->m_sub_scopes.erase(i);
        --m_size;
        break;
      }
    }
  }

  template<typename T>
  typename ScopeTable<T>::Iterator ScopeTable<T>::find(const Scope& scope) {
    auto& node = find(m_root, scope);
    return Iterator(node);
  }

  template<typename T>
  typename ScopeTable<T>::ConstIterator ScopeTable<T>::find(
      const Scope& scope) const {
    auto& node = find(m_root, scope);
    return ConstIterator(node);
  }

  template<typename T>
  typename ScopeTable<T>::Iterator ScopeTable<T>::begin() {
    return Iterator(m_root);
  }

  template<typename T>
  typename ScopeTable<T>::ConstIterator ScopeTable<T>::begin() const {
    return ConstIterator(m_root);
  }

  template<typename T>
  typename ScopeTable<T>::Iterator ScopeTable<T>::end() {
    return Iterator();
  }

  template<typename T>
  typename ScopeTable<T>::ConstIterator ScopeTable<T>::end() const {
    return ConstIterator();
  }

  template<typename T>
  ScopeTable<T>::ScopeTable()
    : m_root(Scope::GLOBAL, {}),
      m_size(1) {}

  template<typename T>
  void ScopeTable<T>::insert(
      Details::Node<Element>& root, Scope scope, Element value) {
    auto sub_scopes = std::vector<std::unique_ptr<Details::Node<Element>>>();
    auto i = root.m_sub_scopes.begin();
    while(i != root.m_sub_scopes.end()) {
      if(scope < std::get<0>((*i)->m_element)) {
        insert(**i, std::move(scope), std::move(value));
        return;
      } else if(scope == std::get<0>((*i)->m_element)) {
        std::get<1>((*i)->m_element) = std::move(value);
        return;
      } else if(scope > std::get<0>((*i)->m_element)) {
        sub_scopes.push_back(std::move(*i));
        i = root.m_sub_scopes.erase(i);
      } else {
        ++i;
      }
    }
    auto sub_scope = std::make_unique<Details::Node<Element>>(
      std::move(scope), std::move(value));
    sub_scope->m_sub_scopes.swap(sub_scopes);
    root.m_sub_scopes.push_back(std::move(sub_scope));
    ++m_size;
  }

  template<typename T>
  Details::Node<typename ScopeTable<T>::Element>& ScopeTable<T>::find(
      Details::Node<Element>& root, const Scope& scope) {
    for(auto& sub_scope : root.m_sub_scopes) {
      if(scope <= std::get<0>(sub_scope->m_element)) {
        return find(*sub_scope, scope);
      }
    }
    return root;
  }

  template<typename T>
  const Details::Node<typename ScopeTable<T>::Element>& ScopeTable<T>::find(
      const Details::Node<Element>& root, const Scope& scope) {
    return find(const_cast<Details::Node<Element>&>(root), scope);
  }

  template<typename T>
  std::pair<Details::Node<typename ScopeTable<T>::Element>*,
      Details::Node<typename ScopeTable<T>::Element>*> ScopeTable<T>::find(
      Details::Node<Element>* parent, Details::Node<Element>* root,
      const Scope& scope) {
    for(auto& sub_scope : root->m_sub_scopes) {
      if(scope <= std::get<0>(sub_scope->m_element)) {
        return find(root, sub_scope.get(), scope);
      }
    }
    return {parent, root};
  }
}

namespace Beam {
  template<typename T>
  constexpr auto is_structure<Nexus::ScopeTable<T>> = false;

  template<typename T>
  struct Send<Nexus::ScopeTable<T>> {
    template<IsSender S>
    void operator ()(S& sender, const char* name,
        const Nexus::ScopeTable<T>& value) const {
      sender.start_sequence(name, static_cast<int>(value.get_size()));
      for(auto i = value.begin(); i != value.end(); ++i) {
        sender.send(*i);
      }
      sender.end_sequence();
    }
  };

  template<typename T>
  struct Receive<Nexus::ScopeTable<T>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, Nexus::ScopeTable<T>& value) const {
      auto size = 0;
      receiver.start_sequence(name, size);
      for(auto i = 0; i < size; ++i) {
        auto entry = std::tuple<Nexus::Scope, T>();
        receiver.shuttle(entry);
        value.set(
          std::move(std::get<0>(entry)), std::move(std::get<1>(entry)));
      }
      receiver.end_sequence();
    }
  };
}

#endif
