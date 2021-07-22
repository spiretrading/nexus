#include <QApplication>
#include "Spire/Spire/Resources.hpp"

#include "Spire/Ui/TextAreaBox.hpp"
#include <QHBoxLayout>

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new QWidget();
  auto l = new QHBoxLayout(w);
  auto t = new TextAreaBox();
  l->addWidget(t);
  w->resize(600, 400);
  w->show();
  application->exec();
}
