#include "Spire/Ui/ScrollableListBox.hpp"
#include <QResizeEvent>
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

  auto has_range(const ScrollBar::Range& range) {
    return range.m_end - range.m_start > 1;
  }
}

ScrollableListBox::ScrollableListBox(ListView& list_view, QWidget* parent)
    : QWidget(parent),
      m_list_view(&list_view),
      m_size_policy(m_list_view->sizePolicy()),
      m_is_showing(false) {
  m_list_view->installEventFilter(this);
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

bool ScrollableListBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_list_view && event->type() == QEvent::Resize) {
    auto gap = [&] {
      if(m_styles.m_direction == Qt::Orientation::Vertical) {
        return m_styles.m_item_gap;
      }
      return m_styles.m_overflow_gap;
    }();
    if(auto item = m_list_view->get_list_item(0)) {
      m_scroll_box->get_horizontal_scroll_bar().set_line_size(
        item->width() + gap);
      m_scroll_box->get_vertical_scroll_bar().set_line_size(
        item->height() + gap);
    }
  }
  return QWidget::eventFilter(watched, event);
}

void ScrollableListBox::keyPressEvent(QKeyEvent* event) {
  if(event->modifiers() & Qt::AltModifier) {
    auto& horizontal_scroll_bar = m_scroll_box->get_horizontal_scroll_bar();
    if(has_range(horizontal_scroll_bar.get_range())) {
      if(event->key() == Qt::Key_PageUp) {
        scroll_page_up(horizontal_scroll_bar);
        event->accept();
      } else if(event->key() == Qt::Key_PageDown) {
        scroll_page_down(horizontal_scroll_bar);
        event->accept();
      }
    }
  }
  QWidget::keyPressEvent(event);
}

void ScrollableListBox::resizeEvent(QResizeEvent* event) {
  if(m_is_showing) {
    m_is_showing = false;
    on_current(m_list_view->get_current()->get());
  }
  QWidget::resizeEvent(event);
}

void ScrollableListBox::showEvent(QShowEvent* event) {
  m_is_showing = true;
  QWidget::showEvent(event);
}

void ScrollableListBox::on_current(const optional<int>& current) {
  if(!current) {
    return;
  }
  if(auto item = m_list_view->get_list_item(*current)) {
    m_scroll_box->scroll_to(*item);
  }
}

void ScrollableListBox::on_list_view_style() {
  m_styles.m_direction = Qt::Orientation::Vertical;
  m_styles.m_item_gap = 0;
  m_styles.m_overflow_gap = 0;
  auto overflow = Overflow::NONE;
  auto& stylist = find_stylist(*m_list_view);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (EnumProperty<Qt::Orientation> direction_style) {
        stylist.evaluate(direction_style, [&] (auto d) {
          m_styles.m_direction = d;
        });
      },
      [&] (EnumProperty<Overflow> overflow_style) {
        stylist.evaluate(overflow_style, [&] (auto o) {
          overflow = o;
        });
      },
      [&] (const ListItemGap& item_gap) {
        stylist.evaluate(item_gap, [&] (auto gap) {
          m_styles.m_item_gap = gap;
        });
      },
      [&] (const ListOverflowGap& overflow_gap) {
        stylist.evaluate(overflow_gap, [&] (auto gap) {
          m_styles.m_overflow_gap = gap;
        });
      });
  }
  auto horizontal_policy = [&] {
    if(m_styles.m_direction == Qt::Orientation::Horizontal) {
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
    if(m_styles.m_direction == Qt::Orientation::Vertical) {
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

void Spire::navigate_to_index(ScrollableListBox& list_box, int index) {
  navigate_to_index(list_box.get_list_view(), index);
  auto& list_view = list_box.get_list_view();
  list_box.get_scroll_box().scroll_to(*list_view.get_list_item(index));
}
