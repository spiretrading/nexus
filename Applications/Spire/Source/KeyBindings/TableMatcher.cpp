#include "Spire/KeyBindings/TableMatcher.hpp"

using namespace Spire;
using namespace rtv;

TableMatcher::TableMatcher(std::shared_ptr<TableModel> model,
    Builder builder)
    : m_model(std::move(model)),
      m_builder(std::move(builder)),
      m_operation_connection(m_model->connect_operation_signal(
        std::bind_front(&TableMatcher::on_operation, this))) {
  for(auto row = 0; row < m_model->get_row_size(); ++row) {
    auto trie = std::make_unique<Trie<QChar, char>>('\0');
    for(auto column = 0; column < m_model->get_column_size(); ++column) {
      for(auto& text : m_builder(to_any(m_model->at(row, column)))) {
        trie->insert(text.toLower().data(), char());
      }
    }
    m_rows.push_back(std::move(trie));
  }
}

bool TableMatcher::match(int row, const QString& text) const {
  return m_rows[row]->startsWith(text.toLower().data()) != m_rows[row]->end();
}

void TableMatcher::on_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      auto trie = std::make_unique<Trie<QChar, char>>('\0');
      for(auto column = 0; column < operation.m_row->get_size(); ++column) {
        for(auto& text : m_builder(operation.m_row->get(column))) {
          trie->insert(text.toLower().data(), char());
        }
      }
      m_rows.insert(std::next(m_rows.begin(), operation.m_index),
        std::move(trie));
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_rows.erase(std::next(m_rows.begin(), operation.m_index));
    },
    [&] (const TableModel::UpdateOperation& operation) {
      for(auto& text : m_builder(operation.m_previous)) {
        m_rows[operation.m_row]->erase(text.toLower().data());
      }
      for(auto& text : m_builder(operation.m_value)) {
        m_rows[operation.m_row]->insert(text.toLower().data(), char());
      }
    },
    [&] (const TableModel::MoveOperation& operation) {
      if(operation.m_source < operation.m_destination) {
        std::rotate(std::next(m_rows.begin(), operation.m_source),
          std::next(m_rows.begin(), operation.m_source + 1),
          std::next(m_rows.begin(), operation.m_destination + 1));
      } else {
        std::rotate(
          std::next(m_rows.rbegin(), m_rows.size() - operation.m_source - 1),
          std::next(m_rows.rbegin(), m_rows.size() - operation.m_source),
          std::next(m_rows.rbegin(), m_rows.size() - operation.m_destination));
      }
    });
}
