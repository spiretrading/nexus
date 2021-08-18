#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include "Spire/Ui/CalendarDatePicker.hpp"

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto  w = new QWidget();
  auto c = new CalendarDatePicker(w);
  w->resize(800, 800);
  w->show();
  application->exec();
}
