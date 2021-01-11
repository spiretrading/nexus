#include "Spire/CanvasView/CustomNodeChildrenTable.hpp"
#include <QApplication>
#include <QComboBox>
#include <QKeyEvent>
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"
#include "Spire/CanvasView/CustomNodeDialog.hpp"
#include "Spire/UI/UserProfile.hpp"

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

CustomNodeChildrenTable::CustomNodeChildrenTable(QWidget* dialog)
    : QTableWidget(dialog),
      m_dialog(static_cast<CustomNodeDialog*>(dialog)),
      m_typeComboBox(nullptr) {}

QComboBox* CustomNodeChildrenTable::GetCanvasTypeComboBox() {
  return m_typeComboBox;
}

void CustomNodeChildrenTable::OpenTypeEditor(const QTableWidgetItem& item) {
  assert(m_typeComboBox == nullptr && item.column() == TYPE_COLUMN);
  m_typeComboBox = new QComboBox();
  const auto& types =
    m_dialog->m_userProfile->GetCanvasTypeRegistry().GetTypes();
  for(auto i = types.begin(); i != types.end(); ++i) {
    const auto& type = *i;
    m_typeComboBox->addItem(QString::fromStdString(type.GetName()));
    auto compatibility = type.GetCompatibility(
      *m_dialog->m_children[item.row()].m_type);
    if(compatibility == CanvasType::Compatibility::EQUAL) {
      m_typeComboBox->setCurrentIndex(distance(types.begin(), i));
    }
  }
  setCellWidget(item.row(), item.column(), m_typeComboBox);
  m_typeComboBox->setFocus();
}

void CustomNodeChildrenTable::CommitTypeEditor(QTableWidgetItem* item) {
  auto index = m_typeComboBox->currentIndex();
  const auto& type =
    m_dialog->m_userProfile->GetCanvasTypeRegistry().GetTypes()[index];
  auto& child = m_dialog->m_nodes[item->row()];
  if(!IsCompatible(type, child->GetType())) {
    child = MakeDefaultCanvasNode(type);
    m_dialog->m_children[item->row()].m_type = type;
  }
  item->setText(QString::fromStdString(type.GetName()));
  setCellWidget(item->row(), item->column(), nullptr);
  m_typeComboBox = nullptr;
  setFocus();
}

void CustomNodeChildrenTable::keyPressEvent(QKeyEvent* event) {
  auto item = currentItem();
  if(item == nullptr) {
    return QTableWidget::keyPressEvent(event);
  }
  auto key = event->key();
  if(key != Qt::Key_Tab && key != Qt::Key_Backtab && item != nullptr &&
      item->column() == TYPE_COLUMN && m_typeComboBox == nullptr &&
      !event->text().isEmpty()) {
    if(item->row() < static_cast<int>(m_dialog->m_children.size())) {
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

void CustomNodeChildrenTable::dropEvent(QDropEvent* event) {
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
  auto& nodes = m_dialog->m_nodes;
  auto& children = m_dialog->m_children;
  auto draggedNode = CanvasNode::Clone(*nodes[dragIndex]);
  auto draggedChild = children[dragIndex];
  nodes.erase(nodes.begin() + dragIndex);
  children.erase(children.begin() + dragIndex);
  nodes.insert(nodes.begin() + dropIndex, std::move(draggedNode));
  children.insert(children.begin() + dropIndex, draggedChild);
  reset();
  for(auto i = children.begin(); i != children.end(); ++i) {
    m_dialog->AddItem(*i, distance(children.begin(), i));
  }
  event->accept();
}
