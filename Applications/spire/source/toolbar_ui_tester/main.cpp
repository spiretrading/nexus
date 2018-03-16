#include <QApplication>
#include "spire/spire/resources.hpp"
#include "spire/toolbar/recently_closed_model.hpp"
#include "spire/toolbar/toolbar_window.hpp"
#include "spire/toolbar_ui_tester/toolbar_ui_tester.hpp"
#include "spire/ui/window.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Toolbar UI Tester"));
  initialize_resources();
  auto window = new toolbar_window();
  spire::window frame(window);
  frame.show();
  recently_closed_model model;
  toolbar_ui_tester tester(&model);
  tester.setGeometry(frame.pos().x(),
    frame.pos().y() + frame.height() + 100, 0, 0);
  frame.installEventFilter(&tester);
  tester.setAttribute(Qt::WA_ShowWithoutActivating);
  tester.show();
  application->exec();
}
