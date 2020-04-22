#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"
#include <QPainter>
#include "Spire/KeyBindings/InputFieldEditor.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

KeyBindingItemDelegate::KeyBindingItemDelegate(QWidget* parent)
  : QStyledItemDelegate(parent),
    m_item_delegate(new CustomVariantItemDelegate(this)),
    m_background_color(Qt::white) {}

void KeyBindingItemDelegate::set_background_color(const QColor& color) {
  m_background_color = color;
}

connection KeyBindingItemDelegate::connect_item_modified_signal(
    const ItemModifiedSignal::slot_type& slot) const {
  return m_item_modified_signal.connect(slot);
}

void KeyBindingItemDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  painter->save();
  painter->fillRect(option.rect, m_background_color);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  painter->setFont(font);
  painter->drawText(QPoint(scale_width(8), scale_height(6)),
    m_item_delegate->displayText(index.data(), QLocale()));
  painter->restore();
}

void KeyBindingItemDelegate::updateEditorGeometry(QWidget* editor,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  if(index.row() == 0) {
    auto rect = option.rect.translated(0, 1);
    editor->move(rect.topLeft());
    rect.setHeight(rect.height() - 1);
    editor->resize(rect.size());
  } else {
    editor->move(option.rect.topLeft());
    editor->resize(option.rect.size());
  }
}

void KeyBindingItemDelegate::on_editing_finished() {
  auto editor = static_cast<QWidget*>(sender());
  editor->close();
}
