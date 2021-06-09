#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QLineEdit>
#include <QHBoxLayout>

class MyStyle : QStyle {
  public:


};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new QWidget();
  auto l = new QHBoxLayout(w);
  auto le = new QLineEdit(w);
  le->setStyleSheet("background-color: white; color: black;");
  l->addWidget(le);
  w->resize(400, 400);
  w->show();
  application->exec();
}
