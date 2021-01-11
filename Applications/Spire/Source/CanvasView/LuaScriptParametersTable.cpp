#include "Spire/CanvasView/LuaScriptParametersTable.hpp"
#include <QApplication>
#include <QComboBox>
#include <QKeyEvent>
#include "Spire/Canvas/LuaNodes/LuaScriptNode.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"
#include "Spire/CanvasView/LuaScriptDialog.hpp"
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

LuaScriptParametersTable::LuaScriptParametersTable(QWidget* dialog)
    : QTableWidget(dialog),
      m_dialog(static_cast<LuaScriptDialog*>(dialog)),
      m_typeComboBox(nullptr) {}

QComboBox* LuaScriptParametersTable::GetCanvasTypeComboBox() {
  return m_typeComboBox;
}

void LuaScriptParametersTable::OpenTypeEditor(const QTableWidgetItem& item) {
  assert(m_typeComboBox == nullptr && item.column() == TYPE_COLUMN);
  m_typeComboBox = new QComboBox();
  auto types = m_dialog->m_userProfile->GetCanvasTypeRegistry().GetTypes();
  for(auto i = types.begin(); i != types.end(); ++i) {
    m_typeComboBox->addItem(QString::fromStdString(i->GetName()));
    auto compatibility = i->GetCompatibility(
      *m_dialog->m_parameters[item.row()].m_type);
    if(compatibility == CanvasType::Compatibility::EQUAL) {
      m_typeComboBox->setCurrentIndex(distance(types.begin(), i));
    }
  }
  setCellWidget(item.row(), item.column(), m_typeComboBox);
  m_typeComboBox->setFocus();
}

void LuaScriptParametersTable::CommitTypeEditor(QTableWidgetItem* item) {
  auto index = m_typeComboBox->currentIndex();
  auto& type =
    m_dialog->m_userProfile->GetCanvasTypeRegistry().GetTypes()[index];
  auto& parameter = m_dialog->m_nodes[item->row()];
  if(!IsCompatible(type, parameter->GetType())) {
    parameter = MakeDefaultCanvasNode(type);
    m_dialog->m_parameters[item->row()].m_type = static_cast<NativeType&>(type);
  }
  item->setText(QString::fromStdString(type.GetName()));
  setCellWidget(item->row(), item->column(), nullptr);
  m_typeComboBox = nullptr;
  setFocus();
}

void LuaScriptParametersTable::keyPressEvent(QKeyEvent* event) {
  auto item = currentItem();
  if(item == nullptr) {
    return QTableWidget::keyPressEvent(event);
  }
  auto key = event->key();
  if(key != Qt::Key_Tab && key != Qt::Key_Backtab && item != nullptr &&
      item->column() == TYPE_COLUMN && m_typeComboBox == nullptr &&
      !event->text().isEmpty()) {
    if(item->row() < static_cast<int>(m_dialog->m_parameters.size())) {
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
    m_dialog->DeleteParameter(item);
    return;
  }
  return QTableWidget::keyPressEvent(event);
}

void LuaScriptParametersTable::dropEvent(QDropEvent* event) {
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
  auto& parameters = m_dialog->m_parameters;
  auto draggedNode = CanvasNode::Clone(*nodes[dragIndex]);
  auto draggedParameter = parameters[dragIndex];
  nodes.erase(nodes.begin() + dragIndex);
  parameters.erase(parameters.begin() + dragIndex);
  nodes.insert(nodes.begin() + dropIndex, std::move(draggedNode));
  parameters.insert(parameters.begin() + dropIndex, draggedParameter);
  reset();
  for(auto i = parameters.begin(); i != parameters.end(); ++i) {
    m_dialog->AddParameter(*i, distance(parameters.begin(), i));
  }
  event->accept();
}
