#include "Spire/CanvasView/LinkCell.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

LinkCell::LinkCell(Qt::Orientation orientation,
    Ref<const CanvasNode> parent, Ref<const CanvasNode> child)
    : m_orientation(orientation),
      m_parent(parent.get()),
      m_child(child.get()) {
  setFlags(flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable &
    ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
}

Qt::Orientation LinkCell::GetOrientation() const {
  return m_orientation;
}

const CanvasNode& LinkCell::GetChild() const {
  return *m_child;
}

const CanvasNode& LinkCell::GetNode() const {
  return *m_parent;
}

QTableWidgetItem* LinkCell::clone() const {
  return new LinkCell(m_orientation, Ref(*m_parent), Ref(*m_child));
}

QVariant LinkCell::data(int role) const {
  return QTableWidgetItem::data(role);
}
