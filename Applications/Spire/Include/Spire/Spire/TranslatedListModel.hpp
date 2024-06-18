#ifndef SPIRE_TRANSLATED_LIST_MODEL_HPP
#define SPIRE_TRANSLATED_LIST_MODEL_HPP
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"

namespace Spire {

  /**
   * Implements a ListModel that provides a view over a source list by
   * translating its indicies.
   */
  template<typename T>
  class TranslatedListModel : public ListModel<T> {
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
       * Constructs a TranslatedListModel from a ListModel.
       * @param source The list to translate.
       */
      explicit TranslatedListModel(std::shared_ptr<ListModel<Type>> source);

      /**
       * Given an index into the source list, returns the index in this
       * translated list.
       */
      int get_source_to_translation(int index) const;

      /**
       * Given an index into this list, returns the index in the source list.
       */
      int get_translation_to_source(int index) const;

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
      std::vector<int> m_translation;
      std::vector<int> m_reverse_translation;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      void translate(int direction, int index);
      void on_operation(const Operation& operation);
  };

  template<typename T>
  TranslatedListModel<T>::TranslatedListModel(
      std::shared_ptr<ListModel<Type>> source)
      : m_source(std::move(source)),
        m_translation(m_source->get_size()),
        m_reverse_translation(m_source->get_size()) {
    std::iota(m_translation.begin(), m_translation.end(), 0);
    m_reverse_translation = m_translation;
    m_source_connection = m_source->connect_operation_signal(
      std::bind_front(&TranslatedListModel::on_operation, this));
  }

  template<typename T>
  int TranslatedListModel<T>::get_source_to_translation(int index) const {
    return m_reverse_translation[index];
  }

  template<typename T>
  int TranslatedListModel<T>::get_translation_to_source(int index) const {
    return m_translation[index];
  }

  template<typename T>
  int TranslatedListModel<T>::get_size() const {
    return m_source->get_size();
  }

  template<typename T>
  const typename TranslatedListModel<T>::Type&
      TranslatedListModel<T>::get(int index) const {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_source->get(m_translation[index]);
  }

  template<typename T>
  QValidator::State TranslatedListModel<T>::set(int index, const Type& value) {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    return m_source->set(m_translation[index], value);
  }

  template<typename T>
  QValidator::State TranslatedListModel<T>::insert(
      const Type& value, int index) {
    return QValidator::State::Invalid;
  }

  template<typename T>
  QValidator::State TranslatedListModel<T>::move(int source, int destination) {
    if(source < 0 || source >= get_size() || destination < 0 ||
        destination >= get_size()) {
      throw std::out_of_range("The source or destination is out of range.");
    }
    if(source == destination) {
      return QValidator::State::Acceptable;
    }
    auto source_index = m_translation[source];
    auto direction = [&] {
      if(source < destination) {
        return 1;
      }
      return -1;
    }();
    m_reverse_translation[source_index] += destination - source;
    for(auto index = source; index != destination; index += direction) {
      m_translation[index] = m_translation[index + direction];
      m_reverse_translation[m_translation[index]] -= direction;
    }
    m_translation[destination] = source_index;
    m_transaction.push(MoveOperation(source, destination));
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State TranslatedListModel<T>::remove(int index) {
    return m_source->remove(m_translation[index]);
  }

  template<typename T>
  boost::signals2::connection TranslatedListModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void TranslatedListModel<T>::transact(
      const std::function<void ()>& transaction) {
    m_transaction.transact(transaction);
  }

  template<typename T>
  void TranslatedListModel<T>::translate(int direction, int index) {
    auto reverse_index = m_reverse_translation[index];
    for(auto i = 0; i != static_cast<int>(m_translation.size()); ++i) {
      if(m_translation[i] >= index) {
        m_translation[i] += direction;
      }
      if(m_reverse_translation[i] >= reverse_index) {
        m_reverse_translation[i] += direction;
      }
    }
  }

  template<typename T>
  void TranslatedListModel<T>::on_operation(const Operation& operation) {
    visit(operation,
      [&] (const StartTransaction&) {
        m_transaction.start();
      },
      [&] (const EndTransaction&) {
        m_transaction.end();
      },
      [&] (const AddOperation& operation) {
        if(operation.m_index >= static_cast<int>(m_translation.size())) {
          m_reverse_translation.push_back(operation.m_index);
          m_translation.push_back(operation.m_index);
          m_transaction.push(operation);
          return;
        }
        auto reverse_index = m_reverse_translation[operation.m_index];
        translate(1, operation.m_index);
        m_translation.insert(
          m_translation.begin() + reverse_index, operation.m_index);
        m_reverse_translation.insert(
          m_reverse_translation.begin() + operation.m_index, reverse_index);
        m_transaction.push(AddOperation(reverse_index));
      },
      [&] (const MoveOperation& operation) {
        auto direction = [&] {
          if(operation.m_source < operation.m_destination) {
            return 1;
          }
          return -1;
        }();
        for(auto i = 0; i != static_cast<int>(m_translation.size()); ++i) {
          auto& index = m_translation[i];
          if(direction * index > direction * operation.m_source &&
              direction * index <= direction * operation.m_destination) {
            index -= direction;
          } else if(index == operation.m_source) {
            index = operation.m_destination;
          }
        }
        for(auto i = 0; i != static_cast<int>(m_translation.size()); ++i) {
          m_reverse_translation[m_translation[i]] = i;
        }
      },
      [&] (const PreRemoveOperation& operation) {
        auto reverse_index = m_reverse_translation[operation.m_index];
        m_transaction.push(PreRemoveOperation(reverse_index));
      },
      [&] (const RemoveOperation& operation) {
        auto reverse_index = m_reverse_translation[operation.m_index];
        translate(-1, operation.m_index);
        m_translation.erase(m_translation.begin() + reverse_index);
        m_reverse_translation.erase(
          m_reverse_translation.begin() + operation.m_index);
        m_transaction.push(RemoveOperation(reverse_index));
      },
      [&] (const UpdateOperation& operation) {
        auto translated_index = m_reverse_translation[operation.m_index];
        m_transaction.push(UpdateOperation(
          translated_index, operation.get_previous(), operation.get_value()));
      });
  }

}

#endif
