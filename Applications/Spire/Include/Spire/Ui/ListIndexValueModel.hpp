#ifndef SPIRE_LIST_INDEX_VALUE_MODEL_HPP
#define SPIRE_LIST_INDEX_VALUE_MODEL_HPP
#include <type_traits>
#include <boost/signals2/shared_connection_block.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/ListModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Details {
  template<typename T>
  struct is_index_optional : std::false_type {};

  template<typename T>
  struct is_index_optional<boost::optional<T>> : std::true_type {};
}

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
      ListIndexValueModel(std::shared_ptr<ListModel> list,
        std::shared_ptr<ValueModel<SearchType>> value);

      /**
       * Returns the state of the current value, by default this is
       * <i>QValidator::State::Acceptable</i>
       */
      QValidator::State get_state() const override;

      /** Returns the current value. */
      const boost::optional<int>& get() const override;

      /**
       * Sets the current value. By default this operation is a no-op that
       * always returns <i>QValidator::State::Invalid</i>.
       */
      QValidator::State set(const boost::optional<int>& value) override;

      /** Connects a slot to the UpdateSignal. */
      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      static constexpr auto is_optional =
        Details::is_index_optional<SearchType>::value;
      using ListType = typename decltype([] {
        if constexpr(is_optional) {
          return std::type_identity<typename SearchType::value_type>();
        } else {
          return std::type_identity<SearchType>();
        }
      }())::type;
      LocalValueModel<boost::optional<int>> m_index;
      std::shared_ptr<ListModel> m_list;
      std::shared_ptr<ValueModel<SearchType>> m_value;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const SearchType& current);
  };

  template<typename T>
  ListIndexValueModel<T>::ListIndexValueModel(std::shared_ptr<ListModel> list,
      std::shared_ptr<ValueModel<SearchType>> value)
      : m_list(std::move(list)),
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
      if(m_value->set(m_list->get<ListType>(*value)) ==
          QValidator::Invalid) {
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
  void ListIndexValueModel<T>::on_current(const SearchType& current) {
    for(auto i = 0; i != m_list->get_size(); ++i) {
      try {
        if(m_list->get<ListType>(i) == current) {
          m_index.set(i);
          return;
        }
      } catch(const std::bad_any_cast&) {}
    }
    m_index.set(boost::none);
  }
}

#endif
