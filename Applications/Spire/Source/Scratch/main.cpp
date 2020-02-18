#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QHBoxLayout>
#include "Spire/Ui/DateInputWidget.hpp"

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = new QWidget();
  window->setStyleSheet("background-color: white;");
  auto layout = new QHBoxLayout(window);
  auto date_input1 = new DateInputWidget(
    boost::posix_time::second_clock::local_time());
  date_input1->setFixedSize(scale(112, 26));
  date_input1->setFocus();
  layout->addWidget(date_input1);
  auto date_input2 = new DateInputWidget(
    boost::posix_time::second_clock::local_time());
  date_input2->setFixedSize(scale(112, 26));
  date_input2->setFocus();
  layout->addWidget(date_input2);
  auto date_input3 = new DateInputWidget(
    boost::posix_time::second_clock::local_time());
  date_input3->setFixedSize(scale(112, 26));
  date_input3->setFocus();
  layout->addWidget(date_input3);
  window->resize(600, 400);
  window->show();
  application->exec();
}
