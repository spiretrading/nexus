#ifndef SPIRE_PROXY_LIST_MODEL_HPP
#define SPIRE_PROXY_LIST_MODEL_HPP
#include <memory>
#include <boost/optional/optional.hpp>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements a ListModel that forwards calls to an underlying ListModel which
   * can be replaced.
   * @param <T> The type of value being listed.
   */
  template<typename T>
  class ProxyListModel : public ListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;
      using OperationSignal = typename ListModel<T>::OperationSignal;
      using AddOperation = typename ListModel<T>::AddOperation;
      using PreRemoveOperation = typename ListModel<T>::PreRemoveOperation;
      using RemoveOperation = typename ListModel<T>::RemoveOperation;
      using Operation = typename ListModel<T>::Operation;

      /**
       * Constructs a ProxyListModel.
       * @param source The list to forward calls to.
       */
      explicit ProxyListModel(std::shared_ptr<ListModel<Type>> source);

      /** Returns the list being proxied. */
      const std::shared_ptr<ListModel<Type>>& get_source() const;

      /**
       * Sets the list to forward calls to. The values of the previous source
       * are removed and the values of the new source are added within a single
       * transaction.
       * @param source The list to forward calls to.
       */
      void set_source(std::shared_ptr<ListModel<Type>> source);

      int get_size() const override;
      const Type& get(int index) const override;
      QValidator::State set(int index, const Type& value) override;
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
      std::shared_ptr<ListModel<Type>> m_source;
      boost::optional<int> m_size;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_connection;

      void on_operation(const Operation& operation);
  };

  template<typename T>
  ProxyListModel(std::shared_ptr<T>) -> ProxyListModel<typename T::Type>;

  template<typename T>
  ProxyListModel<T>::ProxyListModel(std::shared_ptr<ListModel<Type>> source)
      : m_source(std::move(source)) {
    m_connection = m_source->connect_operation_signal(
      std::bind_front(&ProxyListModel::on_operation, this));
  }

  template<typename T>
  const std::shared_ptr<ListModel<typename ProxyListModel<T>::Type>>&
      ProxyListModel<T>::get_source() const {
    return m_source;
  }

  template<typename T>
  void ProxyListModel<T>::set_source(std::shared_ptr<ListModel<Type>> source) {
    m_connection.disconnect();
    auto end = [&] {
      m_size = boost::none;
      m_connection = m_source->connect_operation_signal(
        std::bind_front(&ProxyListModel::on_operation, this));
    };
    try {
      m_transaction.transact([&] {
        m_size = m_source->get_size();
        while(*m_size != 0) {
          m_transaction.push(PreRemoveOperation(*m_size - 1));
          --*m_size;
          m_transaction.push(RemoveOperation(*m_size));
        }
        m_source = std::move(source);
        auto size = m_source->get_size();
        while(*m_size != size) {
          ++*m_size;
          m_transaction.push(AddOperation(*m_size - 1));
        }
      });
    } catch(...) {
      if(source) {
        m_source = std::move(source);
      }
      end();
      throw;
    }
    end();
  }

  template<typename T>
  int ProxyListModel<T>::get_size() const {
    if(m_size) {
      return *m_size;
    }
    return m_source->get_size();
  }

  template<typename T>
  const typename ProxyListModel<T>::Type&
      ProxyListModel<T>::get(int index) const {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_source->get(index);
  }

  template<typename T>
  QValidator::State ProxyListModel<T>::set(int index, const Type& value) {
    return m_source->set(index, value);
  }

  template<typename T>
  QValidator::State ProxyListModel<T>::insert(const Type& value, int index) {
    return m_source->insert(value, index);
  }

  template<typename T>
  QValidator::State ProxyListModel<T>::move(int source, int destination) {
    return m_source->move(source, destination);
  }

  template<typename T>
  QValidator::State ProxyListModel<T>::remove(int index) {
    return m_source->remove(index);
  }

  template<typename T>
  boost::signals2::connection ProxyListModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void ProxyListModel<T>::transact(const std::function<void ()>& transaction) {
    m_transaction.transact([&] {
      transaction();
    });
  }

  template<typename T>
  void ProxyListModel<T>::on_operation(const Operation& operation) {
    m_transaction.push(operation);
  }
}

#endif
