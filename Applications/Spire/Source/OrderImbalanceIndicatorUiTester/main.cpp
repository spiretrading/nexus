#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(
    QObject::tr("Order Imbalance Indicator UI Tester"));
  initialize_resources();
  application->exec();
}