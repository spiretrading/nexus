#ifndef SPIRE_ROW_VIEW_LIST_MODEL_HPP
#define SPIRE_ROW_VIEW_LIST_MODEL_HPP
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableRowIndexTracker.hpp"

namespace Spire {
namespace Details {
  template<typename T>
  struct AnyRefRowViewListModel {};

  template<>
  struct AnyRefRowViewListModel<AnyRef> {
    mutable AnyRef m_reference;
  };
}

  /**
   * Implements a ListModel that provides a view into a single row of a source
   * table model.
   * @param <T> The type of values stored in the row to view.
   */
  template<typename T>
  class RowViewListModel :
      public ListModel<T>, private Details::AnyRefRowViewListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;

      using OperationSignal = ListModel<T>::OperationSignal;

      using AddOperation = typename ListModel<T>::AddOperation;

      using MoveOperation = typename ListModel<T>::MoveOperation;

      using PreRemoveOperation = typename ListModel<T>::PreRemoveOperation;

      using RemoveOperation = typename ListModel<T>::RemoveOperation;

      using UpdateOperation = typename ListModel<T>::UpdateOperation;

      using StartTransaction = typename ListModel<T>::StartTransaction;

      using EndTransaction = typename ListModel<T>::EndTransaction;

      /**
       * Constructs a RowViewListModel from a specified row of the table model.
       * @param source The table model.
       * @param row The index of the row to be viewed. 
       */
      RowViewListModel(std::shared_ptr<TableModel> source, int row);

      int get_size() const override;

      const Type& get(int index) const override;

      QValidator::State set(int index, const Type& value) override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

      using ListModel<T>::transact;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      std::shared_ptr<TableModel> m_source;
      TableRowIndexTracker m_row;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      void on_operation(const TableModel::Operation& operation);
  };

  template<typename T>
  RowViewListModel<T>::RowViewListModel(
      std::shared_ptr<TableModel> source, int row)
      : m_source(std::move(source)),
        m_row(row) {
    if(row < 0 || row >= m_source->get_row_size()) {
      m_source = nullptr;
      m_row.set(-1);
    } else {
      m_source_connection = m_source->connect_operation_signal(
        std::bind_front(&RowViewListModel::on_operation, this));
    }
  }

  template<typename T>
  int RowViewListModel<T>::get_size() const {
    if(m_row.get_index() == -1) {
      return 0;
    }
    return m_source->get_column_size();
  }

  template<typename T>
  const typename RowViewListModel<T>::Type& RowViewListModel<T>::get(int index)
      const {
    if(m_row.get_index() == -1) {
      throw std::out_of_range("The row is out of range.");
    }
    if constexpr(std::is_same_v<Type, AnyRef>) {
      this->m_reference = m_source->at(m_row.get_index(), index);
      return this->m_reference;
    } else {
      return m_source->get<Type>(m_row.get_index(), index);
    }
  }

  template<typename T>
  QValidator::State RowViewListModel<T>::set(int index, const Type& value) {
    if(m_row.get_index() == -1) {
      return QValidator::State::Invalid;
    }
    return m_source->set(m_row.get_index(), index, value);
  }

  template<typename T>
  boost::signals2::connection RowViewListModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void RowViewListModel<T>::transact(
      const std::function<void ()>& transaction) {
    m_transaction.transact(transaction);
  }

  template<typename T>
  void RowViewListModel<T>::on_operation(
      const TableModel::Operation& operation) {
    visit(operation,
      [&] (const TableModel::StartTransaction&) {
        m_transaction.start();
      },
      [&] (const TableModel::EndTransaction&) {
        m_transaction.end();
      },
      [&] (const TableModel::UpdateOperation& operation) {
        if(m_row.get_index() == operation.m_row) {
          m_transaction.push(UpdateOperation(operation.m_column,
            std::any_cast<const Type&>(operation.m_previous),
            std::any_cast<const Type&>(operation.m_value)));
        }
      },
      [&] (const auto& operation) {
        m_row.update(operation);
        if(m_row.get_index() == -1) {
          m_source_connection.disconnect();
          m_source = nullptr;
        }
      });
  }
}

#endif
