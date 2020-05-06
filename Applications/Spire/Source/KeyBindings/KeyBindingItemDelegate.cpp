#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"
#include <QKeyEvent>
#include <QPainter>
#include <QTableView>
#include "Spire/KeyBindings/InputFieldEditor.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

KeyBindingItemDelegate::KeyBindingItemDelegate(QWidget* parent)
  : QStyledItemDelegate(parent),
    m_item_delegate(new CustomVariantItemDelegate(this)) {}

void KeyBindingItemDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  painter->save();
  painter->fillRect(option.rect,
    index.data(Qt::BackgroundRole).value<QColor>());
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  painter->setFont(font);
  auto pos = QPoint(option.rect.left() + scale_width(8),
    option.rect.bottom() - scale_height(7));
  painter->drawText(pos,
    m_item_delegate->displayText(index.data(), QLocale()));
  painter->restore();
}

void KeyBindingItemDelegate::updateEditorGeometry(QWidget* editor,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  editor->move(option.rect.topLeft());
  editor->resize(option.rect.size());
  if(index.row() == 0) {
    auto rect = option.rect.translated(0, 1);
    editor->move(rect.topLeft());
    editor->resize(rect.width(), rect.height() - 1);
  }
  auto table = reinterpret_cast<QTableView*>(parent());
  if(table->horizontalHeader()->visualIndex(index.column()) == 0) {
    editor->move(editor->pos().x() + 1, editor->pos().y());
    editor->resize(editor->width() - 1, editor->height());
  }
}

void KeyBindingItemDelegate::on_editing_finished() {
  auto editor = static_cast<QWidget*>(sender());
  editor->close();
}

bool KeyBindingItemDelegate::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) {
      return true;
    }
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
      return false;
    }
  }
  return QStyledItemDelegate::eventFilter(watched, event);
}
