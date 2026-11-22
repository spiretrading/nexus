#ifndef SPIRE_UNION_LIST_MODEL_HPP
#define SPIRE_UNION_LIST_MODEL_HPP
#include <algorithm>
#include <functional>
#include <list>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "Spire/Spire/TransactionLogModel.hpp"

namespace Spire {

  /** A copyable function that hashes a value without modifying it. */
  template<typename H, typename T>
  concept HashFunction = std::copy_constructible<H> &&
    std::regular_invocable<const H&, const T&> &&
    std::convertible_to<std::invoke_result_t<const H&, const T&>, std::size_t>;

  /** Combines source lists into a read-only list of distinct values. */
  template<typename T, HashFunction<T> H = std::hash<T>,
    typename E = std::equal_to<T>> requires std::copy_constructible<E> &&
      std::equivalence_relation<const E&, const T&, const T&>
  class UnionListModel : public TransactionLogModel<ListModel<T>> {
    public:
      using Type = typename ListModel<T>::Type;
      using Operation = typename ListModel<T>::Operation;
      using AddOperation = typename ListModel<T>::AddOperation;
      using MoveOperation = typename ListModel<T>::MoveOperation;
      using RemoveOperation = typename ListModel<T>::RemoveOperation;
      using PreRemoveOperation = typename ListModel<T>::PreRemoveOperation;
      using UpdateOperation = typename ListModel<T>::UpdateOperation;
      using StartTransaction = typename ListModel<T>::StartTransaction;
      using EndTransaction = typename ListModel<T>::EndTransaction;

      /** Computes a value's hash. */
      using Hash = H;

      /** Tests whether two values represent the same entry. */
      using IsEqual = E;

      /** Constructs an empty union using default-constructed callables. */
      UnionListModel();

      /** Constructs an empty union with custom hashing and equality. */
      UnionListModel(Hash hash, IsEqual is_equal);

      /** Adds a source list to the union. */
      void add_source(std::shared_ptr<ListModel<Type>> source);

      /** Removes a source list from the union. */
      void remove_source(const std::shared_ptr<ListModel<Type>>& source);

      int get_size() const override;
      const Type& get(int index) const override;
      using ListModel<T>::remove;

    private:
      struct Entry {
        int m_index;
        std::list<Type> m_values;
      };
      struct Contribution {
        Entry* m_entry;
        typename std::list<Type>::iterator m_value;
      };
      struct Source {
        std::shared_ptr<ListModel<Type>> m_model;
        std::vector<Contribution> m_contributions;
        boost::signals2::scoped_connection m_connection;
      };
      std::unordered_map<Type, std::unique_ptr<Entry>, Hash, IsEqual> m_index;
      std::vector<Entry*> m_entries;
      std::vector<std::unique_ptr<Source>> m_sources;

      void add(Source& source, int index, const Type& value);
      void remove(Source& source, int index);
      void on_operation(Source& source, const Operation& operation);
  };

  template<typename T, HashFunction<T> H, typename E> requires
    std::copy_constructible<E> &&
    std::equivalence_relation<const E&, const T&, const T&>
  UnionListModel<T, H, E>::UnionListModel()
    : UnionListModel(Hash(), IsEqual()) {}

  template<typename T, HashFunction<T> H, typename E> requires
    std::copy_constructible<E> &&
    std::equivalence_relation<const E&, const T&, const T&>
  UnionListModel<T, H, E>::UnionListModel(Hash hash, IsEqual is_equal)
    : m_index(0, std::move(hash), std::move(is_equal)) {}

  template<typename T, HashFunction<T> H, typename E> requires
    std::copy_constructible<E> &&
    std::equivalence_relation<const E&, const T&, const T&>
  void UnionListModel<T, H, E>::add_source(
      std::shared_ptr<ListModel<Type>> source) {
    auto i = std::ranges::find(m_sources, source, &Source::m_model);
    if(i != m_sources.end()) {
      return;
    }
    auto entry = std::make_unique<Source>();
    entry->m_model = std::move(source);
    auto pointer = entry.get();
    m_sources.push_back(std::move(entry));
    pointer->m_connection = pointer->m_model->connect_operation_signal(
      [=, this] (const auto& operation) {
        on_operation(*pointer, operation);
      });
    this->transact([&] {
      for(auto i = 0; i != pointer->m_model->get_size(); ++i) {
        add(*pointer, i, pointer->m_model->get(i));
      }
    });
  }

