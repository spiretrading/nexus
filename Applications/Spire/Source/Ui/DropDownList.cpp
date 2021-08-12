#include "Spire/Ui/DropDownList.hpp"
#include <QEvent>
#include <QHBoxLayout>
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
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto scrollable_list_box = new ScrollableListBox(*m_list_view, this);
  scrollable_list_box->setFocusProxy(m_list_view);
  set_style(*scrollable_list_box,
    SCROLLABLE_LIST_STYLE(get_style(*scrollable_list_box)));
  layout->addWidget(scrollable_list_box);
  m_panel = new OverlayPanel(this, parent);
  m_panel->set_closed_on_blur(true);
}

bool DropDownList::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    auto list_item = m_list_view->get_list_item(0);
    if(!list_item) {
      return QWidget::event(event);
    }
    setMaximumHeight(10 * list_item->sizeHint().height());
    setMinimumWidth(m_panel->parentWidget()->size().width());
    m_panel->show();
  }
  return QWidget::event(event);
}
