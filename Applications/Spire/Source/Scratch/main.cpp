//#include <QApplication>
//#include "Spire/Spire/Resources.hpp"
//
//#include "Spire/Ui/TextAreaBox.hpp"
//#include <QHBoxLayout>
//
//using namespace Spire;
//
//int main(int argc, char** argv) {
//  auto application = new QApplication(argc, argv);
//  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
//  application->setApplicationName(QObject::tr("Scratch"));
//  initialize_resources();
//  auto w = new QWidget();
//  auto l = new QHBoxLayout(w);
//  auto t = new TextAreaBox();
//  l->addWidget(t);
//  w->resize(600, 400);
//  w->show();
//  application->exec();
//}
#include <QApplication>
#include "Spire/Spire/Resources.hpp"

#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Styles/StyleSheet.hpp"
#include <QLabel>
#include <QHBoxLayout>

using namespace Spire;
using namespace Spire::Styles;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new QWidget();
  auto layout = new QHBoxLayout(w);
  layout->setContentsMargins({});
  auto label = make_label("Test Label");
  label->setFixedSize(scale(300, 200));
  auto scroll_box = new ScrollBox(new Box(label));
  layout->addWidget(scroll_box);
  auto style = StyleSheet();
  style.get(Body()).
    set(horizontal_padding(scale_width(10))).
    set(vertical_padding(scale_height(10)));
  style.get(Body() / Body()).
    set(BackgroundColor(QColor::fromRgb(0xFF, 0x0, 0x0)));
  proxy_style(*w, *scroll_box);
  set_style(*w, style);
  w->resize(scale(200, 100));
  w->show();
  application->exec();
}
