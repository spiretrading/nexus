#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QVBoxLayout>
#include "Spire/Ui/DateInputWidget.hpp"

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = new QWidget();
  window->setStyleSheet("background-color: white;");
  auto layout = new QVBoxLayout(window);
  auto date_input = new DateInputWidget(
    boost::posix_time::second_clock::local_time());
  date_input->setFixedSize(scale(112, 26));
  date_input->setStyleSheet("background-color: red;");
  layout->addWidget(date_input);
  window->resize(600, 400);
  window->show();
  application->exec();
}
