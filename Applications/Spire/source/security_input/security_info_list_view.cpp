#include "spire/security_input/security_info_list_view.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QFocusEvent>
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>
#include <QVBoxLayout>
#include "spire/security_input/security_info_widget.hpp"
#include "spire/spire/dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

namespace {
  const auto MAX_VISIBLE_ITEMS = 5;

  auto make_drop_shadow_effect(QWidget* w) {
    auto drop_shadow = new QGraphicsDropShadowEffect(w);
    drop_shadow->setBlurRadius(scale_width(12));
    drop_shadow->setXOffset(0);
    drop_shadow->setYOffset(0);
    drop_shadow->setColor(QColor(0, 0, 0, 100));
    return drop_shadow;
  }
}

security_info_list_view::security_info_list_view(QWidget* key_widget,
    QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool),
      m_key_widget(key_widget),
      m_current_index(-1),
      m_hover_index(-1) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  auto layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);
  m_scroll_area = new QScrollArea(this);
  m_scroll_area->setGraphicsEffect(make_drop_shadow_effect(m_scroll_area));
  m_scroll_area->setWidgetResizable(true);
  m_scroll_area->setObjectName("security_info_list_view_scrollbar");
  m_scroll_area->setFrameShape(QFrame::NoFrame);
  m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_scroll_area->setStyleSheet(QString(R"(
    #security_info_list_view_scrollbar {
      background-color: #FFFFFF;
      border-bottom: 1px solid #A0A0A0;
      border-left: 1px solid #A0A0A0;
      border-right: 1px solid #A0A0A0;
      border-top: none;
    }
    
    QScrollBar {
      background-color: #FFFFFF;
      border: none;
      width: %1px;
    }

    QScrollBar::handle:vertical {
      background-color: #EBEBEB;
    }

    QScrollBar::sub-line:vertical {
      border: none;
      background: none;
    }

    QScrollBar::add-line:vertical {
      border: none;
      background: none;
    })").arg(scale_height(12)));
  layout->addWidget(m_scroll_area);
  m_list_widget = new QWidget(m_scroll_area);
  auto list_layout = new QVBoxLayout(m_list_widget);
  list_layout->setMargin(0);
  list_layout->setSpacing(0);
  m_list_widget->setStyleSheet("background-color: #FFFFFF;");
  m_scroll_area->setWidget(m_list_widget);
}

void security_info_list_view::set_list(const std::vector<SecurityInfo>& list) {
  while(auto item = m_list_widget->layout()->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  for(auto& security : list) {
    auto icon_path = QString(":/icons/%1.png").arg(
      security.m_security.GetCountry());
    auto security_widget = new security_info_widget(security, this);
    security_widget->connect_commit_signal(
      [=] { commit(security.m_security); });
    m_list_widget->layout()->addWidget(security_widget);
  }
  m_current_index = -1;
  if(m_list_widget->layout()->count() == 0) {
    return;
  }
  auto item_height = m_list_widget->layout()->itemAt(0)->widget()->height();
  auto h = std::min(MAX_VISIBLE_ITEMS, m_list_widget->layout()->count()) *
    item_height;
  setFixedHeight(h);
}

connection security_info_list_view::connect_clicked_signal(
    const selected_signal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

connection security_info_list_view::connect_highlighted_signal(
    const selected_signal::slot_type& slot) const {
  return m_highlighted_signal.connect(slot);
}

void security_info_list_view::leaveEvent(QEvent* event) {
  m_current_index = -1;
  m_hover_index = -1;
}

void security_info_list_view::highlight_next_item() {
  if(m_current_index >= m_list_widget->layout()->count() - 1) {
    return;
  }
  auto previous_widget = [&] () -> security_info_widget* {
    if(m_current_index == -1) {
      return nullptr;
    }
    return static_cast<security_info_widget*>(m_list_widget->layout()->itemAt(
      m_current_index)->widget());
  }();
  ++m_current_index;
  auto current_widget = static_cast<security_info_widget*>(
    m_list_widget->layout()->itemAt(m_current_index)->widget());
  update_highlight(previous_widget, current_widget);
}

void security_info_list_view::highlight_previous_item() {
  if(m_current_index <= 0) {
    return;
  }
  auto previous_widget = static_cast<security_info_widget*>(
    m_list_widget->layout()->itemAt(m_current_index)->widget());
  --m_current_index;
  auto current_widget = static_cast<security_info_widget*>(
    m_list_widget->layout()->itemAt(m_current_index)->widget());
  update_highlight(previous_widget, current_widget);
}

void security_info_list_view::commit(const Security& security) {
  m_commit_signal(security);
}

void security_info_list_view::update_hover_index(QWidget* widget) {
  m_hover_index = m_list_widget->layout()->indexOf(widget);
  if(m_current_index != -1) {
    QCoreApplication::postEvent(m_list_widget->layout()->itemAt(
      m_current_index)->widget(), new QFocusEvent(QEvent::FocusOut));
  }
  m_current_index = m_hover_index;
}

void security_info_list_view::update_highlight(security_info_widget* previous,
    security_info_widget* current) {
  if(previous != nullptr) {
    QCoreApplication::postEvent(previous, new QFocusEvent(QEvent::FocusOut));
  }
  if(current != nullptr) {
    QCoreApplication::postEvent(current, new QFocusEvent(QEvent::FocusIn));
    auto& security = current->get_info().m_security;
    m_highlighted_signal(security);
    m_scroll_area->ensureWidgetVisible(current, 0, 0);
  }
}
