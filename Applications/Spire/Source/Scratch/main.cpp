#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QVBoxLayout>
#include "Spire/Ui/CalendarWidget.hpp"

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = new QWidget();
  auto layout = new QVBoxLayout(window);
  auto calendar = new CalendarWidget();
  layout->addWidget(calendar);
  window->resize(600, 400);
  window->show();
  application->exec();
}
