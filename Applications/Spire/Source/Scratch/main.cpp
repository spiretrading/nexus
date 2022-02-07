#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/TabView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto view = new TabView();
  view->add("A", *make_label("A"));
  view->add("Positions", *make_label("Positions"));
  view->add("Order Log", *make_label("Order Log"));
  view->add("Executions", *make_label("Executions"));
  view->add("Profit/Loss", *make_label("Profit/Loss"));
  view->show();
  application->exec();
}
