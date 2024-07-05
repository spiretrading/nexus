#include "Spire/KeyBindings/KeywordFilteredTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

KeywordFilteredTableModel::KeywordFilteredTableModel(
  std::shared_ptr<TableModel> source, std::shared_ptr<TextModel> keywords)
  : m_filtered_model(std::move(source),
      [] (const auto& value, auto index) {
        return false;
      }),
    m_keywords(std::move(keywords)) {}

int KeywordFilteredTableModel::get_row_size() const {
  return m_filtered_model.get_row_size();
}

int KeywordFilteredTableModel::get_column_size() const {
  return m_filtered_model.get_column_size();
}

AnyRef KeywordFilteredTableModel::at(int row, int column) const {
  return m_filtered_model.at(row, column);
}

QValidator::State KeywordFilteredTableModel::set(
    int row, int column, const std::any& value) {
  return m_filtered_model.set(row, column, value);
}

QValidator::State KeywordFilteredTableModel::remove(int row) {
  return m_filtered_model.remove(row);
}

connection KeywordFilteredTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_filtered_model.connect_operation_signal(slot);
}

void KeywordFilteredTableModel::on_keywords(const QString& keywords) {}
