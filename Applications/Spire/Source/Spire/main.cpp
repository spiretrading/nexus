#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/SpireController.hpp"
#include "Version.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("UI Viewer"));
  application->setApplicationVersion(SPIRE_VERSION);
  initialize_resources();
  auto controller = SpireController();
  controller.open();
  application->exec();
}
