#include "Spire/KeyBindings/NameItemDelegate.hpp"
#include <QFontMetrics>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

using namespace boost::signals2;
using namespace Spire;

NameItemDelegate::NameItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* NameItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new TextInputWidget(index.data().toString(),
    static_cast<QWidget*>(this->parent()));
  connect(editor, &TextInputWidget::editingFinished,
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
  auto metrics = QFontMetrics(font);
  auto shortened_text = metrics.elidedText(index.data().toString(),
    Qt::ElideRight, option.rect.width() - scale_width(8));
  auto pos = QPoint(option.rect.left() + scale_width(9),
    option.rect.bottom() + 1 - (option.rect.height() - metrics.ascent()) / 2);
  painter->drawText(pos, shortened_text);
  painter->restore();
}

void NameItemDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const {
  auto text_input = static_cast<TextInputWidget*>(editor);
  text_input->setText(index.data().value<QString>());
  text_input->setCursorPosition(text_input->text().length());
}

void NameItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto text = static_cast<QLineEdit*>(editor)->text().trimmed();
  model->setData(index, text, Qt::DisplayRole);
}
