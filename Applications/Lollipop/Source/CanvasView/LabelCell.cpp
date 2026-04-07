#include "Spire/CanvasView/LabelCell.hpp"
#include <boost/algorithm/string.hpp>

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

LabelCell::LabelCell(const string& name, Ref<const CanvasNode> node)
    : m_name(name),
      m_node(node.get()) {
  replace_all(m_name, "\\.", ".");
  setFlags(flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable &
    ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
}

const CanvasNode& LabelCell::GetNode() const {
  return *m_node;
}

QTableWidgetItem* LabelCell::clone() const {
  return new LabelCell(m_name, Ref(*m_node));
}

QVariant LabelCell::data(int role) const {
  if(role == Qt::DisplayRole) {
    return QString::fromStdString(m_name);
  } else if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  }
  return QTableWidgetItem::data(role);
}
