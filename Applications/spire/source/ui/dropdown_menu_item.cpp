#include "spire/ui/dropdown_menu_item.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Spire;

DropdownMenuItem::DropdownMenuItem(const QString& text, QWidget* parent)
    : QLabel(text, parent) {
  setFixedHeight(scale_height(20));
  setStyleSheet(QString(R"(
    QLabel {
      background-color: #FFFFFF;
      font-family: Roboto;
      font-size: %1pt;
      padding-left: %2px;
    }

    QLabel:hover {
      background-color: #F2F2FF;
    })").arg(scale_height(7)).arg(scale_width(5)));
}
