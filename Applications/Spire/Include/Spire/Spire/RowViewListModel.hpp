#ifndef SPIRE_ROW_VIEW_LIST_MODEL_HPP
#define SPIRE_ROW_VIEW_LIST_MODEL_HPP
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/ListModelTransactionLog.hpp"
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a ListModel that provides a view into a single row of a source
   * table model.
   * @param <T> The type of values stored in the row to view.
   */
  template<typename T>
  class RowViewListModel : public ListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;

      using OperationSignal = typename ListModel<T>::OperationSignal;

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
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      int m_row;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      void on_operation(const TableModel::Operation& operation);
  };

  template<typename T>
  RowViewListModel<T>::RowViewListModel(
      std::shared_ptr<TableModel> source, int row)
      : m_source(std::move(source)) {
    if(row < 0 || row >= m_source->get_row_size()) {
      m_row = -1;
      m_source = nullptr;
    } else {
      m_row = row;
      m_source_connection = m_source->connect_operation_signal(
        [=] (const auto& operation) { on_operation(operation); });
    }
  }

  template<typename T>
  int RowViewListModel<T>::get_size() const {
    if(m_row == -1) {
      return 0;
    }
    return m_source->get_column_size();
  }

  template<typename T>
  const typename RowViewListModel<T>::Type& RowViewListModel<T>::get(int index)
      const {
    if(m_row == -1) {
      throw std::out_of_range("The row is out of range.");
    }
    return m_source->get<Type>(m_row, index);
  }

  template<typename T>
  QValidator::State RowViewListModel<T>::set(int index, const Type& value) {
    if(m_row == -1) {
      return QValidator::State::Invalid;
    }
    return m_source->set(m_row, index, value);
  }

  template<typename T>
  boost::signals2::connection RowViewListModel<T>::connect_operation_signal(
      const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T>
  void RowViewListModel<T>::on_operation(
      const TableModel::Operation& operation) {
    m_transaction.transact([&] {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          if(m_row >= operation.m_index) {
            ++m_row;
          }
        },
        [&] (const TableModel::MoveOperation& operation) {
          if(m_row == operation.m_source) {
            m_row = operation.m_destination;
          } else if(operation.m_source < operation.m_destination) {
            if(m_row > operation.m_source && m_row <= operation.m_destination) {
              --m_row;
            }
          } else if(m_row >= operation.m_destination &&
              m_row < operation.m_source) {
            ++m_row;
          }
        },
        [&] (const TableModel::RemoveOperation& operation) {
          if(m_row == operation.m_index) {
            m_source_connection.disconnect();
            m_row = -1;
            m_source = nullptr;
          } else if(m_row > operation.m_index) {
            --m_row;
          }
        },
        [&] (const TableModel::UpdateOperation& operation) {
          if(m_row == operation.m_row) {
            m_transaction.push(
              AnyListModel::UpdateOperation(operation.m_column));
          }
        });
      });
  }

}

#endif