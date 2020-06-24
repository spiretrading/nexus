#include <QApplication>
#include "Spire/Spire/Resources.hpp"

#include <QVBoxLayout>
#include <QLabel>

#include "Spire/Ui/FontSelectorWidget.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto a = new QWidget();
  a->resize(800, 600);
  auto w = new FontSelectorWidget(QFont("Roboto"), a);
  auto l = new QVBoxLayout(a);
  l->addWidget(w);
  auto d = new QLabel("Text", w);
  l->addWidget(d);
  w->connect_font_signal([=] (const auto& font) {
    d->setFont(font);
  });
  a->show();
  application->exec();
}
