#include "Spire/Ui/SideInputWidget.hpp"
#include "Nexus/Definitions/Side.hpp"

using namespace Nexus;
using namespace Spire;

SideInputWidget::SideInputWidget(QWidget* parent)
  : FilteredDropDownMenu({QVariant::fromValue<Side>(Side::ASK),
      QVariant::fromValue<Side>(Side::BID)},
    parent) {}

Side SideInputWidget::get_side() const {
  return get_item().value<Side>();
}
