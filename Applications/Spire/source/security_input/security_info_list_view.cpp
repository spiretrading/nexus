#include "spire/security_input/security_info_list_view.hpp"
#include <QEvent>
#include <QFocusEvent>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include "Nexus/Definitions/Country.hpp"
#include "spire/security_input/security_info_widget.hpp"
#include "spire/spire/dimensions.hpp"





#include <QDebug>
#include <QCoreApplication>





using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

namespace {
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
    : QWidget(parent),
      m_key_widget(key_widget),
      m_current_index(-1),
      m_hover_index(-1) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  parent->installEventFilter(this);
  auto top_layout = new QVBoxLayout(this);
  top_layout->setMargin(0);
  top_layout->setSpacing(0);
  m_scroll_area = new QScrollArea(this);
  m_scroll_area->setMaximumHeight(scale_height(200));
  m_scroll_area->setGraphicsEffect(make_drop_shadow_effect(m_scroll_area));
//  auto shadow_size = static_cast<QGraphicsDropShadowEffect*>(
//    m_scroll_area->graphicsEffect())->blurRadius();
//  setFixedWidth(m_scroll_area->width() + shadow_size);
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
  top_layout->addWidget(m_scroll_area);
  m_list_widget = new QWidget(m_scroll_area);
  auto list_layout = new QVBoxLayout(m_list_widget);
  list_layout->setMargin(0);
  list_layout->setSpacing(0);
  m_list_widget->setStyleSheet("background-color: #FFFFFF;");
  m_scroll_area->setWidget(m_list_widget);
  m_key_widget->installEventFilter(m_scroll_area);
}

void security_info_list_view::set_list(const std::vector<SecurityInfo>& list) {
  while(auto item = m_list_widget->layout()->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  for(auto& security : list) {
    auto icon_path = QString(":/icons/%1.png").arg(
      security.m_security.GetCountry());
    auto security_widget = new security_info_widget(security, icon_path, this);
    security_widget->connect_commit_signal([=] (auto& s) { commit(s); });
    security_widget->connect_hovered_signal([=] (auto w) { update_hover_index(w); });
    m_list_widget->layout()->addWidget(security_widget);
  }
  //resize(m_scroll_area->widget()->width(), list.size() * scale_height(40));
  m_current_index = -1;
}

connection security_info_list_view::connect_clicked_signal(
    const selected_signal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

connection security_info_list_view::connect_highlighted_signal(
    const selected_signal::slot_type& slot) const {
  return m_highlighted_signal.connect(slot);
}

bool security_info_list_view::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_key_widget) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Down) {
        highlight_next_item();
        return true;
      } else if(e->key() == Qt::Key_Up) {
        highlight_previous_item();
        return true;
      }
    }
  }
  if(watched == parent()) {
    if(event->type() == QEvent::Resize) {
      setFixedWidth(static_cast<QWidget*>(parent())->width());
    }
  }
  //if(watched == m_scroll_area) {
  //  if(event->type() == QEvent::Resize) {
  //    auto shadow_size = static_cast<QGraphicsDropShadowEffect*>(
  //      m_scroll_area->graphicsEffect())->blurRadius();
  //    setFixedSize(m_scroll_area->width() + shadow_size,
  //      m_list_widget->height() + shadow_size);
  //  }
  //}
  return false;
}

void security_info_list_view::leaveEvent(QEvent* event) {
  m_current_index = -1;
  m_hover_index = -1;
}

void security_info_list_view::highlight_next_item() {
  if(m_list_widget->layout()->itemAt(m_current_index + 1) != nullptr) {
    if(m_list_widget->layout()->itemAt(m_current_index) != nullptr) {
      //m_list_widget->layout()->itemAt(m_current_index)->widget()->clearFocus();
      QCoreApplication::postEvent(
        m_list_widget->layout()->itemAt(m_current_index)->widget(),
        new QFocusEvent(QEvent::FocusOut));
    }
    //m_list_widget->layout()->itemAt(++m_current_index)->widget()->setFocus();
    QCoreApplication::postEvent(
      m_list_widget->layout()->itemAt(++m_current_index)->widget(),
      new QFocusEvent(QEvent::FocusIn));
    auto security = static_cast<security_info_widget*>(
      m_list_widget->layout()->itemAt(m_current_index)->widget())->
        get_security();
    m_highlighted_signal(security);
    if(m_current_index > M_MAX_VISIBLE_ITEMS - 1) {
      m_scroll_area->ensureWidgetVisible(
        m_list_widget->layout()->itemAt(m_current_index)->widget(), 0, 0);
    }
  }
}

void security_info_list_view::highlight_previous_item() {
  if(m_list_widget->layout()->itemAt(m_current_index - 1) != nullptr) {
    //m_list_widget->layout()->itemAt(m_current_index)->widget()->clearFocus();
    QCoreApplication::postEvent(
      m_list_widget->layout()->itemAt(m_current_index)->widget(),
      new QFocusEvent(QEvent::FocusOut));
    //m_list_widget->layout()->itemAt(++m_current_index)->widget()->setFocus();
    QCoreApplication::postEvent(
      m_list_widget->layout()->itemAt(--m_current_index)->widget(),
      new QFocusEvent(QEvent::FocusIn));
    auto security = static_cast<security_info_widget*>(
      m_list_widget->layout()->itemAt(m_current_index)->widget())->
        get_security();
    m_highlighted_signal(security);
    if(m_current_index < M_MAX_VISIBLE_ITEMS - 1) {
      m_scroll_area->ensureWidgetVisible(
        m_list_widget->layout()->itemAt(m_current_index)->widget(), 0, 0);
    }
  }
}

void security_info_list_view::commit(
    const Nexus::Security& security) {
  m_commit_signal(security);
}

void security_info_list_view::update_hover_index(QWidget* widget) {
  m_hover_index = m_list_widget->layout()->indexOf(widget);
  if(m_current_index != -1) {
    QCoreApplication::postEvent(
      m_list_widget->layout()->itemAt(m_current_index)->widget(),
      new QFocusEvent(QEvent::FocusOut));
  }
  m_current_index = m_hover_index;
}
