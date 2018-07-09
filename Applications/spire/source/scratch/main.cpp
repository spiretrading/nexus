#include <QApplication>
#include <QListWidget>
#include <QListWidgetItem>
#include "spire/spire/resources.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto list = new QListWidget();
  list->setFixedSize(400, 600);
  auto item1 = new QListWidgetItem("Item 1", list);
  item1->setBackground(Qt::red);
  auto item2 = new QListWidgetItem("Item 2", list);
  item2->setBackground(Qt::green);
  auto item3 = new QListWidgetItem("Item 3", list);
  item3->setBackground(Qt::blue);
  list->setStyleSheet(R"(
    QListWidget::item:selected {
      background-color: transparent;
      border: 1px solid black 1px solid black;
    })");
  list->show();
  application->exec();
}
