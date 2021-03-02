#include <QApplication>
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
  auto widget = new Box();
  auto style = widget->get_style();
  style.get(Any()).get_block().set(BackgroundColor(QColor::fromRgb(255, 0, 0)));
  style.get(Hovered()).get_block().set(
    BackgroundColor(QColor::fromRgb(0, 255, 0)));
  style.get(!Active()).get_block().set(
    BackgroundColor(QColor::fromRgb(100, 100, 100)));
  style.get(Hovered() && !Active()).get_block().set(
    BackgroundColor(QColor::fromRgb(0, 0, 255)));
  widget->set_style(style);
  widget->show();
  application->exec();
}
