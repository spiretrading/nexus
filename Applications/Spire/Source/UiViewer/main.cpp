#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/UiViewer/UiViewerWindow.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("UI Viewer"));
  initialize_resources();
  auto window = UiViewerWindow();
  window.show();
  application.exec();
}
