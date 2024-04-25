#include <QApplication>
#include <QWidget>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;
using namespace Spire::Styles;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto parent = QWidget();
  auto box = Box();
  auto button = Button(&box);
  enclose(parent, button);
  update_style(parent, [] (auto& style) {
    style.get(Any() > is_a<Button>()).set(BackgroundColor(QColor(0xFF0000)));
  });
  parent.resize(640, 480);
  parent.show();
  application.exec();
}
