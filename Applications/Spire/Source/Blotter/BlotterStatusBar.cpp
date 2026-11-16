#include "Spire/Blotter/BlotterStatusBar.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;
using namespace Spire::Styles;

BlotterStatusBar::BlotterStatusBar(QWidget* parent)
    : QWidget(parent) {
  setFixedHeight(scale_height(26));
  auto body = new Box();
  update_style(*body, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderTopSize(scale_height(1))).
      set(BorderTopColor(QColor(0xC8C8C8)));
  });
  enclose(*this, *body);
}
