#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QBoxLayout>
#include <QWidget>
#include <QPushButton>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/TimeInputWidget.hpp"

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = new QWidget();
  auto layout = new QVBoxLayout(window);
  auto input = new TimeInputWidget(window);
  input->setFixedSize(scale(112, 26));
  auto button = new QPushButton("Set time 12:34 AM", window);
  button->connect(button, &QPushButton::clicked, [=] {
    input->set_time(Scalar(2040000));
  });
  layout->addWidget(input);
  layout->addWidget(button);
  window->resize(600, 300);
  window->show();
  application->exec();
}
