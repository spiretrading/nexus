#include <QApplication>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
#include "spire/spire/resources.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new QWidget();
  w->setBaseSize(640, 480);
  auto l = new QVBoxLayout(w);
  l->setMargin(50);
  auto *effect = new QGraphicsDropShadowEffect();
  effect->setBlurRadius(50);
  effect->setXOffset(0);
  effect->setYOffset(0);
  effect->setColor(QColor(0, 0, 0, 70));
  auto w2 = new QWidget();
  w2->setGraphicsEffect(effect);
  w2->setStyleSheet("background-color: #F5F5F5; border: 1px solid #8C8C8C;");
  w->setStyleSheet("background-color: #FFFFFF;");
  l->addWidget(w2);
  w->show();
  application->exec();
}
