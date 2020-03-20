#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QWidget>
#include "Spire/Ui/DateInputWidget.hpp"
#include <QHBoxLayout>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Spire/Ui/DropDownMenu.hpp"
#include "Spire/Charting/DropDownColorPicker.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Toolbar/ToolbarMenu.hpp"
#include "Spire/Charting/TrendLineStyleDropDownMenu.hpp"

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new QWidget();
  auto layout = new QHBoxLayout(w);
  layout->setContentsMargins(scale_width(8), scale_height(8), scale_height(8),
    scale_height(200));
  auto drop_down = new DropDownMenu({"ABC", "DEF", "GHI"}, w);
  drop_down->setFixedSize(scale(80, 26));
  layout->addWidget(drop_down);
  auto color_picker = new DropDownColorPicker(w);
  color_picker->setFixedSize(scale(80, 26));
  layout->addWidget(color_picker);
  auto line = new TrendLineStyleDropDownMenu(w);
  line->setFixedSize(scale(80, 18));
  layout->addWidget(line);
  auto tool = new ToolbarMenu("ABC", w);
  tool->add("ABC");
  tool->add("DEF");
  tool->add("GHI");
  tool->setFixedSize(scale(80, 26));
  layout->addWidget(tool);
  auto date = new DateInputWidget(boost::posix_time::from_time_t(1000000), w);
  date->setFixedSize(scale(80, 26));
  layout->addWidget(date);
  w->resize(600, 400);
  w->show();
  application->exec();
}
