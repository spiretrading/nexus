#include "Spire/Blotter/BlotterTasksCommandBar.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;
using namespace Spire::Styles;

BlotterTasksCommandBar::BlotterTasksCommandBar(QWidget* parent)
    : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto content = new QWidget();
  make_hbox_layout(content);
  auto body = new Box(content);
  update_style(*body, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xC8C8C8))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  enclose(*this, *body);
}
