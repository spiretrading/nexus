#include "Spire/Ui/DropDownList.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto SCROLLABLE_LIST_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_size(0));
    return style;
  }
}

DropDownList::DropDownList(ListView& list_view, QWidget* parent)
    : QWidget(parent),
      m_list_view(&list_view) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto scrollable_list_box = new ScrollableListBox(*m_list_view, this);
  set_style(*scrollable_list_box,
    SCROLLABLE_LIST_STYLE(get_style(*scrollable_list_box)));
  layout->addWidget(scrollable_list_box);
  m_panel = new OverlayPanel(this, parent);
  m_panel->set_closed_on_blur(true);
  m_panel->layout()->itemAt(0)->widget()->setSizePolicy(QSizePolicy::Minimum,
    QSizePolicy::Preferred);
  if(auto list_item = m_list_view->get_list_item(0)) {
    setMaximumHeight(10 * list_item->sizeHint().height());
    get_panel_border_size();
    setMinimumWidth(m_panel->parentWidget()->size().width() -
      m_panel_border_size.width());
  }
}

bool DropDownList::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    if(m_list_view->get_list_model()->get_size() > 0) {
      m_panel->show();
    }
  }
  return QWidget::event(event);
}

void DropDownList::get_panel_border_size() {
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
