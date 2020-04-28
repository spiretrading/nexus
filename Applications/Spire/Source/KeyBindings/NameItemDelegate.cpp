#include "Spire/KeyBindings/NameItemDelegate.hpp"
#include <QFontMetrics>
#include <QPainter>
#include "Spire/KeyBindings/NameInputEditor.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

NameItemDelegate::NameItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* NameItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto str = index.data().value<QString>();
  auto editor = new NameInputEditor(parent);
  connect(editor, &NameInputEditor::editingFinished,
    this, &NameItemDelegate::on_editing_finished);
  return editor;
}

void NameItemDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  painter->save();
  painter->fillRect(option.rect,
    index.data(Qt::BackgroundRole).value<QColor>());
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  painter->setFont(font);
  auto pos = QPoint(option.rect.left() + scale_width(8),
    option.rect.bottom() - scale_height(7));
  auto metrics = QFontMetrics(font);
  auto shortened_text = metrics.elidedText(index.data().toString(),
    Qt::ElideRight, option.rect.width() - scale_width(8));
  painter->drawText(pos, shortened_text);
  painter->restore();
}

void NameItemDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const {
  auto line_edit = static_cast<NameInputEditor*>(editor);
  line_edit->setText(index.data().value<QString>());
  line_edit->setCursorPosition(line_edit->text().length());
}

void NameItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto text = static_cast<QLineEdit*>(editor)->text().trimmed();
  model->setData(index, text, Qt::DisplayRole);
  m_item_modified_signal(index);
}
