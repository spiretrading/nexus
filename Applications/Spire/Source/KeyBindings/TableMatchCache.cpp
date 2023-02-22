#include "Spire/KeyBindings/TableMatchCache.hpp"

using namespace Spire;

TableMatchCache::TableMatchCache(std::shared_ptr<TableModel> table,
    MatcherBuilder builder)
    : m_table(std::move(table)),
      m_builder(std::move(builder)),
      m_operation_connection(m_table->connect_operation_signal(
        std::bind_front(&TableMatchCache::on_operation, this))) {
  for(auto row = 0; row < m_table->get_row_size(); ++row) {
    auto cache = std::vector<MatchCache>();
    for(auto column = 0; column < m_table->get_column_size(); ++column) {
      cache.emplace_back(m_builder(m_table, row, column));
    }
    m_caches.push_back(std::move(cache));
  }
}

bool TableMatchCache::matches(int row, const QString& query) {
  for(auto column = 0; column < m_table->get_column_size(); ++column) {
    if(m_caches[row][column].matches(query)) {
      return true;
    }
  }
  return false;
}

void TableMatchCache::on_operation(const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      auto cache = std::vector<MatchCache>();
      for(auto column = 0; column < m_table->get_column_size(); ++column) {
        cache.emplace_back(m_builder(m_table, operation.m_index, column));
      }
      m_caches.insert(std::next(m_caches.begin(), operation.m_index),
        std::move(cache));
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_caches.erase(std::next(m_caches.begin(), operation.m_index));
    },
    [&] (const TableModel::UpdateOperation& operation) {
      m_caches[operation.m_row][operation.m_column] = MatchCache(
        m_builder(m_table, operation.m_row, operation.m_column));
    },
    [&] (const TableModel::MoveOperation& operation) {
      if(operation.m_source < operation.m_destination) {
        std::rotate(std::next(m_caches.begin(), operation.m_source),
          std::next(m_caches.begin(), operation.m_source + 1),
          std::next(m_caches.begin(), operation.m_destination + 1));
      } else {
        std::rotate(
          std::next(m_caches.rbegin(),
            m_caches.size() - operation.m_source - 1),
          std::next(m_caches.rbegin(), m_caches.size() - operation.m_source),
          std::next(m_caches.rbegin(),
            m_caches.size() - operation.m_destination));
      }
    });
}
