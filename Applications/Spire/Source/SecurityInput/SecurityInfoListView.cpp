#include "Spire/SecurityInput/SecurityInfoListView.hpp"
#include <QVBoxLayout>
#include "Spire/SecurityInput/SecurityInfoWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/DropShadow.hpp"
#include "Spire/Ui/ScrollArea.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto MAX_VISIBLE_ITEMS = 5;
}

SecurityInfoListView::SecurityInfoListView(QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool |
        Qt::WindowStaysOnTopHint),
      m_highlighted_index(-1),
      m_active_index(-1) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  setStyleSheet("border-color: transparent;");
  m_shadow = new DropShadow(false, this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_scroll_area = new ScrollArea(this);
  m_scroll_area->setWidgetResizable(true);
  m_scroll_area->setObjectName("security_info_list_view_scroll_area");
  layout->addWidget(m_scroll_area);
  m_list_widget = new QWidget(m_scroll_area);
  auto list_layout = new QVBoxLayout(m_list_widget);
  list_layout->setContentsMargins({});
  list_layout->setSpacing(0);
  m_list_widget->setStyleSheet("background-color: #FFFFFF;");
  m_scroll_area->setWidget(m_list_widget);
}

void SecurityInfoListView::set_list(const std::vector<SecurityInfo>& list) {
  while(auto item = m_list_widget->layout()->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  m_highlighted_index = -1;
  m_active_index = -1;
  for(int i = 0; i != list.size(); ++i) {
    auto& security = list[i];
    auto icon_path = QString(":/Icons/%1.png").arg(
      security.m_security.GetCountry());
    auto security_widget = new SecurityInfoWidget(security, this);
    security_widget->connect_highlighted_signal(
      [=] (auto value) { on_highlight(i, value); });
    security_widget->connect_commit_signal(
      [=] { on_commit(security.m_security); });
    m_list_widget->layout()->addWidget(security_widget);
  }
  if(m_list_widget->layout()->count() == 0) {
    setFixedHeight(0);
    return;
  }
  auto item_height = m_list_widget->layout()->itemAt(0)->widget()->height();
  auto h = min(MAX_VISIBLE_ITEMS, m_list_widget->layout()->count()) *
    item_height;
  setFixedHeight(h + scale_width(1));
}

void SecurityInfoListView::activate_next() {
  if(m_list_widget->layout()->count() == 1) {
    update_active(0);
  } else if(m_active_index == -1) {
    update_active(m_highlighted_index + 1);
  } else {
    update_active(m_active_index + 1);
  }
}

void SecurityInfoListView::activate_previous() {
  if(m_active_index == 0) {
    return;
  } else if(m_active_index == -1) {
    update_active(m_highlighted_index - 1);
  } else {
    update_active(m_active_index - 1);
  }
}

connection SecurityInfoListView::connect_activate_signal(
    const ActivateSignal::slot_type& slot) const {
  return m_activate_signal.connect(slot);
}

connection SecurityInfoListView::connect_commit_signal(
    const CommitSignal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

void SecurityInfoListView::update_active(int active_index) {
  if(active_index == m_active_index || active_index < -1 ||
      active_index >= m_list_widget->layout()->count()) {
    return;
  }
  if(m_highlighted_index != -1) {
    auto highlighted_widget = static_cast<SecurityInfoWidget*>(
      m_list_widget->layout()->itemAt(m_highlighted_index)->widget());
    highlighted_widget->remove_highlight();
  }
  if(m_active_index != -1) {
    auto active_widget = static_cast<SecurityInfoWidget*>(
      m_list_widget->layout()->itemAt(m_active_index)->widget());
    active_widget->remove_highlight();
  };
  m_active_index = active_index;
  if(m_active_index == -1) {
    return;
  }
  auto active_widget = static_cast<SecurityInfoWidget*>(
    m_list_widget->layout()->itemAt(m_active_index)->widget());
  active_widget->set_highlighted();
  m_scroll_area->ensureWidgetVisible(active_widget, 0, 0);
  m_activate_signal(active_widget->get_info().m_security);
}

void SecurityInfoListView::on_highlight(int index, bool is_highlighted) {
  if(is_highlighted) {
    if(m_active_index != -1 && m_active_index != index) {
      auto active_widget = static_cast<SecurityInfoWidget*>(
        m_list_widget->layout()->itemAt(m_active_index)->widget());
      active_widget->remove_highlight();
      m_active_index = -1;
    }
    m_highlighted_index = index;
    auto highlighted_widget = static_cast<SecurityInfoWidget*>(
      m_list_widget->layout()->itemAt(m_highlighted_index)->widget());
    highlighted_widget->set_highlighted();
  } else {
    m_highlighted_index = m_active_index;
  }
}

void SecurityInfoListView::on_commit(const Security& security) {
  m_commit_signal(security);
}
