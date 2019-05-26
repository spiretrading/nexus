#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/SpireController.hpp"
#include "Version.hpp"

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
