#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/ScrollableLayer.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto layer = new ScrollableLayer();
  layer->resize(300, 300);
  layer->show();
  application->exec();
}
