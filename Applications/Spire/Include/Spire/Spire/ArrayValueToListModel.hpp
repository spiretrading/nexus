#ifndef ARRAY_VALUE_TO_LIST_MODEL_HPP
#define ARRAY_VALUE_TO_LIST_MODEL_HPP
#include <vector>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"

namespace Spire {

  /**
   * Implements a model that converts a std::vector value model to a ListModel.
   * @param <T> The type of list elements.
   */
  template<typename T>
  class ArrayValueToListModel : public ListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;
      using Operation = typename ListModel<T>::Operation;
      using OperationSignal = ListModel<T>::OperationSignal;
      using AddOperation = typename ListModel<T>::AddOperation;
      using PreRemoveOperation = typename ListModel<T>::PreRemoveOperation;
      using RemoveOperation = typename ListModel<T>::RemoveOperation;
      using UpdateOperation = typename ListModel<T>::UpdateOperation;
      using StartTransaction = typename ListModel<T>::StartTransaction;
      using EndTransaction = typename ListModel<T>::EndTransaction;

      explicit ArrayValueToListModel(
        std::shared_ptr<ValueModel<std::vector<Type>>> source);

      int get_size() const override;
      const Type& get(int index) const override;
      QValidator::State set(int index, const Type& value) override;
      QValidator::State insert(const Type& value, int index) override;
      QValidator::State remove(int index) override;
      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;
      void transact(const std::function<void()>& transaction) override;

    private:
      std::shared_ptr<ValueModel<std::vector<Type>>> m_source;
      std::vector<Type> m_data;
      bool m_is_transaction;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_connection;

      void on_update(const std::vector<Type>& data);
  };

  template<typename T>
  ArrayValueToListModel<T>::ArrayValueToListModel(
    std::shared_ptr<ValueModel<std::vector<Type>>> source)
    : m_source(std::move(source)),
      m_data(m_source->get()),
      m_is_transaction(false),
      m_connection(m_source->connect_update_signal(
        std::bind_front(&ArrayValueToListModel::on_update, this))) {}

  template<typename T>
  int ArrayValueToListModel<T>::get_size() const {
    return static_cast<int>(m_data.size());
  }

  template<typename T>
  const ArrayValueToListModel<T>::Type&
      ArrayValueToListModel<T>::get(int index) const {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_data[index];
  }

  template<typename T>
  QValidator::State ArrayValueToListModel<T>::set(int index,
      const Type& value) {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    auto previous = m_data[index];
    m_data[index] = value;
    if(!m_is_transaction) {
      auto blocker = boost::signals2::shared_connection_block(m_connection);
      m_source->set(m_data);
    }
    m_transaction.push(UpdateOperation(index, std::move(previous), value));
    return QValidator::Acceptable;
  }

  template<typename T>
  QValidator::State ArrayValueToListModel<T>::insert(const Type& value,
      int index) {
    if(index < 0 || index > get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    m_data.insert(std::next(m_data.begin(), index), value);
    if(!m_is_transaction) {
      auto blocker = boost::signals2::shared_connection_block(m_connection);
      m_source->set(m_data);
    }
    m_transaction.push(AddOperation(index));
    return QValidator::Acceptable;
  }

  template<typename T>
  QValidator::State ArrayValueToListModel<T>::remove(int index) {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    m_transaction.transact([&] {
      m_transaction.push(PreRemoveOperation(index));
      m_data.erase(std::next(m_data.begin(), index));
      if(!m_is_transaction) {
        auto blocker = boost::signals2::shared_connection_block(m_connection);
        m_source->set(m_data);
      }
      m_transaction.push(RemoveOperation(index));
    });
    return QValidator::Acceptable;
  }

  template<typename T>
  boost::signals2::connection
      ArrayValueToListModel<T>::connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void ArrayValueToListModel<T>::transact(
      const std::function<void()>& transaction) {
    m_is_transaction = true;
    m_transaction.transact(transaction);
    auto blocker = boost::signals2::shared_connection_block(m_connection);
    m_source->set(m_data);
    m_is_transaction = false;
  }

  template<typename T>
  void ArrayValueToListModel<T>::on_update(const std::vector<Type>& data) {
    if(m_data == data) {
      return;
    }
    m_transaction.transact([&] {
      auto size = get_size();
      for(auto i = size - 1; i >= 0; --i) {
        m_transaction.push(PreRemoveOperation(i));
        m_data.erase(std::next(m_data.begin(), i));
        m_transaction.push(RemoveOperation(i));
      }
      for(const auto& value : data) {
        auto index = get_size();
        m_data.insert(std::next(m_data.begin(), index), value);
        m_transaction.push(AddOperation(index));
      }
    });
  }
}

#endif
