#include "Spire/KeyBindings/TableMatchCache.hpp"

using namespace Spire;

TableMatchCache::TableMatchCache(std::shared_ptr<TableModel> model,
    Matcher matcher)
    : m_model(std::move(model)),
      m_matcher(std::move(matcher)),
      m_operation_connection(m_model->connect_operation_signal(
        std::bind_front(&TableMatchCache::on_operation, this))) {
  for(auto row = 0; row < m_model->get_row_size(); ++row) {
    auto caches = std::vector<MatchCache>();
    for(auto column = 0; column < m_model->get_column_size(); ++column) {
      caches.push_back(MatchCache(m_matcher));
    }
    m_caches.push_back(std::move(caches));
  }
}

bool TableMatchCache::matches(int row, int column, const QString& source) {
  return m_caches[row][column].matches(
    to_any(m_model->at(row, column)), source);
}

void TableMatchCache::on_operation(const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      auto caches = std::vector<MatchCache>();
      for(auto column = 0; column < m_model->get_column_size(); ++column) {
        caches.push_back(MatchCache(m_matcher));
      }
      m_caches.insert(std::next(m_caches.begin(), operation.m_index),
        std::move(caches));
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_caches.erase(std::next(m_caches.begin(), operation.m_index));
    },
    [&] (const TableModel::UpdateOperation& operation) {
      m_caches[operation.m_row][operation.m_column] = MatchCache(m_matcher);
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
