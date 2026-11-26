#ifndef SPIRE_FILTERED_LIST_MODEL_HPP
#define SPIRE_FILTERED_LIST_MODEL_HPP
#include <boost/scope/scope_exit.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TransactionLogModel.hpp"

namespace Spire {

  /** Implements a ListModel that filters values out of a source list. */
  template<typename T>
  class FilteredListModel : public TransactionLogModel<ListModel<T>> {
    public:
      using Type = typename ListModel<T>::Type;
      using Operation = typename ListModel<T>::Operation;
      using OperationSignal = typename ListModel<T>::OperationSignal;
      using AddOperation = typename ListModel<T>::AddOperation;
      using MoveOperation = typename ListModel<T>::MoveOperation;
      using PreRemoveOperation = typename ListModel<T>::PreRemoveOperation;
      using RemoveOperation = typename ListModel<T>::RemoveOperation;
      using UpdateOperation = typename ListModel<T>::UpdateOperation;
      using StartTransaction = typename ListModel<T>::StartTransaction;
      using EndTransaction = typename ListModel<T>::EndTransaction;

      /**
       * The type of callable used to filter elements.
       * @param list The list being filtered.
       * @param index The index of the element within the <i>list</i> to test.
       * @return <code>true</code> iff the <i>index</i> should be excluded.
       */
      using Filter =
        std::function<bool (const ListModel<Type>& list, int index)>;

      /**
       * Constructs a FilteredListModel from a ListModel and a filter function.
       * @param source The list to filter.
       * @param filter The filter applied to every element of the <i>source</i>.
       */
      FilteredListModel(std::shared_ptr<ListModel<Type>> source, Filter filter);

      /** Applies a new filter to this list. */
      void set_filter(const Filter& filter);

      /** Reapplies the filter to source indices in [first, end). */
      void refresh(int first, int end);

      /**
       * Maps an index from this list into the source list.
       * @param index An index into this table.
       * @return The corresponding index into the source list or <code>-1</code>
       *         iff the index is not valid.
       */
      int index_to_source(int index) const;

      /**
       * Maps an index from the source list to this list.
       * @param index An index into the source list.
       * @return The corresponding index into this list, or <code>-1</code> iff
       *         the index is not valid.
       */
      int index_from_source(int index) const;

      int get_size() const override;
      const Type& get(int index) const override;
      QValidator::State set(int index, const Type& value) override;
      QValidator::State push(const Type& value) override;
      QValidator::State insert(const Type& value, int index) override;
      QValidator::State move(int source, int destination) override;
      QValidator::State remove(int index) override;
      using ListModel<T>::insert;
      using ListModel<T>::remove;

    private:
      std::shared_ptr<ListModel<Type>> m_source;
      Filter m_filter;
      int m_filter_count;
      bool m_is_refreshing;
      bool m_has_pending_removal;
      std::vector<int> m_filtered_data;
      std::vector<int> m_suffix;
      boost::signals2::scoped_connection m_source_connection;

      std::tuple<bool, std::vector<int>::iterator> find(int index);
      void split_at(int source);
      void on_operation(const Operation& operation);
  };

  template<typename T>
  FilteredListModel(
    std::shared_ptr<T>, typename FilteredListModel<typename T::Type>::Filter) ->
      FilteredListModel<typename T::Type>;

  template<typename T>
  FilteredListModel<T>::FilteredListModel(
      std::shared_ptr<ListModel<Type>> source, Filter filter)
      : m_source(std::move(source)),
        m_filter(std::move(filter)),
        m_filter_count(0),
        m_is_refreshing(false),
        m_has_pending_removal(false) {
    for(auto i = 0; i != m_source->get_size(); ++i) {
      if(!m_filter(*m_source, i)) {
        m_filtered_data.push_back(i);
      }
    }
    this->m_transaction.set_size(get_size());
    m_source_connection = m_source->connect_operation_signal(
      std::bind_front(&FilteredListModel::on_operation, this));
  }

  template<typename T>
  void FilteredListModel<T>::set_filter(const Filter& filter) {
    m_filter = filter;
    refresh(0, m_source->get_size());
  }

