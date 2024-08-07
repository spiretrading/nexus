#include <QApplication>
#include "Spire/SignInUiTester/SignInUiTester.hpp"
#include "Spire/SignIn/SignInWindow.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Window.hpp"
#include "Version.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("SignIn UI Tester"));
  initialize_resources();
  auto tracks = std::vector<Track>();
  tracks.push_back(Track::CURRENT);
  tracks.push_back(Track::PREVIEW);
  tracks.push_back(Track::CLASSIC);
  auto track = std::make_shared<LocalTrackModel>(Track::CURRENT);
  auto window =
    SignInWindow(SPIRE_VERSION, tracks, track, {"Live Trading", "Development"});
  window.show();
  auto tester = SignInUiTester(window);
  window.installEventFilter(&tester);
  tester.setGeometry(
    window.pos().x(), window.pos().y() + window.height() + 100, 0, 0);
  tester.setAttribute(Qt::WA_ShowWithoutActivating);
  tester.show();
  application.exec();
}
