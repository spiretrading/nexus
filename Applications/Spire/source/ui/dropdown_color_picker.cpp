#include "spire/ui/dropdown_color_picker.hpp"
#include <QHBoxLayout>
#include "spire/ui/flat_button.hpp"

using namespace Spire;

DropdownColorPicker::DropdownColorPicker(QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto button = new FlatButton(this);
  layout->addWidget(button);
  setFocusProxy(button);
}
