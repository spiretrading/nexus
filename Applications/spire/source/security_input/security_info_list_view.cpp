#include "spire/security_input/security_info_list_view.hpp"
#include <QVBoxLayout>
#include "spire/security_input/security_info_widget.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

namespace {
  const auto MAX_VISIBLE_ITEMS = 5;
}

security_info_list_view::security_info_list_view(QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool),
      m_highlighted_index(-1),
      m_active_index(-1) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  m_shadow = std::make_unique<drop_shadow>(false, this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_scroll_area = new QScrollArea(this);
  m_scroll_area->setWidgetResizable(true);
  m_scroll_area->setObjectName("security_info_list_view_scroll_area");
  m_scroll_area->setFrameShape(QFrame::NoFrame);
  m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_scroll_area->setStyleSheet(QString(R"(
    #security_info_list_view_scroll_area {
      background-color: #FFFFFF;
      border-bottom: %2px solid #A0A0A0;
      border-left: %2px solid #A0A0A0;
      border-right: %2px solid #A0A0A0;
      border-top: none;
    }
    
    QScrollBar {
      background-color: #FFFFFF;
      border: none;
      width: %1px;
    }

    QScrollBar::handle:vertical {
      background-color: #EBEBEB;
      margin-left: %3px;
    }

    QScrollBar::sub-line:vertical {
      background: none;
      border: none;
    }

    QScrollBar::add-line:vertical {
      background: none;
      border: none;
    })").arg(scale_height(12)).arg(scale_width(1)).arg(scale_width(2)));
  layout->addWidget(m_scroll_area);
  m_list_widget = new QWidget(m_scroll_area);
  auto list_layout = new QVBoxLayout(m_list_widget);
  list_layout->setContentsMargins({});
  list_layout->setSpacing(0);
  m_list_widget->setStyleSheet("background-color: #FFFFFF;");
  m_scroll_area->setWidget(m_list_widget);
}

void security_info_list_view::set_list(const std::vector<SecurityInfo>& list) {
  while(auto item = m_list_widget->layout()->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  m_highlighted_index = -1;
  m_active_index = -1;
  for(int i = 0; i != list.size(); ++i) {
    auto& security = list[i];
    auto icon_path = QString(":/icons/%1.png").arg(
      security.m_security.GetCountry());
    auto security_widget = new security_info_widget(security, this);

    // GCC workaround.
    security_widget->connect_highlighted_signal(
      [=] (auto value) { this->on_highlight(i, value); });
    security_widget->connect_commit_signal(
      [=] { on_commit(security.m_security); });
    m_list_widget->layout()->addWidget(security_widget);
  }
  if(m_list_widget->layout()->count() == 0) {
    setFixedHeight(0);
    return;
  }
  auto item_height = m_list_widget->layout()->itemAt(0)->widget()->height();
  auto h = std::min(MAX_VISIBLE_ITEMS, m_list_widget->layout()->count()) *
    item_height;
  setFixedHeight(h + scale_width(1));
}

void security_info_list_view::activate_next() {
  if(m_list_widget->layout()->count() == 1) {
    update_active(0);
  } else if(m_active_index == -1) {
    update_active(m_highlighted_index + 1);
  } else {
    update_active(m_active_index + 1);
  }
}

void security_info_list_view::activate_previous() {
  if(m_active_index == 0) {
    return;
  } else if(m_active_index == -1) {
    update_active(m_highlighted_index - 1);
  } else {
    update_active(m_active_index - 1);
  }
}

connection security_info_list_view::connect_activate_signal(
    const activate_signal::slot_type& slot) const {
  return m_activate_signal.connect(slot);
}

connection security_info_list_view::connect_commit_signal(
    const commit_signal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

void security_info_list_view::update_active(int active_index) {
  if(active_index == m_active_index || active_index < -1 ||
      active_index >= m_list_widget->layout()->count()) {
    return;
  }
  if(m_highlighted_index != -1) {
    auto highlighted_widget = static_cast<security_info_widget*>(
      m_list_widget->layout()->itemAt(m_highlighted_index)->widget());
    highlighted_widget->remove_highlight();
  }
  if(m_active_index != -1) {
    auto active_widget = static_cast<security_info_widget*>(
      m_list_widget->layout()->itemAt(m_active_index)->widget());
    active_widget->remove_highlight();
  };
  m_active_index = active_index;
  if(m_active_index == -1) {
    return;
  }
  auto active_widget = static_cast<security_info_widget*>(
    m_list_widget->layout()->itemAt(m_active_index)->widget());
  active_widget->set_highlighted();
  m_scroll_area->ensureWidgetVisible(active_widget, 0, 0);
  m_activate_signal(active_widget->get_info().m_security);
}

void security_info_list_view::on_highlight(int index, bool is_highlighted) {
  if(is_highlighted) {
    if(m_active_index != -1 && m_active_index != index) {
      auto active_widget = static_cast<security_info_widget*>(
        m_list_widget->layout()->itemAt(m_active_index)->widget());
      active_widget->remove_highlight();
      m_active_index = -1;
    }
    m_highlighted_index = index;
    auto highlighted_widget = static_cast<security_info_widget*>(
      m_list_widget->layout()->itemAt(m_highlighted_index)->widget());
    highlighted_widget->set_highlighted();
  } else {
    m_highlighted_index = m_active_index;
  }
}

void security_info_list_view::on_commit(const Security& security) {
  m_commit_signal(security);
}
