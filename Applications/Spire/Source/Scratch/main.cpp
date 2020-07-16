#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QLabel>
#include <QWidget>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/FilteredDropDownMenu.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto test_window = new QWidget();
  auto layout = new QHBoxLayout(test_window);
  auto label = new QLabel("Null", test_window);
  label->setFont(QFont("Roboto", 14));
  label->move(scale_width(10), scale_height(10));
  auto dropdown1 = new StaticDropDownMenu({"One", "Two", "Three", "Four",
    "Five", "Six", "Seven"}, test_window);
  dropdown1->setFixedSize(scale(100, 28));
  dropdown1->connect_selected_signal([&] (const auto& value) {
    label->setText(value.value<QString>());
  });
  layout->addWidget(dropdown1);
  auto c = new ColorSelectorButton(Qt::red, test_window);
  c->setFixedSize(scale(100, 28));
  c->connect_color_signal([&] (const auto& color) {
    label->setText(color.name());
  });
  layout->addWidget(c);
  auto f = new FilteredDropDownMenu({"ABC", "ACD", "BCD", "BDE", "CDE", "CEF",
    "DEF", "DFG"}, test_window);
  f->setFixedSize(scale(100, 28));
  f->connect_selected_signal([=] (const auto& item) {
    label->setText(item.toString());
  });
  layout->addWidget(f);
  auto dropdown2 = new StaticDropDownMenu({"One", "Two", "Three", "Four",
    "Five", "Six", "Seven"}, "Numbers", test_window);
  dropdown2->setFixedSize(scale(100, 28));
  dropdown2->connect_selected_signal([&] (const auto& value) {
    label->setText(value.value<QString>());
  });
  layout->addWidget(dropdown2);
  auto t = new TextInputWidget(test_window);
  t->setFixedSize(scale(100, 28));
  t->connect(t, &QLineEdit::editingFinished, [=] {
    label->setText(t->text());
  });
  layout->addWidget(t);
  auto l1 = new QLabel("Static Menu", test_window);
  l1->move(scale_width(50), scale_height(30));
  auto l2 = new QLabel("Color Selector", test_window);
  l2->move(scale_width(200), scale_height(30));
  auto l3 = new QLabel("Filtered Menu", test_window);
  l3->move(scale_width(350), scale_height(30));
  auto l4 = new QLabel("Static w Fixed Label", test_window);
  l4->move(scale_width(500), scale_height(30));
  auto l6 = new QLabel("Text Input Widget", test_window);
  l6->move(scale_width(650), scale_height(30));
  test_window->setFixedSize(scale(800, 150));
  test_window->show();
  dropdown1->setFocus();
  application->exec();
}
