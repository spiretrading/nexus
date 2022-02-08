#include "Spire/Ui/ScrollableListBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_default_style() {
    auto style = StyleSheet();
    style.get(Any()).
      set(border(scale_width(1), QColor(0xC8C8C8))).
      set(BackgroundColor(QColor(0xFFFFFF)));
    return style;
  }
}

ScrollableListBox::ScrollableListBox(ListView& list_view, QWidget* parent)
    : QWidget(parent),
      m_list_view(&list_view) {
  m_list_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusProxy(m_list_view);
  m_scroll_box = new ScrollBox(m_list_view);
  m_scroll_box->setFocusPolicy(Qt::NoFocus);
  proxy_style(*this, *m_scroll_box);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_scroll_box);
  set_style(*this, make_default_style());
  m_current_connection = m_list_view->get_current()->connect_update_signal(
    [=] (const auto& current) { on_current(current); });
  update_style(*m_list_view, [&] (auto& style) {
    style.get(Any()).set(EdgeNavigation::CONTAIN);
  });
}

ListView& ScrollableListBox::get_list_view() {
  return *m_list_view;
}

ScrollBox& ScrollableListBox::get_scroll_box() {
  return *m_scroll_box;
}

void ScrollableListBox::on_current(const optional<int>& current) {
  if(!current) {
    return;
  }
  m_scroll_box->scroll_to(*m_list_view->get_list_item(*current));
}
