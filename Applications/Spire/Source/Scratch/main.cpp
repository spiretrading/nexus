#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include <QTime>

using namespace Spire;

#include <QLabel>
#include <QBoxLayout>
#include <QWidget>
#include <QPushButton>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/TimeInputWidget.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;

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
  auto label = new QLabel("12:00 AM", window);
  layout->addWidget(input);
  layout->addWidget(label);
  layout->addWidget(button);
  auto button2 = new QPushButton("Set time 4:56 PM", window);
  layout->addWidget(button2);
  input->connect_time_signal([=] (auto duration) {
    auto d = static_cast<boost::posix_time::time_duration>(duration);
    auto str = [&] {
      if(d.hours() < 12) {
        auto hour = d.hours();
        if(hour == 0) {
          hour = 12;
        }
        auto hour_text = QString("%1%2").arg(hour < 10 ? "0" : "").arg(hour);
        auto minute_text = QString("%1%2").arg(d.minutes() < 10 ? "0" : "").arg(d.minutes());
        return QString("%1:%2 %3").arg(hour_text).arg(minute_text).arg("AM");
      }
      auto hour = d.hours();
      if(hour > 12) {
        hour -= 12;
      }
      auto hour_text = QString("%1%2").arg(hour < 10 ? "0" : "").arg(hour);
      auto minute_text = QString("%1%2").arg(d.minutes() < 10 ? "0" : "").arg(d.minutes());
      return QString("%1:%2 %3").arg(hour_text).arg(minute_text).arg("PM");
    }();
    label->setText(str);
  });
  QObject::connect(button, &QPushButton::clicked, [=] {
    input->set_time(Scalar(minutes(34)));
  });
  QObject::connect(button2, &QPushButton::clicked, [=] {
    input->set_time(Scalar(hours(16) + minutes(56)));
  });
  window->resize(600, 300);
  window->show();
  application->exec();
}
