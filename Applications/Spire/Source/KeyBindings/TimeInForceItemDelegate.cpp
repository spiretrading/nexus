#include "Spire/KeyBindings/TimeInForceItemDelegate.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/FilteredDropDownMenu.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto create_time_in_force_item_list() {
    static auto list = std::vector<QVariant>({
      displayText(TimeInForce(TimeInForce::Type::DAY)),
      displayText(TimeInForce(TimeInForce::Type::FOK)),
      displayText(TimeInForce(TimeInForce::Type::GTC)),
      displayText(TimeInForce(TimeInForce::Type::GTD)),
      displayText(TimeInForce(TimeInForce::Type::GTX)),
      displayText(TimeInForce(TimeInForce::Type::IOC)),
      displayText(TimeInForce(TimeInForce::Type::MOC)),
      displayText(TimeInForce(TimeInForce::Type::OPG))
    });
    return list;
  }

  auto get_time_in_force_variant(const QString& text) {
    for(auto type : { TimeInForce::Type::DAY, TimeInForce::Type::FOK,
        TimeInForce::Type::GTC, TimeInForce::Type::GTD, TimeInForce::Type::GTX,
        TimeInForce::Type::IOC, TimeInForce::Type::MOC,
        TimeInForce::Type::OPG }) {
      if(text == Spire::displayText(TimeInForce(type))) {
        return QVariant::fromValue<TimeInForce>({type});
      }
    }
    return QVariant();
  }
}

TimeInForceItemDelegate::TimeInForceItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* TimeInForceItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto current_data = [&] {
    auto data = index.data(Qt::DisplayRole);
    if(data.isValid()) {
      return Spire::displayText(data.value<TimeInForce>());
    }
    return QString();
  }();
  auto editor = new FilteredDropDownMenu(create_time_in_force_item_list(),
    static_cast<QWidget*>(this->parent()));
  editor->set_current_item(current_data);
  editor->set_style(FilteredDropDownMenu::Style::CELL);
  connect(editor, &FilteredDropDownMenu::editingFinished,
    this, &TimeInForceItemDelegate::on_editing_finished);
  return editor;
}

void TimeInForceItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<FilteredDropDownMenu*>(
    editor)->get_item_or_invalid().value<QString>().toUpper();
  model->setData(index, get_time_in_force_variant(item), Qt::DisplayRole);
}
