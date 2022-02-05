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
  auto label_1 = make_label("Hello");
  label_1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  view->add("Hello", *label_1);
  view->add("Goodbye", *make_label("Goodbye"));
  view->show();
  application->exec();
}
