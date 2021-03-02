#include <QApplication>
#include <QHBoxLayout>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Styles/NotSelector.hpp"
#include "Spire/Styles/StyledWidget.hpp"

using namespace Spire;
using namespace Spire::Styles;

struct Box : StyledWidget {
  void paintEvent(QPaintEvent* event) override {
    auto block = compute_style();
    auto style = QString();
    if(auto background_color = Spire::Styles::find<BackgroundColor>(block)) {
      style = "background-color: " +
        background_color->get_expression().as<QColor>().name(QColor::HexArgb);
    }
    setStyleSheet(style);
    StyledWidget::paintEvent(event);
  }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto box = new Box();
  auto style = box->get_style();
  style.get(Any()).get_block().set(BackgroundColor(QColor::fromRgb(255, 0, 0)));
  style.get((Active() && !Hovered()) || (!Active() >> Hovered())).get_block().set(
    BackgroundColor(QColor::fromRgb(0, 255, 0)));
  box->set_style(style);
  auto container = new QWidget();
  auto l = new QHBoxLayout();
  l->addWidget(box);
  container->setLayout(l);
  container->show();
  application->exec();
}
