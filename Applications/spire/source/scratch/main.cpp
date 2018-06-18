#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QScrollArea>
#include <QTableWidget>
#include <QWidget>
#include "spire/spire/resources.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto table = new QTableWidget();
  table->setColumnCount(2);
  table->setRowCount(10);
  auto x = new QLabel("Label 1", table);
  x->setFixedWidth(300);
  x->setStyleSheet("background-color: red;");
  table->show();
  auto q = typeid(*(table->viewport())).name();
  table->connect(table, &QTableWidget::itemSelectionChanged,
    [=] { if(table->selectionModel()->selectedRows().size() > 0) {
            table->setIndexWidget(table->selectionModel()->selectedRows().first(), x); 
          }});
  application->exec();
}
