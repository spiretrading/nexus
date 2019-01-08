#include <QApplication>
#include "spire/spire/resources.hpp"
#include "spire/spire/spire_controller.hpp"
#include "spire/version.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems"));
  application->setApplicationName(QObject::tr("Spire"));
  application->setApplicationVersion(SPIRE_VERSION);
  initialize_resources();
  auto controller = SpireController();
  controller.open();
  application->exec();
}
