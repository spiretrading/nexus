#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include "spire/spire/resources.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = new QWidget();
  window->resize(300, 500);
  auto layout = new QHBoxLayout(window);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto w1 = new QWidget(window);
  auto l1 = new QHBoxLayout();
  l1->setContentsMargins({});
  l1->setSpacing(0);
  w1->setFixedSize(150, 20);
  w1->setStyleSheet("background-color: red;");
  l1->addWidget(w1);
  layout->addLayout(l1);
  auto l2 = new QHBoxLayout();
  l2->setContentsMargins({});
  l2->setSpacing(0);
  auto w2 = new QWidget(window);
  w2->setFixedSize(150, 20);
  w2->setStyleSheet("background-color: yellow;");
  l2->addWidget(w2);
  layout->addLayout(l2);
  window->show();
  application->exec();
}
