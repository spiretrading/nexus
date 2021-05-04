#include "Spire/Ui/TranslatedTableModel.hpp"
#include <algorithm>

using namespace boost::signals2;
using namespace Spire;

TranslatedTableModel::ScopeExit::ScopeExit(std::function<void()> f)
  : m_f(std::move(f)) {}

TranslatedTableModel::ScopeExit::~ScopeExit() {
  m_f();
}

TranslatedTableModel::TranslatedTableModel(std::shared_ptr<TableModel> source)
    : m_source(source),
      m_transaction_level(0) {
  m_index_mapping.resize(source->get_row_size());
  std::iota(m_index_mapping.begin(), m_index_mapping.end(), 0);
  m_source_operation_connection = m_source->connect_operation_signal(
    [=] (const Operation& operation) {
      visit(operation,
        [=] (const AddOperation& add_operation) {
          std::transform(m_index_mapping.begin(), m_index_mapping.end(),
            m_index_mapping.begin(),
            [&] (auto index) {
              if(index >= add_operation.m_index) {
                return index + 1;
              }
              return index;
            });
          m_index_mapping.push_back(add_operation.m_index);
          push(AddOperation{get_row_size() - 1});
        },
        [=] (const MoveOperation& move_operation) {
          if(move_operation.m_source < move_operation.m_destination) {
            std::transform(m_index_mapping.begin(), m_index_mapping.end(),
              m_index_mapping.begin(),
              [&] (auto index) {
                if(index > move_operation.m_source &&
                    index <= move_operation.m_destination) {
                  return index - 1;
                } else if(index == move_operation.m_source) {
                  return move_operation.m_destination;
                }
                return index;
              });
          } else {
            std::transform(m_index_mapping.begin(), m_index_mapping.end(),
              m_index_mapping.begin(),
              [&] (auto index) {
                if(index >= move_operation.m_destination &&
                    index < move_operation.m_source) {
                  return index + 1;
                } else if(index == move_operation.m_source) {
                  return move_operation.m_destination;
                }
                return index;
              });
          }
        },
        [=] (const RemoveOperation& remove_operation) {
          auto removed_index = std::distance(m_index_mapping.begin(),
            m_index_mapping.erase(std::find(m_index_mapping.begin(),
            m_index_mapping.end(), remove_operation.m_index)));
          std::transform(m_index_mapping.begin(), m_index_mapping.end(),
            m_index_mapping.begin(),
            [&] (auto index) {
              if(index > remove_operation.m_index) {
                return index - 1;
              }
              return index;
            });
          push(RemoveOperation{removed_index});
        },
        [=] (const UpdateOperation& update_operation) {
          push(UpdateOperation{std::distance(m_index_mapping.begin(),
            std::find(m_index_mapping.begin(), m_index_mapping.end(),
            update_operation.m_row)), update_operation.m_column});
        });
    });
}

void TranslatedTableModel::move(int source, int destination) {
  if(source < 0 || source >= get_row_size() || destination < 0 ||
      destination >= get_row_size()) {
    throw std::out_of_range("The source or destination is out of range.");
  }
  if(source == destination) {
    return;
  }
  auto source_row = std::move(m_index_mapping[source]);
  if(source < destination) {
    std::move(std::next(m_index_mapping.begin(), source + 1),
      std::next(m_index_mapping.begin(), destination + 1),
      std::next(m_index_mapping.begin(), source));
  } else {
    std::move_backward(std::next(m_index_mapping.begin(), destination),
      std::next(m_index_mapping.begin(), source),
      std::next(m_index_mapping.begin(), source + 1));
  }
  m_index_mapping[destination] = std::move(source_row);
  push(MoveOperation{source, destination});
}

int TranslatedTableModel::get_row_size() const {
  return m_source->get_row_size();
}

int TranslatedTableModel::get_column_size() const {
  return m_source->get_column_size();
}

const std::any& TranslatedTableModel::at(int row, int column) const {
  if(row < 0 || row >= get_row_size() || column < 0 ||
      column >= get_column_size()) {
    throw std::out_of_range("The row or column is out of range.");
  }
  return m_source->at(m_index_mapping[row], column);
}

connection TranslatedTableModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  return m_operation_signal.connect(slot);
}

void TranslatedTableModel::push(Operation&& operation) {
  if(m_transaction_level > 0) {
    m_transaction.m_operations.push_back(std::move(operation));
  } else {
    m_operation_signal(operation);
  }
}
