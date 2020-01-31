#include "Spire/Ui/InputWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

InputWidget::InputWidget(QWidget* parent)
    : QLineEdit(parent) {
  setStyleSheet(QString(R"(
    QLineEdit {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %3px;
      padding-left: %4px;
    }

    QLineEdit:focus {
      border: %1px solid #4B23A0 %2px solid #4B23A0;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(scale_height(12))
        .arg(scale_width(8)));
}
