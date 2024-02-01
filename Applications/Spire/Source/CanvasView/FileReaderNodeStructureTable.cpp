#include "Spire/CanvasView/FileReaderNodeStructureTable.hpp"
#include <QApplication>
#include <QComboBox>
#include <QKeyEvent>
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"
#include "Spire/CanvasView/FileReaderNodeDialog.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

namespace {
  enum {
    NAME_COLUMN,
    TYPE_COLUMN,
  };

  static const int COLUMN_COUNT = 2;
}

FileReaderNodeStructureTable::FileReaderNodeStructureTable(QWidget* dialog)
    : QTableWidget(dialog),
      m_dialog(static_cast<FileReaderNodeDialog*>(dialog)),
      m_typeComboBox(nullptr) {}

QComboBox* FileReaderNodeStructureTable::GetCanvasTypeComboBox() {
  return m_typeComboBox;
}

void FileReaderNodeStructureTable::OpenTypeEditor(
    const QTableWidgetItem& item) {
  assert(m_typeComboBox == nullptr && item.column() == TYPE_COLUMN);
  m_typeComboBox = new QComboBox();
  auto types =
    m_dialog->m_userProfile->GetCanvasTypeRegistry().GetNativeTypes();
  for(auto i = types.begin(); i != types.end(); ++i) {
    m_typeComboBox->addItem(QString::fromStdString(i->GetName()));
    auto compatibility = i->GetCompatibility(
      *m_dialog->m_fields[item.row()].m_type);
    if(compatibility == CanvasType::Compatibility::EQUAL) {
      m_typeComboBox->setCurrentIndex(distance(types.begin(), i));
    }
  }
  setCellWidget(item.row(), item.column(), m_typeComboBox);
  m_typeComboBox->setFocus();
}

void FileReaderNodeStructureTable::CommitTypeEditor(QTableWidgetItem* item) {
  auto index = m_typeComboBox->currentIndex();
  std::shared_ptr<NativeType> type =
    m_dialog->m_userProfile->GetCanvasTypeRegistry().GetNativeTypes()[index];
  m_dialog->m_fields[item->row()].m_type = type;
  item->setText(QString::fromStdString(type->GetName()));
  setCellWidget(item->row(), item->column(), nullptr);
  m_typeComboBox = nullptr;
  setFocus();
}

void FileReaderNodeStructureTable::keyPressEvent(QKeyEvent* event) {
  auto item = currentItem();
  if(item == nullptr) {
    return QTableWidget::keyPressEvent(event);
  }
  auto key = event->key();
  if(key != Qt::Key_Tab && key != Qt::Key_Backtab && item != nullptr &&
      item->column() == TYPE_COLUMN && m_typeComboBox == nullptr &&
      !event->text().isEmpty()) {
    if(item->row() < static_cast<int>(m_dialog->m_fields.size())) {
      OpenTypeEditor(*item);
      QApplication::sendEvent(m_typeComboBox, event);
    }
    return;
  }
  if(m_typeComboBox != nullptr) {
    if(key == Qt::Key_Enter || key == Qt::Key_Return) {
      CommitTypeEditor(item);
    } else if(key == Qt::Key_Escape) {
      setCellWidget(item->row(), item->column(), nullptr);
      m_typeComboBox = nullptr;
      setFocus();
    }
    return;
  }
  if(key == Qt::Key_Delete) {
    m_dialog->DeleteItem(item);
    return;
  }
  return QTableWidget::keyPressEvent(event);
}

void FileReaderNodeStructureTable::dropEvent(QDropEvent* event) {
  auto dragIndex = currentItem()->row();
  auto dropIndex = indexAt(event->pos()).row();
  if(dragIndex == dropIndex || dropIndex == -1) {
    return;
  }
  if(dropIndex == 0) {
    dropIndex = 1;
  } else if(dropIndex == rowCount() - 1) {
    dropIndex = rowCount() - 2;
  }
  auto& fields = m_dialog->m_fields;
  auto draggedField = fields[dragIndex];
  fields.erase(fields.begin() + dragIndex);
  fields.erase(fields.begin() + dragIndex);
  fields.insert(fields.begin() + dropIndex, draggedField);
  reset();
  for(auto i = fields.begin(); i != fields.end(); ++i) {
    m_dialog->AddField(*i, distance(fields.begin(), i));
  }
  event->accept();
}
