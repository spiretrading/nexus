#ifndef SPIRE_LIST_INDEX_VALUE_MODEL_HPP
#define SPIRE_LIST_INDEX_VALUE_MODEL_HPP
#include <type_traits>
#include <boost/signals2/shared_connection_block.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {
namespace Details {
  template<typename T>
  struct is_index_optional : std::false_type {};

  template<typename T>
  struct is_index_optional<boost::optional<T>> : std::true_type {};
}

  template<typename> class LocalValueModel;

  /**
   * Models an index into a ListModel containing the value represented by a
   * ValueModel.
   * @param <T> The type of value to find within the ListModel.
   */
  template<typename T>
  class ListIndexValueModel : public ValueModel<boost::optional<int>> {
    public:

      /** The type of value to find within the ListModel. */
      using SearchType = T;

      /**
       * Constructs a ListIndexValueModel.
       * @param list The ListModel to index into.
       * @param value The value to find within the <i>list</i>.
       */
      ListIndexValueModel(std::shared_ptr<ListModel<SearchType>> list,
        std::shared_ptr<ValueModel<SearchType>> value);

      QValidator::State get_state() const override;
      const boost::optional<int>& get() const override;
      QValidator::State test(const boost::optional<int>& value) const override;
      QValidator::State set(const boost::optional<int>& value) override;
      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      static constexpr auto is_optional =
        Details::is_index_optional<SearchType>::value;
      LocalValueModel<boost::optional<int>> m_index;
      std::shared_ptr<ListModel<SearchType>> m_list;
      boost::signals2::scoped_connection m_list_connection;
      std::shared_ptr<ValueModel<SearchType>> m_value;
      boost::signals2::scoped_connection m_current_connection;

      void search(const SearchType& value);
      void on_operation(
        const typename ListModel<SearchType>::Operation& operation);
      void on_current(const SearchType& current);
  };

  template<typename L, typename T>
  ListIndexValueModel(std::shared_ptr<L>, std::shared_ptr<T>) ->
    ListIndexValueModel<typename T::Type>;

  template<typename T>
  ListIndexValueModel<T>::ListIndexValueModel(
      std::shared_ptr<ListModel<SearchType>> list,
      std::shared_ptr<ValueModel<SearchType>> value)
      : m_list(std::move(list)),
        m_list_connection(m_list->connect_operation_signal(
          std::bind_front(&ListIndexValueModel::on_operation, this))),
        m_value(std::move(value)),
        m_current_connection(m_value->connect_update_signal(
          std::bind_front(&ListIndexValueModel::on_current, this))) {
    on_current(m_value->get());
  }

  template<typename T>
  QValidator::State ListIndexValueModel<T>::get_state() const {
    return m_index.get_state();
  }

  template<typename T>
  const boost::optional<int>& ListIndexValueModel<T>::get() const {
    return m_index.get();
  }

  template<typename T>
  QValidator::State ListIndexValueModel<T>::test(
      const boost::optional<int>& value) const {
    if(!value) {
      if constexpr(is_optional) {
        if(m_value->test(boost::none) == QValidator::Invalid) {
          return QValidator::Invalid;
        }
        return m_index.test(boost::none);
      } else {
        return QValidator::Invalid;
      }
    } else if(value && *value < 0 || value >= m_list->get_size()) {
      return QValidator::Invalid;
    }
    try {
      if(m_value->test(m_list->get(*value)) == QValidator::Invalid) {
        return QValidator::Invalid;
      }
    } catch(const std::bad_any_cast&) {
      return QValidator::Invalid;
    }
    return m_index.test(*value);
  }

  template<typename T>
  QValidator::State ListIndexValueModel<T>::set(
      const boost::optional<int>& value) {
    if(!value) {
      if constexpr(is_optional) {
        auto blocker =
          boost::signals2::shared_connection_block(m_current_connection);
        if(m_value->set(boost::none) == QValidator::Invalid) {
          return QValidator::Invalid;
        }
        return m_index.set(boost::none);
      } else {
        return QValidator::Invalid;
      }
    } else if(value && *value < 0 || value >= m_list->get_size()) {
      return QValidator::Invalid;
    }
    try {
      auto blocker =
        boost::signals2::shared_connection_block(m_current_connection);
      if(m_value->set(m_list->get(*value)) == QValidator::Invalid) {
        return QValidator::Invalid;
      }
    } catch(const std::bad_any_cast&) {
      return QValidator::Invalid;
    }
    return m_index.set(*value);
  }

  template<typename T>
  boost::signals2::connection ListIndexValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_index.connect_update_signal(slot);
  }

  template<typename T>
  void ListIndexValueModel<T>::search(const SearchType& value) {
    for(auto i = 0; i != m_list->get_size(); ++i) {
      try {
        if(m_list->get(i) == value) {
          m_index.set(i);
          return;
        }
      } catch(const std::bad_any_cast&) {}
    }
    m_index.set(boost::none);
  }

  template<typename T>
  void ListIndexValueModel<T>::on_operation(
      const typename ListModel<SearchType>::Operation& operation) {
    visit(operation,
      [&] (const typename ListModel<SearchType>::AddOperation& operation) {
        if(m_index.get() && operation.m_index <= *m_index.get()) {
          m_index.set(*m_index.get() + 1);
        }
      },
      [&] (const typename ListModel<SearchType>::RemoveOperation& operation) {
        if(!m_index.get()) {
          return;
        }
        if(operation.m_index < *m_index.get()) {
          m_index.set(*m_index.get() - 1);
        } else if(operation.m_index == *m_index.get()) {
          search(m_value->get());
        }
      },
      [&] (const typename ListModel<SearchType>::UpdateOperation& operation) {
        if(m_index.get() && operation.m_index == *m_index.get()) {
          search(operation.get_previous());
        }
      });
  }

  template<typename T>
  void ListIndexValueModel<T>::on_current(const SearchType& current) {
    search(current);
  }
}

#endif
