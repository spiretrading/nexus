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
      struct RemovedValue {
        int m_index;
        Type m_value;
      };
      using RowChange = std::variant<AddOperation, RemovedValue>;
      std::shared_ptr<ValueModel<std::vector<Type>>> m_source;
      std::vector<Type> m_data;
      std::vector<RowChange> m_changes;
      int m_transaction_depth;
      boost::signals2::scoped_connection m_connection;

      template<typename O>
      QValidator::State apply(const O& operation, auto edit);
      void insert_row(int index, const Type& value);
      void remove_row(int index);
      void restore(const std::vector<Type>& data,
        const std::vector<RowChange>& changes);
      QValidator::State write(const std::vector<Type>& data, auto synchronize);
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
    auto operation = UpdateOperation(index, m_data[index], value);
    return apply(operation, [&] (auto& data) {
      data[index] = operation.get_value();
    });
  }

  template<typename T>
  QValidator::State ArrayValueToListModel<T>::insert(const Type& value,
      int index) {
    if(index < 0 || index > get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return apply(AddOperation(index), [=] (auto& data) {
      data.insert(std::next(data.begin(), index), value);
    });
  }

  template<typename T>
  QValidator::State ArrayValueToListModel<T>::remove(int index) {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return apply(RemoveOperation(index), [&] (auto& data) {
      data.erase(std::next(data.begin(), index));
    });
  }

  template<typename T>
  void ArrayValueToListModel<T>::start_transaction() {
    ++m_transaction_depth;
    TransactionLogModel<ListModel<T>>::start_transaction();
  }

  template<typename T>
  void ArrayValueToListModel<T>::end_transaction() {
    if(--m_transaction_depth == 0) {
      auto changes = std::exchange(m_changes, {});
      try {
        write(m_data, [&] (bool accepted) {
          auto& stored = m_source->get();
          if(accepted) {
            on_update(stored);
          } else {
            restore(stored, changes);
          }
        });
      } catch(...) {
        TransactionLogModel<ListModel<T>>::end_transaction();
        throw;
      }
    }
    TransactionLogModel<ListModel<T>>::end_transaction();
  }

  template<typename T>
  template<typename O>
  QValidator::State ArrayValueToListModel<T>::apply(const O& operation,
      auto edit) {
    auto publish = [&] {
      if constexpr(std::same_as<O, RemoveOperation>) {
        remove_row(operation.m_index);
      } else {
        edit(m_data);
        if constexpr(std::same_as<O, AddOperation>) {
          if(m_transaction_depth != 0) {
            m_changes.emplace_back(operation);
          }
        }
        this->m_transaction.push(operation);
      }
    };
    if(m_transaction_depth != 0) {
      publish();
      return QValidator::Acceptable;
    }
    auto data = m_data;
    edit(data);
    return write(data, [&] (bool accepted) {
      auto& stored = m_source->get();
      if(accepted && stored == data) {
        publish();
      } else {
        on_update(stored);
      }
    });
  }

  template<typename T>
  void ArrayValueToListModel<T>::insert_row(int index, const Type& value) {
    m_data.insert(m_data.begin() + index, value);
    this->m_transaction.push(AddOperation(index));
  }

  template<typename T>
  void ArrayValueToListModel<T>::remove_row(int index) {
    this->m_transaction.transact([&] {
      auto erase = [&] {
        m_data.erase(m_data.begin() + index);
        this->m_transaction.push(RemoveOperation(index));
      };
      try {
        this->m_transaction.push(PreRemoveOperation(index));
      } catch(...) {
        if(m_transaction_depth == 0) {
          erase();
        }
        throw;
      }
      if(m_transaction_depth != 0) {
        m_changes.emplace_back(RemovedValue(index, m_data[index]));
      }
      erase();
    });
  }

  template<typename T>
  void ArrayValueToListModel<T>::restore(
      const std::vector<Type>& data, const std::vector<RowChange>& changes) {
    for(auto i = std::ssize(changes); i-- > 0;) {
      visit(changes[i],
        [&] (const AddOperation& add) {
          remove_row(add.m_index);
        },
        [&] (const RemovedValue& removed) {
          insert_row(removed.m_index, removed.m_value);
        });
    }
    if(m_data.size() != data.size()) {
      on_update(data);
      return;
    }
    for(auto i = 0; i != get_size(); ++i) {
      if(m_data[i] != data[i]) {
        auto previous = std::exchange(m_data[i], data[i]);
        this->m_transaction.push(
          UpdateOperation(i, std::move(previous), data[i]));
      }
    }
  }

  template<typename T>
  QValidator::State ArrayValueToListModel<T>::write(
      const std::vector<Type>& data, auto synchronize) {
    auto state = QValidator::Invalid;
    try {
      auto blocker = boost::signals2::shared_connection_block(m_connection);
      state = m_source->set(data);
    } catch(...) {
      auto& stored = m_source->get();
      if(stored != m_data) {
        synchronize(stored == data);
      }
      throw;
    }
    synchronize(state != QValidator::Invalid);
    return state;
  }

  template<typename T>
  void ArrayValueToListModel<T>::on_update(const std::vector<Type>& data) {
    if(m_data == data) {
      return;
    }
    m_changes.clear();
    this->m_transaction.transact([&] {
      auto size = get_size();
      for(auto i = size - 1; i >= 0; --i) {
        this->m_transaction.push(PreRemoveOperation(i));
        m_data.erase(std::next(m_data.begin(), i));
        this->m_transaction.push(RemoveOperation(i));
      }
      for(const auto& value : data) {
        auto index = get_size();
        insert_row(index, value);
      }
    });
  }
}

#endif
