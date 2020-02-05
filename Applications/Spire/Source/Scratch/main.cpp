#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QBoxLayout>
#include <QWidget>
#include "Spire/Ui/TimeInputWidget.hpp"

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = new QWidget();
  auto layout = new QVBoxLayout(window);
  auto input = new TimeInputWidget(window);
  layout->addWidget(input);
  window->resize(600, 300);
  window->show();
  application->exec();
}
