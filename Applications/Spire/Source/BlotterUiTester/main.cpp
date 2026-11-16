#include <QApplication>
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Blotter UI Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto model = std::make_shared<CompositeBlotterModel>(
    std::make_shared<LocalTextModel>(QObject::tr("New Blotter")));
  auto window = BlotterWindow(model);
  window.show();
  return application.exec();
}
