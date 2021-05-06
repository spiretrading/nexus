#include "Spire/Ui/TranslatedTableModel.hpp"
#include <algorithm>

using namespace boost::signals2;
using namespace Spire;

TranslatedTableModel::TranslatedTableModel(std::shared_ptr<TableModel> source)
    : m_source(std::move(source)),
      m_translation(m_source->get_row_size()),
      m_reverse_translation(m_source->get_row_size()) {
  std::iota(m_translation.begin(), m_translation.end(), 0);
  m_reverse_translation = m_translation;
  m_source_connection = m_source->connect_operation_signal(
    [=] (const auto& operation) { on_operation(operation); });
}

void TranslatedTableModel::move(int source, int destination) {
  if(source < 0 || source >= get_row_size() || destination < 0 ||
      destination >= get_row_size()) {
    throw std::out_of_range("The source or destination is out of range.");
  }
  if(source == destination) {
    return;
  }
  auto source_row = m_translation[source];
  auto direction = [&] {
    if(source < destination) {
      return 1;
    }
    return -1;
  }();
  m_reverse_translation[source_row] += destination - source;
  for(auto index = source; index != destination; index += direction) {
    m_translation[index] = m_translation[index + direction];
    m_reverse_translation[m_translation[index]] -= direction;
  }
  m_translation[destination] = source_row;
  m_transaction.push(MoveOperation{source, destination});
}

int TranslatedTableModel::get_row_size() const {
  return m_source->get_row_size();
}

int TranslatedTableModel::get_column_size() const {
  return m_source->get_column_size();
}

const std::any& TranslatedTableModel::at(int row, int column) const {
  if(row < 0 || row >= get_row_size()) {
    throw std::out_of_range("The row or column is out of range.");
  }
  return m_source->at(m_translation[row], column);
}

connection TranslatedTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void TranslatedTableModel::translate(int direction, int row) {
  auto reverse_row = m_reverse_translation[row];
  for(auto i = 0; i != static_cast<int>(m_translation.size()); ++i) {
    if(m_translation[i] >= row) {
      m_translation[i] += direction;
    }
    if(m_reverse_translation[i] >= reverse_row) {
      m_reverse_translation[i] += direction;
    }
  }
}

void TranslatedTableModel::on_operation(const Operation& operation) {
  transact([&] {
    visit(operation,
      [&] (const AddOperation& operation) {
        if(operation.m_index >= static_cast<int>(m_translation.size())) {
          m_reverse_translation.push_back(operation.m_index);
          m_translation.push_back(operation.m_index);
          m_transaction.push(AddOperation{operation.m_index});
          return;
        }
        auto reverse_index = m_reverse_translation[operation.m_index];
        translate(1, operation.m_index);
        m_translation.insert(m_translation.begin() + reverse_index,
          operation.m_index);
        m_reverse_translation.insert(
          m_reverse_translation.begin() + operation.m_index, reverse_index);
        m_transaction.push(AddOperation{reverse_index});
      },
      [&] (const MoveOperation& operation) {
        if(operation.m_source < operation.m_destination) {
          std::transform(m_translation.begin(), m_translation.end(),
            m_translation.begin(),
            [&] (auto index) {
              if(index > operation.m_source &&
                  index <= operation.m_destination) {
                return index - 1;
              } else if(index == operation.m_source) {
                return operation.m_destination;
              }
              return index;
            });
        } else {
          std::transform(m_translation.begin(), m_translation.end(),
            m_translation.begin(),
            [&] (auto index) {
              if(index >= operation.m_destination &&
                  index < operation.m_source) {
                return index + 1;
              } else if(index == operation.m_source) {
                return operation.m_destination;
              }
              return index;
            });
        }
      },
      [&] (const RemoveOperation& operation) {
        auto reverse_index = m_reverse_translation[operation.m_index];
        translate(-1, operation.m_index);
        m_translation.erase(m_translation.begin() + reverse_index);
        m_reverse_translation.erase(
          m_reverse_translation.begin() + operation.m_index);
        m_transaction.push(RemoveOperation{reverse_index});
      },
      [&] (const UpdateOperation& operation) {
        auto translated_row = m_reverse_translation[operation.m_row];
        m_transaction.push(UpdateOperation{translated_row, operation.m_column});
      });
  });
}
