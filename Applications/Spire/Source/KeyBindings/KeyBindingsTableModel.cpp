#include "Spire/KeyBindings/KeyBindingsTableModel.hpp"
#include <QColor>

using namespace Spire;

KeyBindingsTableModel::KeyBindingsTableModel(QObject* parent)
  : QAbstractTableModel(parent) {}

void KeyBindingsTableModel::set_focus_highlight(const QModelIndex& index) {
  m_focused_cell = index;
  emit dataChanged(m_focused_cell, m_focused_cell,
    {Qt::BackgroundRole});
}

void KeyBindingsTableModel::reset_focus_highlight() {
  if(m_focused_cell.isValid()) {
    auto old_index = m_focused_cell;
    m_focused_cell = QModelIndex();
    emit dataChanged(old_index, old_index, {Qt::BackgroundRole});
  }
}

void KeyBindingsTableModel::set_hover_highlight(const QModelIndex& index) {
  m_hovered_cell = index;
  emit dataChanged(m_hovered_cell, m_hovered_cell,
    {Qt::BackgroundRole});
}

void KeyBindingsTableModel::reset_hover_highlight() {
  if(m_hovered_cell.isValid()) {
    auto old_index = m_hovered_cell;
    m_hovered_cell = QModelIndex();
    emit dataChanged(old_index, old_index, {Qt::BackgroundRole});
  }
}

void KeyBindingsTableModel::set_row_highlight(int row) {
  m_highlighted_row = row;
  auto highlight_index = index(row, 0);
  emit dataChanged(highlight_index, highlight_index, {Qt::BackgroundRole});
}

void KeyBindingsTableModel::reset_row_highlight() {
  if(m_highlighted_row) {
    auto highlight_index = index(*m_highlighted_row, 0);
    m_highlighted_row = boost::none;
    emit dataChanged(highlight_index, highlight_index, {Qt::BackgroundRole});
  }
}

QVariant KeyBindingsTableModel::data(const QModelIndex& index,
    int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  if(role == Qt::BackgroundRole) {
    if(m_highlighted_row && index.row() == m_highlighted_row) {
      return QColor("#FFF1F1");
    } else if(index == m_focused_cell || index == m_hovered_cell) {
      return QColor("#F2F2FF");
    }
    return QColor(Qt::white);
  }
  return QVariant();
}
