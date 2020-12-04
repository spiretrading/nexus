#include "Spire/UiViewer/DropDownMenu2TestWidget.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropDownMenu2.hpp"

using namespace Spire;

namespace {
  auto create_item(const QString& label, QWidget* parent) {
    auto item = new DropDownItem(label, parent);
    item->setFixedHeight(scale_height(20));
    return item;
  }
}

DropDownMenu2TestWidget::DropDownMenu2TestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  auto layout = new QHBoxLayout(container);
  auto label = new QLabel("Parent Input", this);
  label->setFixedSize(scale(100, 26));
  label->setFocusPolicy(Qt::StrongFocus);
  label->setStyleSheet(QString(R"(
    QLabel {
      background-color: white;
      border: 1px solid #C8C8C8;
      font-family: Roboto;
      padding-left: %1px;
    }

    QLabel:hover, QLabel:focus {
      border: 1px solid #4B23A0;
    }})").arg(scale_width(5)));
  layout->addWidget(label);
  auto menu = new DropDownMenu2({
    create_item("AA", this), create_item("AB", this), create_item("AC", this),
    create_item("BA", this), create_item("BB", this), create_item("BC", this),
    create_item("CA", this), create_item("CB", this), create_item("CC", this)},
    label);
}