  template<typename T>
  void FilteredListModel<T>::refresh(int first, int end) {
    ++m_filter_count;
    auto count = m_filter_count;
    auto was_refreshing = std::exchange(m_is_refreshing, true);
    auto restore = boost::scope::scope_exit([&] {
      m_is_refreshing = was_refreshing;
    });
    if(was_refreshing) {
      first = 0;
      end = m_source->get_size();
    }
    this->m_transaction.transact([&] {
      split_at(first);
      for(auto row = first; row < end && count == m_filter_count; ++row) {
        auto visible = !m_suffix.empty() && m_suffix.back() == row;
        auto index = static_cast<int>(m_filtered_data.size());
        if(!m_filter(*m_source, row)) {
          m_filtered_data.push_back(row);
          if(visible) {
            m_suffix.pop_back();
          } else {
            this->m_transaction.push(AddOperation(index));
          }
        } else if(visible) {
          this->m_transaction.push(PreRemoveOperation(index));
          if(count != m_filter_count) {
            break;
          }
          m_suffix.pop_back();
          this->m_transaction.push(RemoveOperation(index));
        }
      }
      if(count != m_filter_count) {
        refresh(0, m_source->get_size());
      }
    });
  }

  template<typename T>
  int FilteredListModel<T>::index_to_source(int index) const {
    if(index < 0 || index >= get_size()) {
      return -1;
    }
    if(index < std::ssize(m_filtered_data)) {
      return m_filtered_data[index];
    }
    return m_suffix[get_size() - 1 - index];
  }

  template<typename T>
  int FilteredListModel<T>::index_from_source(int index) const {
    if(!m_suffix.empty() && index >= m_suffix.back()) {
      auto i = std::lower_bound(m_suffix.rbegin(), m_suffix.rend(), index);
      if(i == m_suffix.rend() || *i != index) {
        return -1;
      }
      return static_cast<int>(m_filtered_data.size() + (i - m_suffix.rbegin()));
    }
    auto i = std::lower_bound(
      m_filtered_data.begin(), m_filtered_data.end(), index);
    if(i != m_filtered_data.end() && *i == index) {
      return static_cast<int>(std::distance(m_filtered_data.begin(), i));
    }
    return -1;
  }

  template<typename T>
  int FilteredListModel<T>::get_size() const {
    return static_cast<int>(m_filtered_data.size() + m_suffix.size());
  }

  template<typename T>
  const typename FilteredListModel<T>::Type&
      FilteredListModel<T>::get(int index) const {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_source->get(index_to_source(index));
  }

  template<typename T>
  QValidator::State FilteredListModel<T>::set(int index, const Type& value) {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_source->set(index_to_source(index), value);
  }

  template<typename T>
  QValidator::State FilteredListModel<T>::push(const Type& value) {
    return m_source->push(value);
  }

  template<typename T>
  QValidator::State FilteredListModel<T>::insert(const Type& value, int index) {
    return QValidator::Invalid;
  }

  template<typename T>
  QValidator::State FilteredListModel<T>::move(int source, int destination) {
    return QValidator::Invalid;
  }

  template<typename T>
  QValidator::State FilteredListModel<T>::remove(int index) {
    return m_source->remove(index_to_source(index));
  }

  template<typename T>
  std::tuple<bool, std::vector<int>::iterator>
      FilteredListModel<T>::find(int index) {
    auto i = std::lower_bound(
      m_filtered_data.begin(), m_filtered_data.end(), index);
    if(i != m_filtered_data.end() && *i == index) {
      return {true, i};
    }
    return {false, i};
  }

  template<typename T>
  void FilteredListModel<T>::split_at(int source) {
    while(!m_filtered_data.empty() && m_filtered_data.back() >= source) {
      m_suffix.push_back(m_filtered_data.back());
      m_filtered_data.pop_back();
    }
    while(!m_suffix.empty() && m_suffix.back() < source) {
      m_filtered_data.push_back(m_suffix.back());
      m_suffix.pop_back();
    }
  }

