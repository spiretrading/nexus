#include "Spire/Blotter/BlotterTaskEntryItemDelegate.hpp"
#include <QPushButton>
#include "Spire/Blotter/BlotterTasksModel.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::LegacyUI;

BlotterTaskEntryItemDelegate::BlotterTaskEntryItemDelegate(
    Ref<UserProfile> userProfile)
    : CustomVariantItemDelegate(Ref(userProfile)) {}

BlotterTaskEntryItemDelegate::~BlotterTaskEntryItemDelegate() {}

QWidget* BlotterTaskEntryItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  if(index.column() != BlotterTasksModel::STICKY_COLUMN) {
    return QStyledItemDelegate::createEditor(parent, option, index);
  }
  QPushButton* button = new QPushButton(parent);
  button->setIcon(QIcon(":/icons/pin2_blue.png"));
  button->setIconSize(QSize(16, 12));
  button->setCheckable(true);
  button->setToolTip(QString("Sticks this task to the blotter."));
  connect(button, &QPushButton::toggled, this,
    &BlotterTaskEntryItemDelegate::OnStickyToggled);
  return button;
}

void BlotterTaskEntryItemDelegate::setEditorData(QWidget* editor,
    const QModelIndex& index) const {
  if(index.column() == BlotterTasksModel::STICKY_COLUMN) {
    const QAbstractItemModel* model = index.model();
    QPushButton* button = qobject_cast<QPushButton*>(editor);
    assert(model->data(index).canConvert<bool>());
    bool isSticky = model->data(index).value<bool>();
    button->setChecked(isSticky);
  }
}

void BlotterTaskEntryItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  if(index.column() == BlotterTasksModel::STICKY_COLUMN) {
    QPushButton* button = qobject_cast<QPushButton*>(editor);
    model->setData(index, button->isChecked());
  }
}

void BlotterTaskEntryItemDelegate::OnStickyToggled(bool checked) {
  QPushButton* button = qobject_cast<QPushButton*>(sender());
  Q_EMIT commitData(button);
}
