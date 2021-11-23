#include "Spire/Ui/DropDownList.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto SCROLLABLE_LIST_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0));
    return style;
  }
}

DropDownList::DropDownList(ListView& list_view, QWidget& parent)
    : QWidget(&parent),
      m_list_view(&list_view),
      m_panel_border_size(0, 0) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_scrollable_list_box = new ScrollableListBox(*m_list_view, this);
  update_style(*m_scrollable_list_box, [&] (auto& style) {
    style = SCROLLABLE_LIST_STYLE(style);
  });
  layout->addWidget(m_scrollable_list_box);
  setFocusProxy(m_scrollable_list_box);
  m_panel = new OverlayPanel(*this, parent);
  m_panel->set_closed_on_focus_out(true);
  on_panel_style();
  m_panel_style_connection =
    connect_style_signal(*m_panel, [=] { on_panel_style(); });
  parent.installEventFilter(this);
  m_scrollable_list_box->installEventFilter(this);
}

QSize DropDownList::sizeHint() const {
  if(!m_size_hint) {
    auto width = std::max(m_panel->parentWidget()->size().width() -
      m_panel_border_size.width(), m_scrollable_list_box->sizeHint().width());
    auto height = [=] {
      if(auto list_item = m_list_view->get_list_item(0)) {
        return std::min(10 * list_item->sizeHint().height(),
          m_scrollable_list_box->sizeHint().height());
      }
      return 0;
    }();
    m_size_hint.emplace(width, height);
  }
  return *m_size_hint;
}

bool DropDownList::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize ||
      event->type() == QEvent::LayoutRequest) {
    m_size_hint = none;
    updateGeometry();
  }
  return QWidget::eventFilter(watched, event);
}

bool DropDownList::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    if(m_list_view->get_list_model()->get_size() > 0) {
      m_panel->show();
      auto margins = m_panel->layout()->contentsMargins();
      m_panel->resize(sizeHint().grownBy(margins) + m_panel_border_size);
    }
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->hide();
  }
  return QWidget::event(event);
}

void DropDownList::on_panel_style() {
  m_panel_border_size = {0, 0};
  auto& stylist = find_stylist(*m_panel);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BorderTopSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_panel_border_size.rheight() += size;
        });
      },
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_panel_border_size.rwidth() += size;
        });
      },
      [&] (const BorderBottomSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_panel_border_size.rheight() += size;
        });
      },
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_panel_border_size.rwidth() += size;
        });
      });
  }
}
