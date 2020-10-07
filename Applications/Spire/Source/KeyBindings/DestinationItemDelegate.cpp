#include "Spire/KeyBindings/DestinationItemDelegate.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Spire/Ui/ComboBoxEditor.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

DestinationItemDelegate::DestinationItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* DestinationItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto menu = new StaticDropDownMenu({"One", "Two", "Three"},
    static_cast<QWidget*>(this->parent()));
  auto editor = new ComboBoxEditor(menu,
    static_cast<QWidget*>(this->parent()));
  editor->set_value(index.data());
  connect(editor, &ComboBoxEditor::editingFinished,
    this, &DestinationItemDelegate::on_editing_finished);
  return editor;
}

void DestinationItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto variant = [&] {
    auto combo_box = static_cast<ComboBoxEditor*>(editor);
    if(combo_box->get_last_key() == Qt::Key_Escape) {
      return index.data();
    } else if(combo_box->get_last_key() == Qt::Key_Delete) {
      return QVariant();
    }
    auto region = Region(
      combo_box->get_value().value<QString>().toStdString());
    return QVariant::fromValue(region);
  }();
  model->setData(index, variant, Qt::DisplayRole);
}
