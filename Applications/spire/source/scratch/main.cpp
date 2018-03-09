#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QMovie>
#include "spire/spire/resources.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto label = new QLabel();
  label->setFixedSize(500, 500);
  auto logo = new QMovie(":/icons/animated-logo@2x.gif");
  logo->setScaledSize(label->size());
  qDebug() << "Is valid: " << logo->isValid();
  //logo->setFileName(QString("://icons/animated-logo.gif"));
  label->setMovie(logo);
  logo->start();
  label->show();
  application->exec();
}
