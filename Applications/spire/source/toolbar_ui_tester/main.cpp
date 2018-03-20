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
  recently_closed_model model;
  auto window = new toolbar_window(model);
  spire::window frame(window);
  frame.show();
  toolbar_ui_tester tester(window, model);
  tester.setGeometry(frame.pos().x(),
    frame.pos().y() + frame.height() + 100, 0, 0);
  frame.installEventFilter(window);
  frame.installEventFilter(&tester);
  tester.setAttribute(Qt::WA_ShowWithoutActivating);
  tester.show();
  application->exec();
}
