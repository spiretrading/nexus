#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
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
      m_list_view(&list_view),
      m_size_policy(m_list_view->sizePolicy()) {
  setFocusProxy(m_list_view);
  m_scroll_box = new ScrollBox(m_list_view);
  m_scroll_box->setFocusPolicy(Qt::NoFocus);
  proxy_style(*this, *m_scroll_box);
  enclose(*this, *m_scroll_box);
  m_list_view_style_connection =
    connect_style_signal(*m_list_view, [=] { on_list_view_style(); });
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

void ScrollableListBox::on_list_view_style() {
  auto direction = Qt::Orientation::Vertical;
  auto overflow = Overflow::NONE;
  auto& stylist = find_stylist(*m_list_view);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (EnumProperty<Qt::Orientation> direction_style) {
        stylist.evaluate(direction_style, [&] (auto d) {
          direction = d;
        });
      },
      [&] (EnumProperty<Overflow> overflow_style) {
        stylist.evaluate(overflow_style, [&] (auto o) {
          overflow = o;
        });
      });
  }
  auto horizontal_policy = [&] {
    if(direction == Qt::Orientation::Horizontal) {
      if(overflow == Overflow::NONE) {
        if(m_size_policy.horizontalPolicy() == QSizePolicy::Preferred) {
          return QSizePolicy::Minimum;
        }
        return m_size_policy.horizontalPolicy();
      }
      return QSizePolicy::Expanding;
    }
    return QSizePolicy::MinimumExpanding;
  }();
  auto vertical_policy = [&] {
    if(direction == Qt::Orientation::Vertical) {
      if(overflow == Overflow::NONE) {
        if(m_size_policy.verticalPolicy() == QSizePolicy::Preferred) {
          return QSizePolicy::Minimum;
        }
        return m_size_policy.verticalPolicy();
      }
      return QSizePolicy::Expanding;
    }
    return QSizePolicy::MinimumExpanding;
  }();
  if(m_list_view->sizePolicy().horizontalPolicy() != horizontal_policy ||
      m_list_view->sizePolicy().verticalPolicy() != vertical_policy) {
    m_list_view->setSizePolicy(horizontal_policy, vertical_policy);
    m_list_view->updateGeometry();
  }
}
