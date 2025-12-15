#include "Spire/CanvasView/ValueCell.hpp"
#include <QColor>
#include "Spire/Canvas/Common/CanvasNode.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

namespace {
  QColor readOnlyTextColor = QColor::fromRgb(77, 77, 77);
  QColor readOnlyBackgroundColor = QColor::fromRgb(212, 208, 200);
}

ValueCell::ValueCell(Ref<const CanvasNode> node)
    : m_node(node.get()) {
  setFlags((flags() | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
}

const vector<CanvasNodeValidationError>& ValueCell::GetErrors() const {
  return m_errors;
}

void ValueCell::AddError(const CanvasNodeValidationError& error) {
  m_errors.push_back(error);
}

void ValueCell::ResetErrors() {
  m_errors.clear();
}

const CanvasNode& ValueCell::GetNode() const {
  return *m_node;
}

QTableWidgetItem* ValueCell::clone() const {
  return new ValueCell(*this);
}

QVariant ValueCell::data(int role) const {
  if(role == Qt::DisplayRole || role == Qt::ToolTipRole) {
    return QString::fromStdString(m_node->GetText());
  } else if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::ForegroundRole) {
    if(m_node->IsReadOnly()) {
      return readOnlyTextColor;
    }
  } else if(role == Qt::BackgroundRole) {
    if(m_node->IsReadOnly()) {
      return readOnlyBackgroundColor;
    }
  }
  return QTableWidgetItem::data(role);
}

ValueCell::ValueCell(const ValueCell& cell)
    : m_node(cell.m_node),
      m_errors(cell.m_errors) {}
