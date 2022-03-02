#include "Spire/Blotter/BlotterOrderLogView.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;
using namespace Spire::Styles;

BlotterOrderLogView::BlotterOrderLogView(QWidget* parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_box = new Box(nullptr, nullptr);
  enclose(*this, *m_box);
  proxy_style(*this, *m_box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0x00FFFF)));
  });
}
