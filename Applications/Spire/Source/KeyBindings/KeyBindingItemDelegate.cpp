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

connection KeyBindingItemDelegate::connect_key_signal(
    const KeySignal::slot_type& slot) const {
  return m_key_signal.connect(slot);
}

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

void KeyBindingItemDelegate::setEditorData(QWidget* editor,
  const QModelIndex& index) const {}

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
    if(e->key() == Qt::Key_Tab) {
      //m_key_signal(static_cast<Qt::Key>(e->key()));
      auto editor = static_cast<QWidget*>(watched);
      Q_EMIT commitData(editor);
      Q_EMIT closeEditor(editor, QAbstractItemDelegate::EditNextItem);
      return true;
    } else if(e->key() == Qt::Key_Backtab) {
      auto editor = static_cast<QWidget*>(watched);
      Q_EMIT commitData(editor);
      Q_EMIT closeEditor(editor, QAbstractItemDelegate::EditPreviousItem);
      return true;
    }
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
      return false;
    }
  }
  return QStyledItemDelegate::eventFilter(watched, event);
}
