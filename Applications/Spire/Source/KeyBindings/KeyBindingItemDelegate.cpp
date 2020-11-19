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
    m_editor_state(EditorState::ACCEPTED),
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

void KeyBindingItemDelegate::setEditorData(QWidget* editor,
  const QModelIndex& index) const {}

void KeyBindingItemDelegate::updateEditorGeometry(QWidget* editor,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  if(index.row() == 0) {
    auto rect = option.rect.translated(-1, 0);
    editor->move(rect.topLeft());
    editor->resize(rect.width() + 2, rect.height() + 1);
  } else {
    auto rect = option.rect.translated(-1, -1);
    editor->move(rect.topLeft());
    editor->resize({rect.width() + 2, rect.height() + 2});
  }
  auto table = reinterpret_cast<QTableView*>(parent());
  if(table->horizontalHeader()->visualIndex(index.column()) == 0) {
    editor->move(editor->pos().x() + 1, editor->pos().y());
    editor->resize(editor->width() - 1, editor->height());
  }
}

KeyBindingItemDelegate::EditorState
    KeyBindingItemDelegate::get_editor_state() const {
  return m_editor_state;
}

void KeyBindingItemDelegate::on_editing_finished() {
  m_editor_state = EditorState::ACCEPTED;
  auto editor = static_cast<QWidget*>(sender());
  editor->close();
}

bool KeyBindingItemDelegate::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    switch(e->key()) {
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
        m_editor_state = EditorState::ACCEPTED;
        {
          auto editor = static_cast<QWidget*>(watched);
          if(e->key() == Qt::Key_Tab) {
            Q_EMIT closeEditor(editor, QAbstractItemDelegate::EditNextItem);
          } else {
            Q_EMIT closeEditor(editor, QAbstractItemDelegate::EditPreviousItem);
          }
        }
        return true;
      case Qt::Key_Enter:
      case Qt::Key_Return:
        return false;
      case Qt::Key_Delete:
        m_editor_state = EditorState::DELETED;
        {
          auto editor = static_cast<QWidget*>(watched);
          Q_EMIT commitData(editor);
          Q_EMIT closeEditor(editor);
        }
        return true;
      case Qt::Key_Escape:
        m_editor_state = EditorState::CANCELLED;  
        {
          auto editor = static_cast<QWidget*>(watched);
          Q_EMIT closeEditor(editor);
        }
        return true;
    }
  }
  return QStyledItemDelegate::eventFilter(watched, event);
}
