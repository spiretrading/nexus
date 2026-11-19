#ifndef ARRAY_VALUE_TO_LIST_MODEL_HPP
#define ARRAY_VALUE_TO_LIST_MODEL_HPP
#include <vector>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TransactionLogModel.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a model that converts a std::vector value model to a ListModel.
   * @param <T> The type of list elements.
   */
  template<typename T>
  class ArrayValueToListModel : public TransactionLogModel<ListModel<T>> {
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
      void start_transaction() override;
      void end_transaction() override;

    private:
      std::shared_ptr<ValueModel<std::vector<Type>>> m_source;
      std::vector<Type> m_data;
      int m_transaction_depth;
      boost::signals2::scoped_connection m_connection;

      void update_source();
      void publish(const Operation& operation);
      void on_update(const std::vector<Type>& data);
  };

  template<typename M>
  ArrayValueToListModel(std::shared_ptr<M>) ->
    ArrayValueToListModel<typename M::Type::value_type>;

  template<typename T>
  ArrayValueToListModel<T>::ArrayValueToListModel(
      std::shared_ptr<ValueModel<std::vector<Type>>> source)
      : m_source(std::move(source)),
        m_data(m_source->get()),
        m_transaction_depth(0) {
    this->m_transaction.set_size(get_size());
    m_connection = m_source->connect_update_signal(
      std::bind_front(&ArrayValueToListModel::on_update, this));
  }

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
    publish(UpdateOperation(index, std::move(previous), value));
    return QValidator::Acceptable;
  }

  template<typename T>
  QValidator::State ArrayValueToListModel<T>::insert(const Type& value,
      int index) {
    if(index < 0 || index > get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    m_data.insert(std::next(m_data.begin(), index), value);
    publish(AddOperation(index));
    return QValidator::Acceptable;
  }

  template<typename T>
  QValidator::State ArrayValueToListModel<T>::remove(int index) {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    this->m_transaction.transact([&] {
      this->m_transaction.push(PreRemoveOperation(index));
      m_data.erase(std::next(m_data.begin(), index));
      publish(RemoveOperation(index));
    });
    return QValidator::Acceptable;
  }

  template<typename T>
  void ArrayValueToListModel<T>::start_transaction() {
    ++m_transaction_depth;
    TransactionLogModel<ListModel<T>>::start_transaction();
  }

  template<typename T>
  void ArrayValueToListModel<T>::end_transaction() {
    auto flush = [&] {
      --m_transaction_depth;
      update_source();
    };
    try {
      TransactionLogModel<ListModel<T>>::end_transaction();
    } catch(...) {
      flush();
      throw;
    }
    flush();
  }

  template<typename T>
  void ArrayValueToListModel<T>::update_source() {
    if(m_transaction_depth != 0) {
      return;
    }
    auto blocker = boost::signals2::shared_connection_block(m_connection);
    m_source->set(m_data);
  }

  template<typename T>
  void ArrayValueToListModel<T>::publish(const Operation& operation) {
    try {
      update_source();
    } catch(...) {
      this->m_transaction.push(operation);
      throw;
    }
    this->m_transaction.push(operation);
  }

  template<typename T>
  void ArrayValueToListModel<T>::on_update(const std::vector<Type>& data) {
    if(m_data == data) {
      return;
    }
    this->m_transaction.transact([&] {
      auto size = get_size();
      for(auto i = size - 1; i >= 0; --i) {
        this->m_transaction.push(PreRemoveOperation(i));
        m_data.erase(std::next(m_data.begin(), i));
        this->m_transaction.push(RemoveOperation(i));
      }
      for(const auto& value : data) {
        auto index = get_size();
        m_data.insert(std::next(m_data.begin(), index), value);
        this->m_transaction.push(AddOperation(index));
      }
    });
  }
}

#endif
