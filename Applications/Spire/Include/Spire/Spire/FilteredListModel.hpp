#ifndef SPIRE_FILTERED_LIST_MODEL_HPP
#define SPIRE_FILTERED_LIST_MODEL_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"

namespace Spire {

  /** Implements a ListModel that filters values out of a source list. */
  template<typename T>
  class FilteredListModel : public ListModel<T> {
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
      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;
      using ListModel<T>::insert;
      using ListModel<T>::remove;
      using ListModel<T>::transact;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      struct RemoveEntry {
        bool m_is_found;
        std::vector<int>::iterator m_iterator;
        int m_index;
      };
      std::shared_ptr<ListModel<Type>> m_source;
      Filter m_filter;
      int m_filter_count;
      boost::optional<RemoveEntry> m_remove_entry;
      std::vector<int> m_filtered_data;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      std::tuple<bool, std::vector<int>::iterator> find(int index);
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
        m_filter_count(0) {
    for(auto i = 0; i != m_source->get_size(); ++i) {
      if(!m_filter(*m_source, i)) {
        m_filtered_data.push_back(i);
      }
    }
    m_source_connection = m_source->connect_operation_signal(
      std::bind_front(&FilteredListModel::on_operation, this));
  }

  template<typename T>
  void FilteredListModel<T>::set_filter(const Filter& filter) {
    ++m_filter_count;
    auto count = m_filter_count;
    m_filter = filter;
    auto source_index = 0;
    auto filtered_index = 0;
    m_transaction.transact([&] {
      while(source_index != m_source->get_size() &&
          filtered_index != static_cast<int>(m_filtered_data.size())) {
        if(!m_filter(*m_source, source_index)) {
          if(m_filtered_data[filtered_index] != source_index) {
            m_filtered_data.insert(
              m_filtered_data.begin() + filtered_index, source_index);
            m_transaction.push(AddOperation(filtered_index));
            if(count != m_filter_count) {
              return;
            }
          }
          ++filtered_index;
        } else {
          if(m_filtered_data[filtered_index] == source_index) {
            m_transaction.push(PreRemoveOperation(filtered_index));
            if(count != m_filter_count) {
              return;
            }
            m_filtered_data.erase(m_filtered_data.begin() + filtered_index);
            m_transaction.push(RemoveOperation(filtered_index));
            if(count != m_filter_count) {
              return;
            }
          }
        }
        ++source_index;
      }
      while(filtered_index != static_cast<int>(m_filtered_data.size())) {
        m_transaction.push(PreRemoveOperation(filtered_index));
        if(count != m_filter_count) {
          return;
        }
        m_filtered_data.erase(m_filtered_data.begin() + filtered_index);
        m_transaction.push(RemoveOperation(filtered_index));
        if(count != m_filter_count) {
          return;
        }
      }
      while(source_index != m_source->get_size()) {
        if(!m_filter(*m_source, source_index)) {
          m_filtered_data.push_back(source_index);
          m_transaction.push(AddOperation(m_filtered_data.size() - 1));
          if(count != m_filter_count) {
            return;
          }
        }
        ++source_index;
      }
    });
  }

  template<typename T>
  int FilteredListModel<T>::index_to_source(int index) const {
    if(index < 0 || index >= get_size()) {
      return -1;
    }
    return m_filtered_data[index];
  }

  template<typename T>
  int FilteredListModel<T>::index_from_source(int index) const {
    auto i = std::lower_bound(
      m_filtered_data.begin(), m_filtered_data.end(), index);
    if(i != m_filtered_data.end() && *i == index) {
      return static_cast<int>(std::distance(m_filtered_data.begin(), i));
    }
    return -1;
  }

  template<typename T>
  int FilteredListModel<T>::get_size() const {
    return m_filtered_data.size();
  }

  template<typename T>
  const typename FilteredListModel<T>::Type&
      FilteredListModel<T>::get(int index) const {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_source->get(m_filtered_data[index]);
  }

  template<typename T>
  QValidator::State FilteredListModel<T>::set(int index, const Type& value) {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_source->set(m_filtered_data[index], value);
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
    return m_source->remove(m_filtered_data[index]);
  }

  template<typename T>
  boost::signals2::connection FilteredListModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void FilteredListModel<T>::transact(
      const std::function<void ()>& transaction) {
    m_transaction.transact(transaction);
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
  void FilteredListModel<T>::on_operation(const Operation& operation) {
    visit(operation,
      [&] (const StartTransaction&) {
        m_transaction.start();
      },
      [&] (const EndTransaction&) {
        m_transaction.end();
      },
      [&] (const AddOperation& operation) {
        if(operation.m_index >= m_source->get_size() - 1) {
          if(!m_filter(*m_source, operation.m_index)) {
            m_filtered_data.push_back(operation.m_index);
            m_transaction.push(
              AddOperation(static_cast<int>(m_filtered_data.size()) - 1));
          }
        } else {
          auto i = std::get<1>(find(operation.m_index));
          std::for_each(i, m_filtered_data.end(), [] (int& value) { ++value; });
          if(!m_filter(*m_source, operation.m_index)) {
            m_transaction.push(AddOperation(
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
          m_transaction.push(MoveOperation(
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
        m_remove_entry.emplace(is_found, i, 0);
        if(m_remove_entry->m_is_found) {
          m_remove_entry->m_index = static_cast<int>(
            m_remove_entry->m_iterator - m_filtered_data.begin());
          ++m_filter_count;
          auto count = m_filter_count;
          m_transaction.push(PreRemoveOperation(m_remove_entry->m_index));
          if(count != m_filter_count) {
            m_remove_entry = boost::none;
          }
        }
      },
      [&] (const RemoveOperation& operation) {
        if(!m_remove_entry) {
          return;
        }
        std::for_each(m_remove_entry->m_iterator, m_filtered_data.end(),
          [] (int& value) { --value; });
        if(m_remove_entry->m_is_found) {
          m_filtered_data.erase(m_remove_entry->m_iterator);
          m_transaction.push(RemoveOperation(m_remove_entry->m_index));
        }
        m_remove_entry = boost::none;
      },
      [&] (const UpdateOperation& operation) {
        auto [is_found, i] = find(operation.m_index);
        if(!m_filter(*m_source, operation.m_index)) {
          if(is_found) {
            m_transaction.push(UpdateOperation(
              static_cast<int>(i - m_filtered_data.begin()),
              operation.get_previous(), operation.get_value()));
          } else {
            auto filtered_index = static_cast<int>(m_filtered_data.insert(
              i, operation.m_index) - m_filtered_data.begin());
            m_transaction.push(AddOperation(filtered_index));
          }
        } else if(is_found) {
          auto index = static_cast<int>(i - m_filtered_data.begin());
          ++m_filter_count;
          auto count = m_filter_count;
          m_transaction.push(PreRemoveOperation(index));
          if(count != m_filter_count) {
            return;
          }
          m_filtered_data.erase(i);
          m_transaction.push(RemoveOperation(index));
        }
      });
  }
}

#endif
