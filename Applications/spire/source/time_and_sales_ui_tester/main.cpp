#include <QApplication>
#include "spire/time_and_sales/time_and_sales_window.hpp"
#include "spire/spire/resources.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Time and Sales UI Tester"));
  initialize_resources();
  time_and_sales_window window;
  window.show();
  application->exec();
}
