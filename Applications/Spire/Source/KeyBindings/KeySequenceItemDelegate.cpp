#include "Spire/KeyBindings/KeySequenceItemDelegate.hpp"
#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto TEXT_PADDING() {
    static auto padding = scale_width(5);
    return padding;
  }

  auto get_key_text(Qt::Key key) {
    if(key == Qt::Key_Shift) {
      return QObject::tr("Shift");
    }
    if(key == Qt::Key_Alt) {
      return QObject::tr("Alt");
    }
    if(key == Qt::Key_Control) {
      return QObject::tr("Ctrl");
    }
    return QKeySequence(key).toString();
  }
}

KeySequenceItemDelegate::KeySequenceItemDelegate(
  const std::vector<KeySequenceEditor::ValidKeySequence>& valid_key_sequences,
  QWidget* parent)
  : QStyledItemDelegate(parent),
    m_valid_key_sequences(valid_key_sequences) {}

connection KeySequenceItemDelegate::connect_item_modified_signal(
    const ItemModifiedSignal::slot_type& slot) const {
  return m_item_modified_signal.connect(slot);
}

QWidget* KeySequenceItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new KeySequenceEditor(
    index.data(Qt::DisplayRole).value<QKeySequence>(), m_valid_key_sequences,
    parent);
  connect(editor, &KeySequenceEditor::editingFinished,
    this, &KeySequenceItemDelegate::on_editing_finished);
  return editor;
}

void KeySequenceItemDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  painter->save();
  painter->fillRect(option.rect, Qt::white);
  if(option.state.testFlag(QStyle::State_Editing)) {
    painter->setPen(QColor("#4B23A0"));
    painter->drawRect(option.rect);
  } else {
    auto sequence = index.data(Qt::DisplayRole).value<QKeySequence>();
    if(!sequence.isEmpty()) {
      draw_key_sequence(sequence, option.rect, painter);
    }
  }
  painter->restore();
}

void KeySequenceItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto key_sequence = reinterpret_cast<KeySequenceEditor*>(
    editor)->get_key_sequence();
  model->setData(index, key_sequence, Qt::DisplayRole);
  for(auto i = 0; i < model->rowCount(); ++i) {
    auto current_index = model->index(i, 1);
    if(current_index.row() != index.row() &&
        current_index.data().value<QKeySequence>() == key_sequence) {
      model->setData(current_index, QVariant());
      m_item_modified_signal(current_index);
      break;
    }
  }
  m_item_modified_signal(index);
}

QSize KeySequenceItemDelegate::sizeHint(const QStyleOptionViewItem& option,
    const QModelIndex& index) const {
  return QStyledItemDelegate::sizeHint(option, index);
}

bool KeySequenceItemDelegate::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(e->key() == Qt::Key_Escape) {
      
      return true;
    }
  }
  return QStyledItemDelegate::eventFilter(watched, event);
}

void KeySequenceItemDelegate::draw_key_sequence(const QKeySequence& sequence,
    const QRect& rect, QPainter* painter) const {
  auto pos = rect.bottomLeft();
  pos.setX(pos.x() + scale_width(8));
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  font.setWeight(55);
  painter->setFont(font);
  auto metrics = QFontMetrics(font);
  for(auto i = 0; i < sequence.count(); ++i) {
    auto text = get_key_text(static_cast<Qt::Key>(sequence[i]));
    auto text_size = QSize(metrics.horizontalAdvance(text), metrics.height());
    draw_key(text, text_size, pos, painter);
    pos.setX(pos.x() + TEXT_PADDING() * 2 + text_size.width() +
      scale_width(8));
  }
}

void KeySequenceItemDelegate::draw_key(const QString& text,
    const QSize& text_size, const QPoint& pos, QPainter* painter) const {
  auto path = QPainterPath();
  path.addRoundedRect(QRectF(pos.x(), pos.y() - scale_height(18) -
    scale_height(2) - 1, text_size.width() + TEXT_PADDING() * 2,
    scale_height(18)), scale_width(2), scale_height(2));
  painter->setPen({QColor("#E2C899"), static_cast<qreal>(scale_width(1))});
  painter->fillPath(path, QColor("#FFEDCD"));
  painter->drawPath(path);
  painter->setPen(Qt::black);
  painter->drawText(pos.x() + TEXT_PADDING(),
    pos.y() - (text_size.height() / 2), text);
}

void KeySequenceItemDelegate::on_editing_finished() {
  auto editor = reinterpret_cast<KeySequenceEditor*>(sender());
  emit commitData(editor);
  emit closeEditor(editor);
}