  template<typename T>
  void FilteredListModel<T>::on_operation(const Operation& operation) {
    if(std::holds_alternative<StartTransaction>(operation)) {
      this->m_transaction.start();
      return;
    } else if(std::holds_alternative<EndTransaction>(operation)) {
      this->m_transaction.end();
      return;
    }
    if(m_is_refreshing) {
      ++m_filter_count;
    }
    if(!m_suffix.empty()) {
      m_filtered_data.insert(
        m_filtered_data.end(), m_suffix.rbegin(), m_suffix.rend());
      m_suffix.clear();
    }
    visit(operation,
      [&] (const AddOperation& operation) {
        if(operation.m_index >= m_source->get_size() - 1) {
          if(!m_filter(*m_source, operation.m_index)) {
            m_filtered_data.push_back(operation.m_index);
            this->m_transaction.push(
              AddOperation(static_cast<int>(m_filtered_data.size()) - 1));
          }
        } else {
          auto i = std::get<1>(find(operation.m_index));
          std::for_each(i, m_filtered_data.end(), [] (int& value) { ++value; });
          if(!m_filter(*m_source, operation.m_index)) {
            this->m_transaction.push(AddOperation(
              static_cast<int>(m_filtered_data.insert(i, operation.m_index) -
                m_filtered_data.begin())));
          }
        }
      },
      [&] (const MoveOperation& operation) {
        auto [is_found, source] = find(operation.m_source);
        if(is_found) {
          auto destination = source;
          if(operation.m_source < operation.m_destination) {
            destination = std::upper_bound(
              source, m_filtered_data.end(), operation.m_destination) - 1;
            for(auto i = source; i != destination; ++i) {
              *i = *(i + 1) - 1;
            }
          } else {
            destination = std::lower_bound(
              m_filtered_data.begin(), source, operation.m_destination);
            for(auto i = source; i != destination; --i) {
              *i = *(i - 1) + 1;
            }
          }
          *destination = operation.m_destination;
          this->m_transaction.push(MoveOperation(
            static_cast<int>(source - m_filtered_data.begin()),
            static_cast<int>(destination - m_filtered_data.begin())));
        } else if(operation.m_source < operation.m_destination) {
          auto destination = std::upper_bound(
            source, m_filtered_data.end(), operation.m_destination);
          std::for_each(source, destination, [] (int& value) { --value; });
        } else {
          auto destination = std::lower_bound(
            m_filtered_data.begin(), source, operation.m_destination);
          std::for_each(destination, source, [] (int& value) { ++value; });
        }
      },
      [&] (const PreRemoveOperation& operation) {
        auto [is_found, i] = find(operation.m_index);
        m_has_pending_removal = true;
        if(is_found) {
          ++m_filter_count;
          this->m_transaction.push(PreRemoveOperation(
            static_cast<int>(i - m_filtered_data.begin())));
        }
      },
      [&] (const RemoveOperation& operation) {
        if(!std::exchange(m_has_pending_removal, false)) {
          return;
        }
        auto [is_found, i] = find(operation.m_index);
        std::for_each(i, m_filtered_data.end(), [] (int& value) { --value; });
        if(is_found) {
          auto index = static_cast<int>(i - m_filtered_data.begin());
          m_filtered_data.erase(i);
          this->m_transaction.push(RemoveOperation(index));
        }
      },
      [&] (const UpdateOperation& operation) {
        auto [is_found, i] = find(operation.m_index);
        if(!m_filter(*m_source, operation.m_index)) {
          if(is_found) {
            this->m_transaction.push(UpdateOperation(
              static_cast<int>(i - m_filtered_data.begin()),
              operation.get_previous(), operation.get_value()));
          } else {
            auto filtered_index = static_cast<int>(m_filtered_data.insert(
              i, operation.m_index) - m_filtered_data.begin());
            this->m_transaction.push(AddOperation(filtered_index));
          }
        } else if(is_found) {
          auto index = static_cast<int>(i - m_filtered_data.begin());
          ++m_filter_count;
          auto count = m_filter_count;
          this->m_transaction.push(PreRemoveOperation(index));
          if(count != m_filter_count) {
            return;
          }
          m_filtered_data.erase(i);
          this->m_transaction.push(RemoveOperation(index));
        }
      });
  }
}

#endif