  template<typename T, HashFunction<T> H, typename E> requires
    std::copy_constructible<E> &&
    std::equivalence_relation<const E&, const T&, const T&>
  void UnionListModel<T, H, E>::remove_source(
      const std::shared_ptr<ListModel<Type>>& source) {
    auto i = std::ranges::find(m_sources, source, &Source::m_model);
    if(i == m_sources.end()) {
      return;
    }
    auto entry = std::move(*i);
    m_sources.erase(i);
    entry->m_connection.disconnect();
    this->transact([&] {
      while(!entry->m_contributions.empty()) {
        remove(*entry, static_cast<int>(entry->m_contributions.size()) - 1);
      }
    });
  }

  template<typename T, HashFunction<T> H, typename E> requires
    std::copy_constructible<E> &&
    std::equivalence_relation<const E&, const T&, const T&>
  int UnionListModel<T, H, E>::get_size() const {
    return static_cast<int>(m_entries.size());
  }

  template<typename T, HashFunction<T> H, typename E> requires
    std::copy_constructible<E> &&
    std::equivalence_relation<const E&, const T&, const T&>
  const typename UnionListModel<T, H, E>::Type&
      UnionListModel<T, H, E>::get(int index) const {
    return m_entries.at(index)->m_values.front();
  }

  template<typename T, HashFunction<T> H, typename E> requires
    std::copy_constructible<E> &&
    std::equivalence_relation<const E&, const T&, const T&>
  void UnionListModel<T, H, E>::add(
      Source& source, int index, const Type& value) {
    auto [i, is_inserted] = m_index.try_emplace(value);
    if(is_inserted) {
      i->second = std::make_unique<Entry>();
      i->second->m_index = get_size();
      m_entries.push_back(i->second.get());
    }
    auto& entry = *i->second;
    entry.m_values.push_back(value);
    source.m_contributions.insert(source.m_contributions.begin() + index,
      Contribution(&entry, std::prev(entry.m_values.end())));
    if(is_inserted) {
      this->m_transaction.push(AddOperation(entry.m_index));
    }
  }

  template<typename T, HashFunction<T> H, typename E> requires
    std::copy_constructible<E> &&
    std::equivalence_relation<const E&, const T&, const T&>
  void UnionListModel<T, H, E>::remove(Source& source, int index) {
    auto contribution = source.m_contributions[index];
    source.m_contributions.erase(source.m_contributions.begin() + index);
    auto& entry = *contribution.m_entry;
    auto is_representative = contribution.m_value == entry.m_values.begin();
    if(entry.m_values.size() == 1) {
      auto index = entry.m_index;
      this->m_transaction.push(PreRemoveOperation(index));
      auto i = m_index.find(*contribution.m_value);
      m_entries.erase(m_entries.begin() + index);
      m_index.erase(i);
      for(auto i = index; i != get_size(); ++i) {
        m_entries[i]->m_index = i;
      }
      this->m_transaction.push(RemoveOperation(index));
    } else if(is_representative) {
      auto previous = *contribution.m_value;
      entry.m_values.erase(contribution.m_value);
      this->m_transaction.push(
        UpdateOperation(entry.m_index, previous, entry.m_values.front()));
    } else {
      entry.m_values.erase(contribution.m_value);
    }
  }

  template<typename T, HashFunction<T> H, typename E> requires
    std::copy_constructible<E> &&
    std::equivalence_relation<const E&, const T&, const T&>
  void UnionListModel<T, H, E>::on_operation(
      Source& source, const Operation& operation) {
    visit(operation,
      [&] (StartTransaction) {
        this->start_transaction();
      },
      [&] (EndTransaction) {
        this->end_transaction();
      },
      [&] (const AddOperation& add) {
        this->add(source, add.m_index, source.m_model->get(add.m_index));
      },
      [&] (const RemoveOperation& remove) {
        this->remove(source, remove.m_index);
      },
      [&] (const MoveOperation& move) {
        auto& contributions = source.m_contributions;
        if(move.m_source < move.m_destination) {
          std::ranges::rotate(contributions.begin() + move.m_source,
            contributions.begin() + move.m_source + 1,
            contributions.begin() + move.m_destination + 1);
        } else if(move.m_source > move.m_destination) {
          std::ranges::rotate(contributions.begin() + move.m_destination,
            contributions.begin() + move.m_source,
            contributions.begin() + move.m_source + 1);
        }
      },
      [&] (const UpdateOperation& update) {
        auto& contribution = source.m_contributions[update.m_index];
        auto& value = update.get_value();
        if(m_index.key_eq()(*contribution.m_value, value)) {
          auto previous = *contribution.m_value;
          *contribution.m_value = value;
          auto& entry = *contribution.m_entry;
          if(contribution.m_value == entry.m_values.begin()) {
            this->m_transaction.push(
              UpdateOperation(entry.m_index, previous, value));
          }
        } else {
          this->transact([&] {
            remove(source, update.m_index);
            add(source, update.m_index, value);
          });
        }
      });
  }
}

#endif
