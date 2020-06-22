#include <QApplication>
#include "Spire/Spire/Resources.hpp"

#include "Spire/Ui/FontSelectorWidget.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new FontSelectorWidget(QFont("Roboto"));
  w->show();
  application->exec();
}
