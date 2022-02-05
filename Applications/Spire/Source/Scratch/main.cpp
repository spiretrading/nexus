#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/TabView.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto view = new TabView();
  view->show();
  application->exec();
}
