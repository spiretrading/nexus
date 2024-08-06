#include <QApplication>
#include "Spire/LoginUiTester/LoginUiTester.hpp"
#include "Spire/Login/LoginWindow.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Window.hpp"
#include "Version.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Login UI Tester"));
  initialize_resources();
  auto tracks = std::vector<Track>();
  tracks.push_back(Track::CLASSIC);
  tracks.push_back(Track::CURRENT);
  tracks.push_back(Track::PREVIEW);
  auto track = std::make_shared<LocalTrackModel>(Track::CURRENT);
  auto window =
    LoginWindow(SPIRE_VERSION, tracks, track, {"Live Trading", "Development"});
  window.show();
  auto tester = LoginUiTester(&window);
  window.installEventFilter(&tester);
  tester.setGeometry(
    window.pos().x(), window.pos().y() + window.height() + 100, 0, 0);
  tester.setAttribute(Qt::WA_ShowWithoutActivating);
  tester.show();
  application.exec();
}
