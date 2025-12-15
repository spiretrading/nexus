#include "Spire/CanvasView/StandardCanvasTableItemDelegate.hpp"
#include <QPainter>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/CanvasView/CanvasTableWidget.hpp"
#include "Spire/CanvasView/LabelCell.hpp"
#include "Spire/CanvasView/LinkCell.hpp"
#include "Spire/CanvasView/ValueCell.hpp"

using namespace Beam;
using namespace Spire;

namespace {
  QColor validBorderColor = QColor::fromRgb(0, 102, 204);
  QColor errorBorderColor = QColor::fromRgb(194, 54, 54);
}

StandardCanvasTableItemDelegate::StandardCanvasTableItemDelegate(
    Ref<CanvasTableWidget> parent)
    : m_parent(parent.get()) {}

void StandardCanvasTableItemDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto cell = static_cast<CanvasCell*>(
    m_parent->item(index.row(), index.column()));
  auto leftCell = static_cast<CanvasCell*>(
    m_parent->item(index.row(), index.column() - 1));
  auto topCell = static_cast<CanvasCell*>(
    m_parent->item(index.row() - 1, index.column()));
  if(cell == nullptr) {
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();
    auto myOption = option;
    auto& border = myOption.rect;
    QPen pen;
    pen.setColor(QColor::fromRgb(208, 215, 229));
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    if(dynamic_cast<ValueCell*>(leftCell) == nullptr) {
      painter->drawLine(border.left(), border.top(), border.left(),
        border.bottom());
    }
    if(dynamic_cast<ValueCell*>(topCell) == nullptr) {
      painter->drawLine(border.left(), border.top(), border.right(),
        border.top());
    }
    painter->restore();
  } else if(dynamic_cast<LinkCell*>(cell) != nullptr) {
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();
    auto linkCell = static_cast<LinkCell*>(cell);
    auto myOption = option;
    QPen pen;
    pen.setColor(QColor::fromRgb(0, 0, 0));
    pen.setWidth(2);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    auto& border = myOption.rect;
    if(linkCell->GetOrientation() == Qt::Vertical) {
      painter->drawLine(border.left() + 1, border.top() + 1, border.left() + 1,
        border.bottom());
    } else {
      painter->drawLine(border.left() + 1, border.top() + 1, border.right(),
        border.top() + 1);
    }
    pen.setColor(QColor::fromRgb(208, 215, 229));
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    if(linkCell->GetOrientation() == Qt::Vertical &&
        dynamic_cast<ValueCell*>(topCell) == nullptr) {
      painter->drawLine(border.left() + 2, border.top(), border.right(),
        border.top());
    } else if(linkCell->GetOrientation() == Qt::Horizontal &&
        dynamic_cast<ValueCell*>(leftCell) == nullptr) {
      painter->drawLine(border.left(), border.top() + 2, border.left(),
        border.bottom());
    }
    painter->restore();
  } else if(dynamic_cast<ValueCell*>(cell) != nullptr) {
    auto valueCell = static_cast<ValueCell*>(cell);
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();
    auto myOption = option;
    QPen pen;
    if(valueCell->GetErrors().empty()) {
      pen.setColor(validBorderColor);
    } else {
      pen.setColor(errorBorderColor);
    }
    pen.setWidth(2);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    auto border = myOption.rect;
    border.setTop(border.top() + 1);
    border.setLeft(border.left() + 1);
    border.setWidth(border.width() - 1);
    border.setHeight(border.height() - 1);
    painter->drawRect(border);
    painter->restore();
  } else if(dynamic_cast<LabelCell*>(cell) != nullptr) {
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();
    auto& border = option.rect;
    auto offset = 0;
    if(dynamic_cast<LinkCell*>(topCell) != nullptr) {
      auto topLink = static_cast<LinkCell*>(topCell);
      if(&topLink->GetChild() == &cell->GetNode()) {
        auto myOption = option;
        QPen pen;
        pen.setColor(QColor::fromRgb(0, 0, 0));
        pen.setWidth(2);
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);
        painter->drawLine(border.left() + 1, border.top() + 1,
          border.left() + 1, border.bottom());
        offset = 2;
      }
    }
    QPen pen;
    pen.setColor(QColor::fromRgb(208, 215, 229));
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    if(dynamic_cast<ValueCell*>(topCell) == nullptr) {
      painter->drawLine(border.left() + offset, border.top(), border.right(),
        border.top());
    }
    if(offset == 0 && dynamic_cast<ValueCell*>(leftCell) == nullptr) {
      painter->drawLine(border.left(), border.top(), border.left(),
        border.bottom());
    }
    painter->restore();
  } else {
    QStyledItemDelegate::paint(painter, option, index);
  }
}
