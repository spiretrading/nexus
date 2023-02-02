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
    auto tries = std::vector<std::unique_ptr<Trie<QChar, char>>>();
    for(auto column = 0; column < m_model->get_column_size(); ++column) {
      auto trie = std::make_unique<Trie<QChar, char>>('\0');
      for(auto& text : m_builder(to_any(m_model->at(row, column)))) {
        trie->insert(text.toLower().data(), char());
      }
      tries.push_back(std::move(trie));
    }
    m_rows.push_back(std::move(tries));
  }
}

bool TableMatcher::match(int row, const QString& text) const {
  auto prefix = text.toLower().data();
  for(auto column = 0; column < m_model->get_column_size(); ++column) {
    if(m_rows[row][column]->startsWith(prefix) != m_rows[row][column]->end()) {
      return true;
    }
  }
  return false;
}

void TableMatcher::on_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      auto tries = std::vector<std::unique_ptr<Trie<QChar, char>>>();
      for(auto column = 0; column < operation.m_row->get_size(); ++column) {
        auto trie = std::make_unique<Trie<QChar, char>>('\0');
        for(auto& text : m_builder(operation.m_row->get(column))) {
          trie->insert(text.toLower().data(), char());
        }
        tries.push_back(std::move(trie));
      }
      m_rows.insert(std::next(m_rows.begin(), operation.m_index),
        std::move(tries));
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_rows.erase(std::next(m_rows.begin(), operation.m_index));
    },
    [&] (const TableModel::UpdateOperation& operation) {
      auto trie = std::make_unique<Trie<QChar, char>>('\0');
      for(auto& text : m_builder(operation.m_value)) {
        trie->insert(text.toLower().data(), char());
      }
      m_rows[operation.m_row][operation.m_column] = std::move(trie);
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
