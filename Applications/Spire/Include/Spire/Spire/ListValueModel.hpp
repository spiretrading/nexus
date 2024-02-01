#ifndef SPIRE_LIST_VALUE_MODEL_HPP
#define SPIRE_LIST_VALUE_MODEL_HPP
#include <functional>
#include <memory>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ValueModel providing a view over the value in a ListModel at
   * a specified index.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class ListValueModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;

      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /**
       * Constructs a ListValueModel from a specified index into a ListModel.
       * @param source The ListModel to view.
       * @param index The index of the value in the <i>source</i> to view. 
       */
      ListValueModel(std::shared_ptr<ListModel<Type>> source, int index);

      QValidator::State get_state() const override;

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<ListModel<Type>> m_source;
      int m_index;
      boost::signals2::scoped_connection m_source_connection;

      void on_operation(const typename ListModel<Type>::Operation& operation);
  };

  template<typename L>
  ListValueModel(std::shared_ptr<L>, int) -> ListValueModel<typename L::Type>;

  /**
   * Constructs a ListValueModel from a specified index into a ListModel.
   * @param source The ListModel to view.
   * @param index The index of the value in the <i>source</i> to view. 
   */
  template<typename T>
  auto make_list_value_model(std::shared_ptr<T> source, int index) {
    return std::make_shared<ListValueModel<typename T::Type>>(
      std::move(source), index);
  }

  template<typename T>
  ListValueModel<T>::ListValueModel(
      std::shared_ptr<ListModel<Type>> source, int index)
      : m_source(std::move(source)) {
    if(index < 0 || index >= m_source->get_size()) {
      m_index = -1;
      m_source = nullptr;
    } else {
      m_index = index;
      m_source_connection = m_source->connect_operation_signal(
        std::bind_front(&ListValueModel::on_operation, this));
    }
  }

  template<typename T>
  QValidator::State ListValueModel<T>::get_state() const {
    if(m_index == -1) {
      return QValidator::State::Invalid;
    }
    return QValidator::State::Acceptable;
  }

  template<typename T>
  const typename ListValueModel<T>::Type& ListValueModel<T>::get() const {
    if(m_index == -1) {
      throw std::out_of_range("Index out of range.");
    }
    return m_source->get(m_index);
  }

  template<typename T>
  QValidator::State ListValueModel<T>::test(const Type& value) const {
    if(m_index == -1) {
      return QValidator::State::Invalid;
    }
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State ListValueModel<T>::set(const Type& value) {
    if(m_index == -1) {
      return QValidator::State::Invalid;
    }
    return m_source->set(m_index, value);
  }

  template<typename T>
  boost::signals2::connection ListValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }

  template<typename T>
  void ListValueModel<T>::on_operation(
      const typename ListModel<Type>::Operation& operation) {
    visit(operation,
      [&] (const ListModel<Type>::AddOperation& operation) {
        if(m_index >= operation.m_index) {
          ++m_index;
        }
      },
      [&] (const ListModel<Type>::RemoveOperation& operation) {
        if(m_index == operation.m_index) {
          m_source_connection.disconnect();
          m_index = -1;
          m_source = nullptr;
        } else if(m_index > operation.m_index) {
          --m_index;
        }
      },
      [&] (const ListModel<Type>::MoveOperation& operation) {
        if(m_index == operation.m_source) {
          m_index = operation.m_destination;
        } else if(operation.m_source < operation.m_destination) {
          if(m_index > operation.m_source &&
              m_index <= operation.m_destination) {
            --m_index;
          }
        } else if(m_index >= operation.m_destination &&
            m_index < operation.m_source) {
          ++m_index;
        }
      },
      [&] (const ListModel<Type>::UpdateOperation& operation) {
        if(operation.m_index == m_index) {
          m_update_signal(get());
        }
      });
  }
}

#endif
